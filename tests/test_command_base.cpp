// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

/// @file test_command_base.cpp
/// T2 — Command 基类、MacroCommand、PropertyApplier 单元测试

#include <gtest/gtest.h>

#include "command/Command.h"
#include "command/CommandContext.h"
#include "command/CommandResult.h"
#include "command/CommandType.h"
#include "command/MacroCommand.h"
#include "command/PropertyApplier.h"
#include "model/PipePoint.h"
#include "model/DocumentObject.h"

#include <memory>
#include <stdexcept>
#include <vector>

// ============================================================
// 测试用 Stub Command：记录 execute/undo 调用顺序
// ============================================================

/// 可配置失败的 Stub，将调用记录到外部 log（execute 写正索引，undo 写负索引）
class StubCommand : public command::Command {
public:
    StubCommand(std::string name, int idx, std::vector<int>* log, bool fail = false)
        : name_(std::move(name)), idx_(idx), log_(log), fail_(fail) {}

    void execute(command::CommandContext&) override {
        if (fail_) throw std::runtime_error("StubCommand: forced failure");
        if (log_) log_->push_back(idx_);
        lastResult_.success = true;
    }

    void undo(command::CommandContext&) override {
        if (log_) log_->push_back(-idx_);
        lastResult_.success = true;
    }

    std::string description() const override { return name_; }
    command::CommandType type() const override { return command::CommandType::SetProperty; }
    nlohmann::json toJson() const override {
        return {{"type", "Stub"}, {"name", name_}, {"idx", idx_}};
    }

private:
    std::string name_;
    int idx_;
    std::vector<int>* log_;
    bool fail_;
};

// ============================================================
// CommandResult 测试
// ============================================================

TEST(CommandResult, DefaultIsFailure) {
    command::CommandResult r;
    EXPECT_FALSE(r.success);
    EXPECT_TRUE(r.errorMessage.empty());
    EXPECT_TRUE(r.createdIds.empty());
    EXPECT_TRUE(r.deletedIds.empty());
    EXPECT_TRUE(r.affectedIds.empty());
}

TEST(CommandResult, ToJson) {
    command::CommandResult r;
    r.success = true;
    r.errorMessage = "";
    auto j = r.toJson();
    EXPECT_TRUE(j["success"].get<bool>());
    EXPECT_EQ(j["createdIds"].size(), 0u);
}

// ============================================================
// Command 基类接口测试（通过 StubCommand）
// ============================================================

TEST(CommandBase, HasUniqueId) {
    std::vector<int> log;
    StubCommand a("A", 1, &log);
    StubCommand b("B", 2, &log);
    EXPECT_NE(a.id(), b.id());
}

TEST(CommandBase, HasTimestamp) {
    std::vector<int> log;
    StubCommand cmd("X", 1, &log);
    // timestamp 应该是最近的时刻（此测试只验证不抛异常）
    auto ts = cmd.timestamp();
    (void)ts;
    SUCCEED();
}

TEST(CommandBase, ExecuteUpdatesLastResult) {
    std::vector<int> log;
    StubCommand cmd("A", 1, &log);
    command::CommandContext ctx{};
    EXPECT_FALSE(cmd.lastResult().success);  // 执行前 success=false
    cmd.execute(ctx);
    EXPECT_TRUE(cmd.lastResult().success);
    EXPECT_EQ(log, std::vector<int>({1}));
}

TEST(CommandBase, UndoAfterExecute) {
    std::vector<int> log;
    StubCommand cmd("A", 5, &log);
    command::CommandContext ctx{};
    cmd.execute(ctx);
    cmd.undo(ctx);
    EXPECT_EQ(log, (std::vector<int>{5, -5}));
}

TEST(CommandBase, Description) {
    std::vector<int> log;
    StubCommand cmd("HelloCmd", 1, &log);
    EXPECT_EQ(cmd.description(), "HelloCmd");
}

TEST(CommandBase, TypeIsSetProperty) {
    std::vector<int> log;
    StubCommand cmd("T", 1, &log);
    EXPECT_EQ(cmd.type(), command::CommandType::SetProperty);
}

TEST(CommandBase, TryMergeReturnsFalseByDefault) {
    std::vector<int> log;
    StubCommand a("A", 1, &log);
    StubCommand b("B", 2, &log);
    EXPECT_FALSE(a.tryMerge(b));
}

