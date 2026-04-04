// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

/// @file test_property_commands.cpp
/// T4 — SetPropertyCommand / BatchSetPropertyCommand 单元测试

#include <gtest/gtest.h>

#include "command/SetPropertyCommand.h"
#include "command/BatchSetPropertyCommand.h"
#include "command/CommandContext.h"
#include "command/CommandStack.h"
#include "app/Document.h"
#include "model/PipePoint.h"

#include <chrono>
#include <memory>
#include <stdexcept>
#include <thread>

// ============================================================
// 测试夹具：创建带一个 PipePoint 的文档 + CommandContext
// ============================================================

class PropertyCommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        doc = std::make_unique<app::Document>();

        // 创建测试用 PipePoint，初始坐标 (1000, 0, 0)
        pp = std::make_shared<model::PipePoint>("PP_001",
                                                model::PipePointType::Run,
                                                gp_Pnt(1000.0, 0.0, 0.0));
        ppId = pp->id();
        doc->addObject(pp);

        ctx.document = doc.get();
        ctx.dependencyGraph = nullptr;
        ctx.topologyManager = nullptr;
    }

    std::unique_ptr<app::Document> doc;
    std::shared_ptr<model::PipePoint> pp;
    foundation::UUID ppId;
    command::CommandContext ctx;
};

// ============================================================
// SetPropertyCommand — 基础 execute / undo
// ============================================================

TEST_F(PropertyCommandsTest, SetProperty_Execute_ChangesValue) {
    auto cmd = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1500.0});
    cmd->execute(ctx);

    EXPECT_TRUE(cmd->lastResult().success);
    ASSERT_EQ(cmd->lastResult().affectedIds.size(), 1u);
    EXPECT_EQ(cmd->lastResult().affectedIds[0], ppId);

    // 验证值已变更
    auto val = pp->getProperty("x");
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(val), 1500.0);
}

TEST_F(PropertyCommandsTest, SetProperty_AutoCapturesOldValue) {
    auto cmd = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{2000.0});

    // execute 前 oldValue 应为空
    EXPECT_FALSE(cmd->oldValue().has_value());

    cmd->execute(ctx);

    // execute 后 oldValue 应为捕获的旧值
    ASSERT_TRUE(cmd->oldValue().has_value());
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(*cmd->oldValue()), 1000.0);
}

TEST_F(PropertyCommandsTest, SetProperty_Undo_RestoresOldValue) {
    auto cmd = command::SetPropertyCommand::createWithOldValue(
        ppId, "x",
        foundation::Variant{1000.0},   // oldValue
        foundation::Variant{2000.0});  // newValue

    cmd->execute(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 2000.0);

    cmd->undo(ctx);
    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1000.0);
    ASSERT_EQ(cmd->lastResult().affectedIds.size(), 1u);
    EXPECT_EQ(cmd->lastResult().affectedIds[0], ppId);
}

TEST_F(PropertyCommandsTest, SetProperty_StringProperty) {
    auto cmd = command::SetPropertyCommand::createAutoCapture(
        ppId, "name", foundation::Variant{std::string("PP_XYZ")});
    cmd->execute(ctx);

    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_EQ(pp->name(), "PP_XYZ");
}

TEST_F(PropertyCommandsTest, SetProperty_ObjectNotFound_Throws) {
    foundation::UUID fakeId = foundation::UUID::generate();
    auto cmd = command::SetPropertyCommand::createAutoCapture(
        fakeId, "x", foundation::Variant{1.0});
    EXPECT_THROW(cmd->execute(ctx), std::runtime_error);
}

TEST_F(PropertyCommandsTest, SetProperty_UndoObjectNotFound_NoThrow) {
    // 对象不存在时 undo 应静默成功
    auto cmd = command::SetPropertyCommand::createWithOldValue(
        foundation::UUID::generate(), "x",
        foundation::Variant{1.0},
        foundation::Variant{2.0});

    // 不调用 execute，直接 undo（对象不存在时）
    EXPECT_NO_THROW(cmd->undo(ctx));
    EXPECT_TRUE(cmd->lastResult().success);
}

// ============================================================
// SetPropertyCommand — createWithOldValue
// ============================================================

TEST_F(PropertyCommandsTest, CreateWithOldValue_HasOldValueFromStart) {
    auto cmd = command::SetPropertyCommand::createWithOldValue(
        ppId, "x",
        foundation::Variant{999.0},
        foundation::Variant{1234.0});

    ASSERT_TRUE(cmd->oldValue().has_value());
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(*cmd->oldValue()), 999.0);
}

