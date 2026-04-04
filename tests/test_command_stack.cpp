// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

/// @file test_command_stack.cpp
/// T3 — CommandStack 命令栈管理器单元测试

#include <gtest/gtest.h>

#include "command/Command.h"
#include "command/CommandContext.h"
#include "command/CommandResult.h"
#include "command/CommandStack.h"
#include "command/CommandType.h"
#include "foundation/Types.h"

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================
// 测试用 Stub Command（可配置合并/不合并）
// ============================================================

/// 记录 execute/undo 调用序列的基础 Stub
class StubCmd : public command::Command {
public:
    explicit StubCmd(std::string name, std::vector<std::string>* log = nullptr)
        : name_(std::move(name)), log_(log)
    {
        // 默认 deletedIds 为空，affectedIds 为空
    }

    void execute(command::CommandContext&) override {
        if (log_) log_->push_back("exec:" + name_);
        lastResult_.success = true;
        lastResult_.affectedIds = affectedOnExec_;
        lastResult_.deletedIds  = deletedOnExec_;
    }

    void undo(command::CommandContext&) override {
        if (log_) log_->push_back("undo:" + name_);
        lastResult_.success = true;
        lastResult_.affectedIds = affectedOnUndo_;
        lastResult_.deletedIds  = deletedOnUndo_;
    }

    std::string description() const override { return name_; }
    command::CommandType type() const override { return command::CommandType::SetProperty; }
    nlohmann::json toJson() const override { return {{"name", name_}}; }

    // 设置 execute 后报告的 deletedIds（用于 sceneRemoveRequested 测试）
    void setDeletedOnExec(std::vector<foundation::UUID> ids) { deletedOnExec_ = std::move(ids); }
    void setAffectedOnExec(std::vector<foundation::UUID> ids) { affectedOnExec_ = std::move(ids); }
    void setDeletedOnUndo(std::vector<foundation::UUID> ids) { deletedOnUndo_ = std::move(ids); }
    void setAffectedOnUndo(std::vector<foundation::UUID> ids) { affectedOnUndo_ = std::move(ids); }

private:
    std::string              name_;
    std::vector<std::string>*log_;
    std::vector<foundation::UUID> affectedOnExec_;
    std::vector<foundation::UUID> deletedOnExec_;
    std::vector<foundation::UUID> affectedOnUndo_;
    std::vector<foundation::UUID> deletedOnUndo_;
};

/// 可合并的 Stub（key 相同时合并）
class MergeableCmd : public command::Command {
public:
    explicit MergeableCmd(std::string key, int value, std::vector<std::string>* log = nullptr)
        : key_(std::move(key)), value_(value), log_(log) {}

    void execute(command::CommandContext&) override {
        if (log_) log_->push_back("exec:merge:" + key_ + "=" + std::to_string(value_));
        lastResult_.success = true;
    }

    void undo(command::CommandContext&) override {
        if (log_) log_->push_back("undo:merge:" + key_);
        lastResult_.success = true;
    }

    bool tryMerge(const Command& next) override {
        const auto* other = dynamic_cast<const MergeableCmd*>(&next);
        if (!other || other->key_ != key_) return false;
        value_ = other->value_; // 保留最新值
        return true;
    }

    std::string description() const override { return "merge:" + key_; }
    command::CommandType type() const override { return command::CommandType::SetProperty; }
    nlohmann::json toJson() const override { return {{"key", key_}, {"value", value_}}; }

    int value() const { return value_; }

private:
    std::string               key_;
    int                       value_;
    std::vector<std::string>* log_;
};

/// 执行时抛出异常的 Stub
class FailingCmd : public command::Command {
public:
    void execute(command::CommandContext&) override {
        throw std::runtime_error("FailingCmd: forced failure");
    }
    void undo(command::CommandContext&) override {}
    std::string description() const override { return "failing"; }
    command::CommandType type() const override { return command::CommandType::SetProperty; }
    nlohmann::json toJson() const override { return {}; }
};

// 空 context（不需要 Document/DependencyGraph）
static command::CommandContext makeCtx() { return {}; }

// ============================================================
// execute + canUndo/canRedo 基础测试
// ============================================================

TEST(CommandStack, InitialState) {
    command::CommandStack stack;
    EXPECT_FALSE(stack.canUndo());
    EXPECT_FALSE(stack.canRedo());
    EXPECT_EQ(stack.undoCount(), 0u);
    EXPECT_EQ(stack.redoCount(), 0u);
    EXPECT_TRUE(stack.nextUndoDescription().empty());
}

