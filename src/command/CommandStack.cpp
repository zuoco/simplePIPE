// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "command/CommandStack.h"

#include <stdexcept>

namespace command {

CommandStack::CommandStack() = default;

CommandResult CommandStack::execute(std::unique_ptr<Command> cmd, CommandContext& ctx)
{
    if (!cmd) {
        throw std::invalid_argument("CommandStack::execute: null command");
    }

    // ---- 宏模式：执行后加入 pendingMacro_，不推入 undoStack_ ----
    if (macroOpen_ && pendingMacro_) {
        cmd->execute(ctx);
        CommandResult result = cmd->lastResult();
        pendingMacro_->addCommand(std::move(cmd));
        emitSceneRemove(result);
        // 宏模式下不 emit commandCompleted / stackChanged（等 closeMacro 后整体 emit）
        return result;
    }

    // ---- 正常模式：尝试合并 ----
    if (!undoStack_.empty()) {
        Command& top = *undoStack_.back();
        if (top.tryMerge(*cmd)) {
            // 合并成功：重新执行 top（tryMerge 已更新参数），不再 push 新命令
            // 注意：SetPropertyCommand::tryMerge 只合并参数，实际 execute 需调用方重新触发
            // 此处按规格：合并后不再 push，直接返回（top 本身已持有最新值）
            CommandResult merged = top.lastResult();
            redoStack_.clear();
            emitSceneRemove(merged);
            commandCompleted.emit(merged.affectedIds);
            stackChanged.emit();
            return merged;
        }
    }

    // ---- 正常模式：执行并入栈 ----
    cmd->execute(ctx);
    CommandResult result = cmd->lastResult();

    redoStack_.clear();
    undoStack_.push_back(std::move(cmd));
    trimToMaxSize();

    emitSceneRemove(result);
    commandCompleted.emit(result.affectedIds);
    stackChanged.emit();

    return result;
}

void CommandStack::undo(CommandContext& ctx)
{
    if (undoStack_.empty()) return;

    auto cmd = std::move(undoStack_.back());
    undoStack_.pop_back();

    cmd->undo(ctx);
    CommandResult result = cmd->lastResult();

    redoStack_.push_back(std::move(cmd));

    emitSceneRemove(result);
    commandUndone.emit(result.affectedIds);
    stackChanged.emit();
}

void CommandStack::redo(CommandContext& ctx)
{
    if (redoStack_.empty()) return;

    auto cmd = std::move(redoStack_.back());
    redoStack_.pop_back();

    // redo 不调用 tryMerge
    cmd->execute(ctx);
    CommandResult result = cmd->lastResult();

    undoStack_.push_back(std::move(cmd));

    emitSceneRemove(result);
    commandRedone.emit(result.affectedIds);
    stackChanged.emit();
}

// ---- 交互式宏 ----

void CommandStack::openMacro(const std::string& description)
{
    if (macroOpen_) return; // 防止重复打开
    pendingMacro_ = std::make_unique<MacroCommand>(description);
    macroOpen_    = true;
}

void CommandStack::closeMacro()
{
    if (!macroOpen_ || !pendingMacro_) return;

    macroOpen_ = false;
    // 子命令已在 execute 阶段各自执行，不重新执行，直接整体入栈
    redoStack_.clear();

    // 收集所有子命令的 affectedIds 作为宏的整体 affectedIds
    std::vector<foundation::UUID> allAffected;
    for (const auto& child : pendingMacro_->children()) {
        const auto& ids = child->lastResult().affectedIds;
        allAffected.insert(allAffected.end(), ids.begin(), ids.end());
    }

    undoStack_.push_back(std::move(pendingMacro_));
    pendingMacro_.reset();
    trimToMaxSize();

    commandCompleted.emit(allAffected);
    stackChanged.emit();
}

void CommandStack::abortMacro(CommandContext& ctx)
{
    if (!macroOpen_ || !pendingMacro_) return;

    macroOpen_ = false;
    const auto& children = pendingMacro_->children();
    // 逆序 undo 已执行子命令
    for (std::size_t i = children.size(); i > 0; --i) {
        try {
            children[i - 1]->undo(ctx);
        } catch (...) {
            // 回滚失败时记录日志，不重新抛出
        }
    }
    pendingMacro_.reset();
    stackChanged.emit();
}

bool CommandStack::isMacroOpen() const
{
    return macroOpen_;
}

// ---- 查询 ----

bool CommandStack::canUndo() const
{
    return !undoStack_.empty();
}

bool CommandStack::canRedo() const
{
    return !redoStack_.empty();
}

std::string CommandStack::nextUndoDescription() const
{
    if (undoStack_.empty()) return {};
    return undoStack_.back()->description();
}

std::size_t CommandStack::undoCount() const
{
    return undoStack_.size();
}

std::size_t CommandStack::redoCount() const
{
    return redoStack_.size();
}

void CommandStack::clear()
{
    undoStack_.clear();
    redoStack_.clear();
}

// ---- Clean 标记 ----

void CommandStack::markClean()
{
    bool wasClean = isClean();
    if (undoStack_.empty()) {
        cleanTopId_ = foundation::UUID{}; // isNull => 初始状态为 clean
    } else {
        cleanTopId_ = undoStack_.back()->id();
    }
    if (!wasClean) {
        cleanStateChanged.emit();
    }
}

bool CommandStack::isClean() const
{
    if (undoStack_.empty()) {
        return cleanTopId_.isNull();
    }
    return undoStack_.back()->id() == cleanTopId_;
}

// ---- 栈深度限制 ----

void CommandStack::setMaxSize(std::size_t maxSize)
{
    maxSize_ = maxSize;
    trimToMaxSize();
}

std::size_t CommandStack::maxSize() const
{
    return maxSize_;
}

// ---- 私有辅助 ----

void CommandStack::emitSceneRemove(const CommandResult& result)
{
    for (const auto& id : result.deletedIds) {
        sceneRemoveRequested.emit(id.toString());
    }
}

void CommandStack::trimToMaxSize()
{
    while (undoStack_.size() > maxSize_) {
        undoStack_.erase(undoStack_.begin());
    }
}

} // namespace command