TEST(CommandBase, ToJson) {
    std::vector<int> log;
    StubCommand cmd("MyCmd", 3, &log);
    auto j = cmd.toJson();
    EXPECT_EQ(j["type"], "Stub");
    EXPECT_EQ(j["name"], "MyCmd");
}

// ============================================================
// MacroCommand — 顺序执行 + 逆序撤销
// ============================================================

TEST(MacroCommand, SequentialExecute) {
    std::vector<int> log;
    auto macro = std::make_unique<command::MacroCommand>("Multi");
    macro->addCommand(std::make_unique<StubCommand>("A", 1, &log));
    macro->addCommand(std::make_unique<StubCommand>("B", 2, &log));
    macro->addCommand(std::make_unique<StubCommand>("C", 3, &log));

    command::CommandContext ctx{};
    macro->execute(ctx);

    EXPECT_TRUE(macro->lastResult().success);
    // 顺序: 1, 2, 3
    EXPECT_EQ(log, (std::vector<int>{1, 2, 3}));
}

TEST(MacroCommand, ReverseUndo) {
    std::vector<int> log;
    auto macro = std::make_unique<command::MacroCommand>("Multi");
    macro->addCommand(std::make_unique<StubCommand>("A", 1, &log));
    macro->addCommand(std::make_unique<StubCommand>("B", 2, &log));
    macro->addCommand(std::make_unique<StubCommand>("C", 3, &log));

    command::CommandContext ctx{};
    macro->execute(ctx);
    log.clear();
    macro->undo(ctx);

    // 逆序: -3, -2, -1
    EXPECT_EQ(log, (std::vector<int>{-3, -2, -1}));
    EXPECT_TRUE(macro->lastResult().success);
}

TEST(MacroCommand, TypeIsMacro) {
    auto macro = std::make_unique<command::MacroCommand>("M");
    EXPECT_EQ(macro->type(), command::CommandType::Macro);
}

TEST(MacroCommand, Description) {
    auto macro = std::make_unique<command::MacroCommand>("InsertElbow");
    EXPECT_EQ(macro->description(), "InsertElbow");
}

TEST(MacroCommand, Children) {
    std::vector<int> log;
    auto macro = std::make_unique<command::MacroCommand>("M");
    macro->addCommand(std::make_unique<StubCommand>("A", 1, &log));
    macro->addCommand(std::make_unique<StubCommand>("B", 2, &log));
    EXPECT_EQ(macro->children().size(), 2u);
}

TEST(MacroCommand, ToJson) {
    std::vector<int> log;
    auto macro = std::make_unique<command::MacroCommand>("TestMacro");
    macro->addCommand(std::make_unique<StubCommand>("A", 1, &log));
    auto j = macro->toJson();
    EXPECT_EQ(j["type"], "Macro");
    EXPECT_EQ(j["description"], "TestMacro");
    EXPECT_EQ(j["children"].size(), 1u);
}

// ============================================================
// MacroCommand — 子命令失败时自动回滚
// ============================================================

TEST(MacroCommand, RollbackOnChildFailure) {
    std::vector<int> log;

    auto macro = std::make_unique<command::MacroCommand>("WithFail");
    // 子命令 A、B 正常执行；子命令 C 失败 → 应逆序撤销 B、A
    macro->addCommand(std::make_unique<StubCommand>("A", 1, &log));
    macro->addCommand(std::make_unique<StubCommand>("B", 2, &log));
    macro->addCommand(std::make_unique<StubCommand>("C_fail", 3, &log, /*fail=*/true));

    command::CommandContext ctx{};
    EXPECT_THROW(macro->execute(ctx), std::runtime_error);

    // A 和 B 执行了（log 中有 1, 2），然后回滚逆序 undo B(-2), A(-1)
    // C 失败前未推入 log（因为 fail=true 在推入前抛出）
    EXPECT_EQ(log, (std::vector<int>{1, 2, -2, -1}));

    // lastResult 应为失败
    EXPECT_FALSE(macro->lastResult().success);
}

TEST(MacroCommand, RollbackOnFirstChildFailure) {
    std::vector<int> log;

    auto macro = std::make_unique<command::MacroCommand>("FirstFail");
    // 第一个子命令就失败，无需回滚任何子命令
    macro->addCommand(std::make_unique<StubCommand>("A_fail", 1, &log, /*fail=*/true));
    macro->addCommand(std::make_unique<StubCommand>("B", 2, &log));

    command::CommandContext ctx{};
    EXPECT_THROW(macro->execute(ctx), std::runtime_error);

    // 没有任何命令成功执行，log 为空
    EXPECT_TRUE(log.empty());
    EXPECT_FALSE(macro->lastResult().success);
}