TEST(CommandStack, ExecutePushesUndoStack) {
    command::CommandStack stack;
    auto ctx = makeCtx();

    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    EXPECT_TRUE(stack.canUndo());
    EXPECT_FALSE(stack.canRedo());
    EXPECT_EQ(stack.undoCount(), 1u);
    EXPECT_EQ(stack.nextUndoDescription(), "A");
}

TEST(CommandStack, ExecuteMultiple) {
    command::CommandStack stack;
    auto ctx = makeCtx();

    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    stack.execute(std::make_unique<StubCmd>("B"), ctx);
    stack.execute(std::make_unique<StubCmd>("C"), ctx);

    EXPECT_EQ(stack.undoCount(), 3u);
    EXPECT_EQ(stack.nextUndoDescription(), "C");
}

TEST(CommandStack, ExecuteClearsRedoStack) {
    command::CommandStack stack;
    auto ctx = makeCtx();

    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    stack.execute(std::make_unique<StubCmd>("B"), ctx);
    stack.undo(ctx);
    EXPECT_EQ(stack.redoCount(), 1u);

    stack.execute(std::make_unique<StubCmd>("C"), ctx); // 新命令清空 redo
    EXPECT_EQ(stack.redoCount(), 0u);
    EXPECT_EQ(stack.undoCount(), 2u); // A + C
}

// ============================================================
// undo/redo 循环
// ============================================================

TEST(CommandStack, UndoRedo) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    std::vector<std::string> log;

    stack.execute(std::make_unique<StubCmd>("A", &log), ctx);
    stack.execute(std::make_unique<StubCmd>("B", &log), ctx);

    stack.undo(ctx);
    EXPECT_EQ(log.back(), "undo:B");
    EXPECT_EQ(stack.undoCount(), 1u);
    EXPECT_EQ(stack.redoCount(), 1u);

    stack.undo(ctx);
    EXPECT_EQ(log.back(), "undo:A");
    EXPECT_FALSE(stack.canUndo());

    stack.redo(ctx);
    EXPECT_EQ(log.back(), "exec:A");
    EXPECT_EQ(stack.undoCount(), 1u);

    stack.redo(ctx);
    EXPECT_EQ(log.back(), "exec:B");
    EXPECT_EQ(stack.undoCount(), 2u);
    EXPECT_FALSE(stack.canRedo());
}

TEST(CommandStack, UndoOnEmptyDoesNothing) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    EXPECT_NO_THROW(stack.undo(ctx));
}

TEST(CommandStack, RedoOnEmptyDoesNothing) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    EXPECT_NO_THROW(stack.redo(ctx));
}

// ============================================================
// openMacro / closeMacro 整体入栈
// ============================================================

TEST(CommandStack, MacroClosePushesAsOne) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    std::vector<std::string> log;

    stack.openMacro("MyMacro");
    EXPECT_TRUE(stack.isMacroOpen());

    // 宏内子命令不推入 undoStack_
    stack.execute(std::make_unique<StubCmd>("A", &log), ctx);
    stack.execute(std::make_unique<StubCmd>("B", &log), ctx);
    EXPECT_EQ(stack.undoCount(), 0u); // 宏未关闭，不入栈

    stack.closeMacro();
    EXPECT_FALSE(stack.isMacroOpen());
    EXPECT_EQ(stack.undoCount(), 1u); // 整体宏作为一个命令入栈

    // 子命令已在 execute 阶段各自执行（log 中有 exec:A、exec:B），不重复执行
    EXPECT_EQ(log.size(), 2u);
    EXPECT_EQ(log[0], "exec:A");
    EXPECT_EQ(log[1], "exec:B");
}

TEST(CommandStack, MacroUndoCallsChildrenUndo) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    std::vector<std::string> log;

    stack.openMacro("MyMacro");
    stack.execute(std::make_unique<StubCmd>("A", &log), ctx);
    stack.execute(std::make_unique<StubCmd>("B", &log), ctx);
    stack.closeMacro();

    log.clear();
    stack.undo(ctx);

    // MacroCommand::undo 逆序撤销子命令
    EXPECT_EQ(log.size(), 2u);
    EXPECT_EQ(log[0], "undo:B");
    EXPECT_EQ(log[1], "undo:A");
}

// ============================================================
// abortMacro 逆序回滚
// ============================================================