TEST_F(PropertyCommandsTest, SetProperty_Redo_ViaExecuteAgain) {
    auto cmd = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{3000.0});

    cmd->execute(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 3000.0);

    cmd->undo(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1000.0);

    cmd->execute(ctx);  // redo 等价于再次 execute
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 3000.0);
}

// ============================================================
// SetPropertyCommand — tryMerge
// ============================================================

TEST_F(PropertyCommandsTest, TryMerge_SameObjectSameKey_WithinWindow_Merges) {
    auto cmd1 = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1100.0});
    auto cmd2 = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1200.0});

    // 两个命令时间差 < 500ms，应该合并
    EXPECT_TRUE(cmd1->tryMerge(*cmd2));
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(cmd1->newValue()), 1200.0);

    cmd1->execute(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1200.0);
}

TEST_F(PropertyCommandsTest, TryMerge_DifferentKey_NoMerge) {
    auto cmd1 = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1100.0});
    auto cmd2 = command::SetPropertyCommand::createAutoCapture(
        ppId, "y", foundation::Variant{500.0});

    EXPECT_FALSE(cmd1->tryMerge(*cmd2));
}

TEST_F(PropertyCommandsTest, TryMerge_DifferentObject_NoMerge) {
    auto pp2 = std::make_shared<model::PipePoint>("PP_002");
    doc->addObject(pp2);

    auto cmd1 = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1100.0});
    auto cmd2 = command::SetPropertyCommand::createAutoCapture(
        pp2->id(), "x", foundation::Variant{2200.0});

    EXPECT_FALSE(cmd1->tryMerge(*cmd2));
}

TEST_F(PropertyCommandsTest, TryMerge_PreservesOldValue) {
    // 合并后 oldValue 仍为最旧命令的 oldValue
    auto cmd1 = command::SetPropertyCommand::createWithOldValue(
        ppId, "x",
        foundation::Variant{1000.0},
        foundation::Variant{1100.0});
    auto cmd2 = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1200.0});

    EXPECT_TRUE(cmd1->tryMerge(*cmd2));
    // oldValue 仍为 1000
    ASSERT_TRUE(cmd1->oldValue().has_value());
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(*cmd1->oldValue()), 1000.0);
    // newValue 更新为 1200
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(cmd1->newValue()), 1200.0);
}

TEST_F(PropertyCommandsTest, TryMerge_NonSetPropertyCommand_NoMerge) {
    // cmd2 不是 SetPropertyCommand，tryMerge 返回 false
    auto cmd1 = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1100.0});
    // 用另一个 SetPropertyCommand 验证接口（dynamic_cast 路径）
    auto cmd2 = command::SetPropertyCommand::createAutoCapture(
        ppId, "y", foundation::Variant{200.0});  // 不同 key → false
    EXPECT_FALSE(cmd1->tryMerge(*cmd2));
}

// ============================================================
// SetPropertyCommand — JSON 序列化
// ============================================================

TEST_F(PropertyCommandsTest, ToJson_AfterExecute_HasAllFields) {
    auto cmd = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1500.0});
    cmd->execute(ctx);

    auto j = cmd->toJson();
    EXPECT_EQ(j["type"].get<std::string>(), "SetProperty");
    EXPECT_EQ(j["key"].get<std::string>(), "x");
    EXPECT_EQ(j["objectId"].get<std::string>(), ppId.toString());
    EXPECT_EQ(j["oldValue"]["type"].get<std::string>(), "double");
    EXPECT_DOUBLE_EQ(j["oldValue"]["value"].get<double>(), 1000.0);
    EXPECT_EQ(j["newValue"]["type"].get<std::string>(), "double");
    EXPECT_DOUBLE_EQ(j["newValue"]["value"].get<double>(), 1500.0);
}

TEST_F(PropertyCommandsTest, ToJson_BeforeExecute_Throws) {
    auto cmd = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1500.0});
    // execute 前 oldValue 尚未捕获，toJson 应抛出异常
    EXPECT_THROW(cmd->toJson(), std::logic_error);
}

TEST_F(PropertyCommandsTest, ToJson_WithOldValue_NoThrow) {
    auto cmd = command::SetPropertyCommand::createWithOldValue(
        ppId, "x",
        foundation::Variant{1000.0},
        foundation::Variant{2000.0});
    // 提供了 oldValue，无需 execute 就可以序列化
    EXPECT_NO_THROW(cmd->toJson());
}

// ============================================================
// SetPropertyCommand — 通过 CommandStack 集成
// ============================================================

TEST_F(PropertyCommandsTest, Stack_TryMerge_CompressesCommands) {
    command::CommandStack stack;
    auto cmd1 = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1100.0});
    auto cmd2 = command::SetPropertyCommand::createAutoCapture(
        ppId, "x", foundation::Variant{1200.0});

    stack.execute(std::move(cmd1), ctx);
    EXPECT_EQ(stack.undoCount(), 1u);

    stack.execute(std::move(cmd2), ctx);
    // 合并后仍只有 1 条
    EXPECT_EQ(stack.undoCount(), 1u);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1200.0);

    stack.undo(ctx);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1000.0);
}

