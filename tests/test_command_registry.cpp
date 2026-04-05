// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

/// @file test_command_registry.cpp
/// T5 — CommandRegistry 统一工厂 + 序列化 单元测试

#include <gtest/gtest.h>

#include "command/CommandRegistry.h"
#include "command/SetPropertyCommand.h"
#include "command/BatchSetPropertyCommand.h"
#include "command/MacroCommand.h"
#include "command/CommandContext.h"
#include "app/Document.h"
#include "model/PipePoint.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================
// 测试夹具：带一个 PipePoint 的文档 + CommandContext + CommandRegistry
// ============================================================

class CommandRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        doc = std::make_unique<app::Document>();
        pp  = std::make_shared<model::PipePoint>("PP_001",
                                                  model::PipePointType::Run,
                                                  gp_Pnt(1000.0, 0.0, 0.0));
        ppId = pp->id();
        doc->addObject(pp);

        ctx.document        = doc.get();
        ctx.dependencyGraph = nullptr;
        ctx.topologyManager = nullptr;

        reg.registerBuiltins();
    }

    std::unique_ptr<app::Document>  doc;
    std::shared_ptr<model::PipePoint> pp;
    foundation::UUID                ppId;
    command::CommandContext         ctx;
    command::CommandRegistry        reg;
};

// ============================================================
// 1. registerFactory + hasCommand + registeredCommands
// ============================================================

TEST_F(CommandRegistryTest, HasCommand_RegisteredBuiltins) {
    EXPECT_TRUE(reg.hasCommand("SetProperty"));
    EXPECT_TRUE(reg.hasCommand("BatchSetProperty"));
    EXPECT_TRUE(reg.hasCommand("Macro"));
}

TEST_F(CommandRegistryTest, RegisteredCommands_ContainsBuiltins) {
    auto cmds = reg.registeredCommands();
    EXPECT_GE(cmds.size(), 3u);

    auto contains = [&](const std::string& name) {
        return std::find(cmds.begin(), cmds.end(), name) != cmds.end();
    };
    EXPECT_TRUE(contains("SetProperty"));
    EXPECT_TRUE(contains("BatchSetProperty"));
    EXPECT_TRUE(contains("Macro"));
}

TEST_F(CommandRegistryTest, RegisterFactory_CustomCommand) {
    command::CommandRegistry r;
    EXPECT_FALSE(r.hasCommand("CustomCmd"));

    int callCount = 0;
    r.registerFactory("CustomCmd", [&callCount](const nlohmann::json&) -> std::unique_ptr<command::Command> {
        ++callCount;
        // 简单返回一个 MacroCommand 占位
        return std::make_unique<command::MacroCommand>("custom");
    });

    EXPECT_TRUE(r.hasCommand("CustomCmd"));
    nlohmann::json params;
    auto cmd = r.createFromParams("CustomCmd", params);
    EXPECT_NE(cmd, nullptr);
    EXPECT_EQ(callCount, 1);
}

// ============================================================
// 2. createFromParams — 基础流程
// ============================================================

TEST_F(CommandRegistryTest, CreateFromParams_SetProperty_AutoCapture) {
    nlohmann::json params = {
        {"objectId", ppId.toString()},
        {"key",      "x"},
        {"newValue", {{"type", "double"}, {"value", 2000.0}}}
    };
    auto cmd = reg.createFromParams("SetProperty", params);
    ASSERT_NE(cmd, nullptr);

    cmd->execute(ctx);
    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 2000.0);
}

TEST_F(CommandRegistryTest, CreateFromParams_SetProperty_WithOldValue) {
    nlohmann::json params = {
        {"objectId", ppId.toString()},
        {"key",      "x"},
        {"oldValue", {{"type", "double"}, {"value", 1000.0}}},
        {"newValue", {{"type", "double"}, {"value", 3000.0}}}
    };
    auto cmd = reg.createFromParams("SetProperty", params);
    ASSERT_NE(cmd, nullptr);

    // downcast 验证 oldValue 已设置
    auto* spc = dynamic_cast<command::SetPropertyCommand*>(cmd.get());
    ASSERT_NE(spc, nullptr);
    ASSERT_TRUE(spc->oldValue().has_value());
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(*spc->oldValue()), 1000.0);
}

TEST_F(CommandRegistryTest, CreateFromParams_UnknownCommand_ThrowsOutOfRange) {
    EXPECT_THROW(
        reg.createFromParams("NonExistentCommand", nlohmann::json{}),
        std::out_of_range);
}