TEST(CommandStack, AbortMacroRollsBackChildren) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    std::vector<std::string> log;

    stack.openMacro("Abortable");
    stack.execute(std::make_unique<StubCmd>("X", &log), ctx);
    stack.execute(std::make_unique<StubCmd>("Y", &log), ctx);

    log.clear();
    stack.abortMacro(ctx);

    EXPECT_FALSE(stack.isMacroOpen());
    EXPECT_EQ(stack.undoCount(), 0u); // 宏未入栈

    // 逆序 undo：Y → X
    EXPECT_EQ(log.size(), 2u);
    EXPECT_EQ(log[0], "undo:Y");
    EXPECT_EQ(log[1], "undo:X");
}

TEST(CommandStack, AbortMacroWhenNotOpen) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    EXPECT_NO_THROW(stack.abortMacro(ctx));
}

// ============================================================
// markClean / isClean 边界行为
// ============================================================

TEST(CommandStack, InitialIsCleanWhenMarkCleanOnEmpty) {
    command::CommandStack stack;
    // 初始状态未调用 markClean，cleanTopId_ 为 null
    // isClean() = undoStack_.empty() && cleanTopId_.isNull() => true
    EXPECT_TRUE(stack.isClean());
}

TEST(CommandStack, DirtyAfterExecute) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    EXPECT_FALSE(stack.isClean());
}

TEST(CommandStack, MarkCleanAfterExecute) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    stack.markClean();
    EXPECT_TRUE(stack.isClean());
}

TEST(CommandStack, DirtyAfterUndoFromClean) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    stack.markClean();

    stack.undo(ctx); // undo 后栈顶变化，isClean() 应返回 false
    EXPECT_FALSE(stack.isClean());
}

TEST(CommandStack, CleanRestoredAfterRedo) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    stack.markClean();

    stack.undo(ctx);
    EXPECT_FALSE(stack.isClean());

    stack.redo(ctx); // 重做后栈顶恢复 clean 状态
    EXPECT_TRUE(stack.isClean());
}

TEST(CommandStack, DirtyAfterNewCommandFromClean) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    stack.markClean();

    stack.undo(ctx);
    stack.execute(std::make_unique<StubCmd>("B"), ctx); // 新分支
    EXPECT_FALSE(stack.isClean());
}

TEST(CommandStack, MarkCleanOnEmptyStack) {
    command::CommandStack stack;
    stack.markClean();
    EXPECT_TRUE(stack.isClean());

    auto ctx = makeCtx();
    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    EXPECT_FALSE(stack.isClean());
}

// ============================================================
// setMaxSize 截断最旧命令
// ============================================================

TEST(CommandStack, MaxSizeDefault) {
    command::CommandStack stack;
    EXPECT_EQ(stack.maxSize(), 1000u);
}

TEST(CommandStack, MaxSizeTruncatesOldest) {
    command::CommandStack stack;
    stack.setMaxSize(3);

    auto ctx = makeCtx();
    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    stack.execute(std::make_unique<StubCmd>("B"), ctx);
    stack.execute(std::make_unique<StubCmd>("C"), ctx);
    EXPECT_EQ(stack.undoCount(), 3u);

    stack.execute(std::make_unique<StubCmd>("D"), ctx); // 第4个，应丢弃 "A"
    EXPECT_EQ(stack.undoCount(), 3u);
    EXPECT_EQ(stack.nextUndoDescription(), "D");
}

TEST(CommandStack, SetMaxSizeShrinks) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    stack.execute(std::make_unique<StubCmd>("B"), ctx);
    stack.execute(std::make_unique<StubCmd>("C"), ctx);
    EXPECT_EQ(stack.undoCount(), 3u);

    stack.setMaxSize(2); // 立即截断
    EXPECT_EQ(stack.undoCount(), 2u);
    EXPECT_EQ(stack.nextUndoDescription(), "C");
}

// ============================================================
// sceneRemoveRequested 信号触发
// ============================================================

TEST(CommandStack, SceneRemoveOnExecute) {
    command::CommandStack stack;
    auto ctx = makeCtx();

    std::vector<std::string> removed;
    stack.sceneRemoveRequested.connect([&](const std::string& id) {
        removed.push_back(id);
    });

    foundation::UUID uid = foundation::UUID::generate();
    auto cmd = std::make_unique<StubCmd>("Del");
    cmd->setDeletedOnExec({uid});

    stack.execute(std::move(cmd), ctx);
    ASSERT_EQ(removed.size(), 1u);
    EXPECT_EQ(removed[0], uid.toString());
}