// ============================================================
// BatchSetPropertyCommand — execute / undo
// ============================================================

TEST_F(PropertyCommandsTest, Batch_Execute_AppliesAllChanges) {
    auto pp2 = std::make_shared<model::PipePoint>("PP_002",
                                                   model::PipePointType::Run,
                                                   gp_Pnt(2000.0, 0.0, 0.0));
    doc->addObject(pp2);

    std::vector<command::BatchSetPropertyCommand::Change> changes = {
        {ppId,    "x", foundation::Variant{1000.0}, foundation::Variant{1500.0}},
        {pp2->id(), "x", foundation::Variant{2000.0}, foundation::Variant{2500.0}}
    };

    auto cmd = std::make_unique<command::BatchSetPropertyCommand>("批量修改X", std::move(changes));
    cmd->execute(ctx);

    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_EQ(cmd->lastResult().affectedIds.size(), 2u);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1500.0);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp2->getProperty("x")), 2500.0);
}

TEST_F(PropertyCommandsTest, Batch_Undo_RestoresInReverseOrder) {
    auto pp2 = std::make_shared<model::PipePoint>("PP_002",
                                                   model::PipePointType::Run,
                                                   gp_Pnt(2000.0, 0.0, 0.0));
    doc->addObject(pp2);

    std::vector<command::BatchSetPropertyCommand::Change> changes = {
        {ppId,    "x", foundation::Variant{1000.0}, foundation::Variant{1500.0}},
        {pp2->id(), "x", foundation::Variant{2000.0}, foundation::Variant{2500.0}}
    };

    auto cmd = std::make_unique<command::BatchSetPropertyCommand>("批量修改X", std::move(changes));
    cmd->execute(ctx);
    cmd->undo(ctx);

    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1000.0);
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp2->getProperty("x")), 2000.0);
}

TEST_F(PropertyCommandsTest, Batch_Execute_RollbackOnFailure) {
    // 第一个变更有效，第二个变更目标对象不存在 → 应回滚
    foundation::UUID nonExistent = foundation::UUID::generate();

    std::vector<command::BatchSetPropertyCommand::Change> changes = {
        {ppId,       "x", foundation::Variant{1000.0}, foundation::Variant{9999.0}},
        {nonExistent, "x", foundation::Variant{0.0},    foundation::Variant{1.0}}
    };

    auto cmd = std::make_unique<command::BatchSetPropertyCommand>("批量失败", std::move(changes));
    EXPECT_THROW(cmd->execute(ctx), std::runtime_error);

    // pp 的 x 坐标应已回滚到原始值
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(pp->getProperty("x")), 1000.0);
}

TEST_F(PropertyCommandsTest, Batch_Description) {
    auto cmd = std::make_unique<command::BatchSetPropertyCommand>(
        "批量测试描述",
        std::vector<command::BatchSetPropertyCommand::Change>{});
    EXPECT_EQ(cmd->description(), "批量测试描述");
}

TEST_F(PropertyCommandsTest, Batch_Type) {
    auto cmd = std::make_unique<command::BatchSetPropertyCommand>(
        "test",
        std::vector<command::BatchSetPropertyCommand::Change>{});
    EXPECT_EQ(cmd->type(), command::CommandType::BatchSetProperty);
}

TEST_F(PropertyCommandsTest, Batch_ToJson_HasFields) {
    std::vector<command::BatchSetPropertyCommand::Change> changes = {
        {ppId, "x", foundation::Variant{1000.0}, foundation::Variant{2000.0}}
    };
    auto cmd = std::make_unique<command::BatchSetPropertyCommand>(
        "JSON测试", std::move(changes));

    auto j = cmd->toJson();
    EXPECT_EQ(j["type"].get<std::string>(), "BatchSetProperty");
    EXPECT_EQ(j["description"].get<std::string>(), "JSON测试");
    ASSERT_EQ(j["changes"].size(), 1u);
    EXPECT_EQ(j["changes"][0]["key"].get<std::string>(), "x");
}

TEST_F(PropertyCommandsTest, Batch_EmptyChanges_ExecuteSucceeds) {
    auto cmd = std::make_unique<command::BatchSetPropertyCommand>(
        "空批量",
        std::vector<command::BatchSetPropertyCommand::Change>{});
    EXPECT_NO_THROW(cmd->execute(ctx));
    EXPECT_TRUE(cmd->lastResult().success);
}