TEST_F(CommandRegistryTest, CreateFromParams_BatchSetProperty) {
    nlohmann::json params = {
        {"description", "批量测试"},
        {"changes", nlohmann::json::array({
            {
                {"objectId", ppId.toString()},
                {"key",      "x"},
                {"oldValue", {{"type", "double"}, {"value", 1000.0}}},
                {"newValue", {{"type", "double"}, {"value", 5000.0}}}
            }
        })}
    };
    auto cmd = reg.createFromParams("BatchSetProperty", params);
    ASSERT_NE(cmd, nullptr);

    cmd->execute(ctx);
    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 5000.0);
}

// ============================================================
// 3. serialize（静态）
// ============================================================

TEST_F(CommandRegistryTest, Serialize_SetProperty_AfterExecute) {
    auto cmd = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1500.0});
    cmd->execute(ctx);

    nlohmann::json j = command::CommandRegistry::serialize(*cmd);
    EXPECT_EQ(j["type"], "SetProperty");
    EXPECT_EQ(j["objectId"], ppId.toString());
    EXPECT_EQ(j["key"], "x");
    EXPECT_DOUBLE_EQ(j["newValue"]["value"].get<double>(), 1500.0);
    EXPECT_DOUBLE_EQ(j["oldValue"]["value"].get<double>(), 1000.0);
}

TEST_F(CommandRegistryTest, Serialize_MacroCommand) {
    auto macro = std::make_unique<command::MacroCommand>("测试宏");
    macro->addCommand(command::SetPropertyCommand::createWithOldValue(
        ppId, "x", foundation::Variant{1000.0}, foundation::Variant{2000.0}));

    macro->execute(ctx);

    nlohmann::json j = command::CommandRegistry::serialize(*macro);
    EXPECT_EQ(j["type"], "Macro");
    EXPECT_EQ(j["description"], "测试宏");
    ASSERT_TRUE(j.contains("children"));
    EXPECT_EQ(j["children"].size(), 1u);
    EXPECT_EQ(j["children"][0]["type"], "SetProperty");
}

// ============================================================
// 4. createFromFullJson + deserialize
// ============================================================

TEST_F(CommandRegistryTest, CreateFromFullJson_SetProperty) {
    nlohmann::json j = {
        {"type",     "SetProperty"},
        {"id",       foundation::UUID::generate().toString()},
        {"objectId", ppId.toString()},
        {"key",      "x"},
        {"oldValue", {{"type", "double"}, {"value", 1000.0}}},
        {"newValue", {{"type", "double"}, {"value", 2500.0}}}
    };

    auto cmd = reg.createFromFullJson(j);
    ASSERT_NE(cmd, nullptr);
    EXPECT_EQ(cmd->type(), command::CommandType::SetProperty);

    cmd->execute(ctx);
    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 2500.0);
}

TEST_F(CommandRegistryTest, Deserialize_SameAs_CreateFromFullJson) {
    nlohmann::json j = {
        {"type",     "SetProperty"},
        {"id",       foundation::UUID::generate().toString()},
        {"objectId", ppId.toString()},
        {"key",      "x"},
        {"oldValue", {{"type", "double"}, {"value", 1000.0}}},
        {"newValue", {{"type", "double"}, {"value", 3500.0}}}
    };

    auto cmd1 = reg.createFromFullJson(j);
    auto cmd2 = reg.deserialize(j);

    ASSERT_NE(cmd1, nullptr);
    ASSERT_NE(cmd2, nullptr);
    EXPECT_EQ(cmd1->type(), cmd2->type());
}

// ============================================================
// 5. SetProperty serialize → deserialize round-trip
// ============================================================

TEST_F(CommandRegistryTest, SerializeDeserialize_SetProperty_RoundTrip) {
    // 创建并执行原始命令
    auto original = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{4000.0});
    original->execute(ctx);

    // 序列化
    nlohmann::json j = command::CommandRegistry::serialize(*original);
    EXPECT_EQ(j["type"], "SetProperty");

    // 恢复到初始状态（undo）
    original->undo(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1000.0);

    // 反序列化后再执行
    auto restored = reg.deserialize(j);
    ASSERT_NE(restored, nullptr);
    restored->execute(ctx);
    EXPECT_TRUE(restored->lastResult().success);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 4000.0);
}

TEST_F(CommandRegistryTest, SerializeDeserialize_BatchSetProperty_RoundTrip) {
    std::vector<command::BatchSetPropertyCommand::Change> changes;
    changes.push_back({ppId, "x",
                       foundation::Variant{1000.0},
                       foundation::Variant{7000.0}});

    auto original = std::make_unique<command::BatchSetPropertyCommand>(
        "批量测试", std::move(changes));
    original->execute(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 7000.0);

    nlohmann::json j = command::CommandRegistry::serialize(*original);
    EXPECT_EQ(j["type"], "BatchSetProperty");

    // undo 恢复
    original->undo(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1000.0);

    // 反序列化再执行
    auto restored = reg.deserialize(j);
    ASSERT_NE(restored, nullptr);
    restored->execute(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 7000.0);
}