TEST(CommandStack, SceneRemoveOnUndo) {
    command::CommandStack stack;
    auto ctx = makeCtx();

    std::vector<std::string> removed;
    stack.sceneRemoveRequested.connect([&](const std::string& id) {
        removed.push_back(id);
    });

    foundation::UUID uid = foundation::UUID::generate();
    auto cmd = std::make_unique<StubCmd>("Create");
    cmd->setDeletedOnUndo({uid}); // undo 创建 = 删除场景节点

    stack.execute(std::move(cmd), ctx);
    removed.clear();

    stack.undo(ctx);
    ASSERT_EQ(removed.size(), 1u);
    EXPECT_EQ(removed[0], uid.toString());
}

// ============================================================
// tryMerge 合并（使用可合并的 Stub）
// ============================================================

TEST(CommandStack, TryMergeSameKey) {
    command::CommandStack stack;
    auto ctx = makeCtx();

    std::vector<std::string> log;
    stack.execute(std::make_unique<MergeableCmd>("x", 1, &log), ctx);
    EXPECT_EQ(stack.undoCount(), 1u);

    // 相同 key 的第二个命令应被合并（不新增栈条目，但合并成功后需检查栈数量）
    stack.execute(std::make_unique<MergeableCmd>("x", 2, &log), ctx);
    // 合并成功：undoStack_ 仍只有 1 个条目
    EXPECT_EQ(stack.undoCount(), 1u);
}

TEST(CommandStack, TryMergeDifferentKey) {
    command::CommandStack stack;
    auto ctx = makeCtx();

    stack.execute(std::make_unique<MergeableCmd>("x", 1), ctx);
    stack.execute(std::make_unique<MergeableCmd>("y", 2), ctx); // 不同 key，不合并
    EXPECT_EQ(stack.undoCount(), 2u);
}

// ============================================================
// stackChanged 信号与 commandCompleted 信号
// ============================================================

TEST(CommandStack, StackChangedSignal) {
    command::CommandStack stack;
    auto ctx = makeCtx();

    int changeCount = 0;
    stack.stackChanged.connect([&]() { ++changeCount; });

    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    EXPECT_EQ(changeCount, 1);

    stack.undo(ctx);
    EXPECT_EQ(changeCount, 2);

    stack.redo(ctx);
    EXPECT_EQ(changeCount, 3);
}

TEST(CommandStack, CommandCompletedSignal) {
    command::CommandStack stack;
    auto ctx = makeCtx();

    std::vector<foundation::UUID> received;
    stack.commandCompleted.connect([&](const std::vector<foundation::UUID>& ids) {
        received = ids;
    });

    foundation::UUID uid = foundation::UUID::generate();
    auto cmd = std::make_unique<StubCmd>("A");
    cmd->setAffectedOnExec({uid});

    stack.execute(std::move(cmd), ctx);
    ASSERT_EQ(received.size(), 1u);
    EXPECT_EQ(received[0], uid);
}

// ============================================================
// clear() 清空所有栈
// ============================================================

TEST(CommandStack, Clear) {
    command::CommandStack stack;
    auto ctx = makeCtx();

    stack.execute(std::make_unique<StubCmd>("A"), ctx);
    stack.execute(std::make_unique<StubCmd>("B"), ctx);
    stack.undo(ctx);

    stack.clear();
    EXPECT_EQ(stack.undoCount(), 0u);
    EXPECT_EQ(stack.redoCount(), 0u);
    EXPECT_FALSE(stack.canUndo());
    EXPECT_FALSE(stack.canRedo());
}

// ============================================================
// 宏内执行的命令不重新执行（closeMacro 直接入栈）
// ============================================================

TEST(CommandStack, MacroChildrenNotReExecutedOnClose) {
    command::CommandStack stack;
    auto ctx = makeCtx();
    std::vector<std::string> log;

    stack.openMacro("M");
    stack.execute(std::make_unique<StubCmd>("P", &log), ctx);
    stack.execute(std::make_unique<StubCmd>("Q", &log), ctx);

    // closeMacro 不触发任何额外 exec
    const std::size_t countBeforeClose = log.size();
    stack.closeMacro();
    EXPECT_EQ(log.size(), countBeforeClose); // closeMacro 不产生新日志条目
}