TEST(MacroCommand, EmptyMacroSucceeds) {
    auto macro = std::make_unique<command::MacroCommand>("Empty");
    command::CommandContext ctx{};
    macro->execute(ctx);
    EXPECT_TRUE(macro->lastResult().success);
}

// ============================================================
// PropertyApplier — apply / read 多态分派
// ============================================================

TEST(PropertyApplier, ApplyNameToPipePoint) {
    auto pp = std::make_shared<model::PipePoint>("original");
    foundation::Variant newName = std::string("renamed");
    command::PropertyApplier::apply(pp.get(), "name", newName);
    EXPECT_EQ(pp->name(), "renamed");
}

TEST(PropertyApplier, ApplyCoordinateToPipePoint) {
    auto pp = std::make_shared<model::PipePoint>("P1");
    command::PropertyApplier::apply(pp.get(), "x", foundation::Variant{3.14});
    command::PropertyApplier::apply(pp.get(), "y", foundation::Variant{2.71});
    command::PropertyApplier::apply(pp.get(), "z", foundation::Variant{1.0});
    auto pos = pp->position();
    EXPECT_NEAR(pos.X(), 3.14, 1e-9);
    EXPECT_NEAR(pos.Y(), 2.71, 1e-9);
    EXPECT_NEAR(pos.Z(), 1.0,  1e-9);
}

TEST(PropertyApplier, ApplyTypeToPipePoint) {
    auto pp = std::make_shared<model::PipePoint>("P");
    foundation::Variant bendType = static_cast<int>(model::PipePointType::Bend);
    command::PropertyApplier::apply(pp.get(), "type", bendType);
    EXPECT_EQ(pp->type(), model::PipePointType::Bend);
}

TEST(PropertyApplier, ReadNameFromPipePoint) {
    auto pp = std::make_shared<model::PipePoint>("ReadMe");
    auto val = command::PropertyApplier::read(pp.get(), "name");
    ASSERT_TRUE(std::holds_alternative<std::string>(val));
    EXPECT_EQ(std::get<std::string>(val), "ReadMe");
}

TEST(PropertyApplier, ReadCoordinateFromPipePoint) {
    auto pp = std::make_shared<model::PipePoint>("Q");
    command::PropertyApplier::apply(pp.get(), "x", foundation::Variant{7.0});
    auto val = command::PropertyApplier::read(pp.get(), "x");
    EXPECT_DOUBLE_EQ(foundation::variantToDouble(val), 7.0);
}

TEST(PropertyApplier, UnknownKeyOnDocumentObjectThrows) {
    // DocumentObject 基类只识别 "name"，其余 key 返回 false → PropertyApplier 抛出
    auto obj = std::make_shared<model::DocumentObject>("BaseObj");
    EXPECT_THROW(
        command::PropertyApplier::apply(obj.get(), "unknown_xyz", foundation::Variant{1.0}),
        std::invalid_argument);
}

TEST(PropertyApplier, NullObjectThrows) {
    EXPECT_THROW(
        command::PropertyApplier::apply(nullptr, "x", foundation::Variant{1.0}),
        std::invalid_argument);
}

TEST(PropertyApplier, NullObjectReadThrows) {
    EXPECT_THROW(
        command::PropertyApplier::read(nullptr, "x"),
        std::invalid_argument);
}

TEST(PropertyApplier, ReadUnknownKeyOnDocumentObjectThrows) {
    auto obj = std::make_shared<model::DocumentObject>("Obj");
    EXPECT_THROW(
        command::PropertyApplier::read(obj.get(), "xyz"),
        std::out_of_range);
}

// ============================================================
// CommandType 枚举完整性
// ============================================================

TEST(CommandType, EnumValues) {
    // 确保关键枚举值存在且可使用
    EXPECT_EQ(static_cast<int>(command::CommandType::SetProperty), 0);
    EXPECT_NE(command::CommandType::Macro, command::CommandType::SetProperty);
    EXPECT_NE(command::CommandType::CreatePipePoint, command::CommandType::DeletePipePoint);
}