// ============================================================
// 6. serializeSequence + deserializeSequence
// ============================================================

TEST_F(CommandRegistryTest, SerializeSequence_DeserializeSequence_RoundTrip) {
    // 创建两个命令并执行
    auto cmd1 = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{2000.0});
    cmd1->execute(ctx);

    auto cmd2 = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{3000.0});
    cmd2->execute(ctx);

    // 序列化序列
    std::vector<const command::Command*> ptrs = {cmd1.get(), cmd2.get()};
    nlohmann::json seq = command::CommandRegistry::serializeSequence(ptrs);

    ASSERT_TRUE(seq.is_array());
    ASSERT_EQ(seq.size(), 2u);
    EXPECT_EQ(seq[0]["type"], "SetProperty");
    EXPECT_EQ(seq[1]["type"], "SetProperty");

    // 恢复到初始状态
    cmd2->undo(ctx);
    cmd1->undo(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1000.0);

    // 反序列化序列并顺序执行
    auto cmds = reg.deserializeSequence(seq);
    ASSERT_EQ(cmds.size(), 2u);

    for (auto& cmd : cmds) {
        cmd->execute(ctx);
    }
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 3000.0);
}

TEST_F(CommandRegistryTest, SerializeSequence_EmptyList) {
    std::vector<const command::Command*> ptrs;
    nlohmann::json seq = command::CommandRegistry::serializeSequence(ptrs);

    ASSERT_TRUE(seq.is_array());
    EXPECT_TRUE(seq.empty());
}

TEST_F(CommandRegistryTest, DeserializeSequence_EmptyArray) {
    nlohmann::json arr = nlohmann::json::array();
    auto cmds = reg.deserializeSequence(arr);
    EXPECT_TRUE(cmds.empty());
}

// ============================================================
// 7. Macro 工厂 — 含子命令序列化 / 反序列化
// ============================================================

TEST_F(CommandRegistryTest, Macro_Deserialize_WithChildren) {
    // 构建含子命令的 MacroCommand 并执行
    auto macro = std::make_unique<command::MacroCommand>("宏测试");
    macro->addCommand(command::SetPropertyCommand::createWithOldValue(
        ppId, "x",
        foundation::Variant{1000.0},
        foundation::Variant{9000.0}));
    macro->execute(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 9000.0);

    // 序列化
    nlohmann::json j = command::CommandRegistry::serialize(*macro);
    EXPECT_EQ(j["type"], "Macro");
    ASSERT_EQ(j["children"].size(), 1u);

    // undo 恢复
    macro->undo(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1000.0);

    // 反序列化后执行
    auto restored = reg.deserialize(j);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->type(), command::CommandType::Macro);

    restored->execute(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 9000.0);
}

// ============================================================
// 8. 边界条件 & 错误路径
// ============================================================

TEST_F(CommandRegistryTest, HasCommand_UnknownName_ReturnsFalse) {
    EXPECT_FALSE(reg.hasCommand("UnknownCommand"));
    EXPECT_FALSE(reg.hasCommand(""));
}

TEST_F(CommandRegistryTest, CreateFromFullJson_UnknownType_ThrowsOutOfRange) {
    nlohmann::json j = {{"type", "GhostCommand"}};
    EXPECT_THROW(reg.createFromFullJson(j), std::out_of_range);
}

TEST_F(CommandRegistryTest, CreateFromParams_TypeFieldInjected) {
    // 验证 createFromParams 会将 commandName 注入到 "type" 字段
    bool typeChecked = false;
    command::CommandRegistry r;
    r.registerFactory("CheckType", [&typeChecked](const nlohmann::json& j) -> std::unique_ptr<command::Command> {
        typeChecked = (j.value("type", "") == "CheckType");
        return std::make_unique<command::MacroCommand>("check");
    });

    auto cmd = r.createFromParams("CheckType", nlohmann::json{});
    EXPECT_TRUE(typeChecked);
}

TEST_F(CommandRegistryTest, RegisterBuiltins_Idempotent_OverwritesSameKey) {
    // 重复注册同一个 key 应该不报错（后注册覆盖前注册）
    EXPECT_NO_THROW(reg.registerBuiltins());
    EXPECT_TRUE(reg.hasCommand("SetProperty"));
}
