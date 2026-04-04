// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "command/Command.h"
#include "command/CommandResult.h"
#include "command/MacroCommand.h"
#include "foundation/Signal.h"
#include "foundation/Types.h"

#include <memory>
#include <string>
#include <vector>

namespace command {

/// 命令栈管理器 — 管理命令的执行、撤销、重做
///
/// 不持有 Document&、DependencyGraph& — 通过信号解耦
/// 必须在主线程调用 execute/undo/redo（OCCT 非线程安全）
class CommandStack {
public:
    // ---- 信号 ----
    foundation::Signal<>                                          stackChanged;        ///< 栈状态变化（UI 刷新）
    foundation::Signal<const std::vector<foundation::UUID>&>      commandCompleted;    ///< 命令执行后（含 affectedIds）
    foundation::Signal<const std::vector<foundation::UUID>&>      commandUndone;       ///< 命令撤销后
    foundation::Signal<const std::vector<foundation::UUID>&>      commandRedone;       ///< 命令重做后
    foundation::Signal<>                                          cleanStateChanged;   ///< clean 标记变化
    foundation::Signal<const std::string&>                        sceneRemoveRequested; ///< 请求移除场景节点（来自 lastResult_.deletedIds）

    explicit CommandStack();

    /// 执行命令：尝试合并 → cmd->execute → 入 undo 栈 → 清空 redo → emit 信号
    /// 若 macroOpen_，命令加入 pendingMacro_（不入栈）
    CommandResult execute(std::unique_ptr<Command> cmd, CommandContext& ctx);

    /// 撤销最近命令
    void undo(CommandContext& ctx);

    /// 重做最近撤销
    void redo(CommandContext& ctx);

    // ---- 交互式宏 ----
    void openMacro(const std::string& description);
    void closeMacro();
    void abortMacro(CommandContext& ctx);  ///< 丢弃待定宏，逆序 undo 已执行子命令
    bool isMacroOpen() const;

    // ---- 查询 ----
    bool        canUndo() const;
    bool        canRedo() const;
    std::string nextUndoDescription() const;
    std::size_t undoCount() const;
    std::size_t redoCount() const;
    void        clear();

    // ---- Clean 标记（文档保存状态）----
    void markClean();   ///< 保存时调用，记录当前栈顶命令 UUID
    bool isClean() const; ///< 当前栈顶 UUID 是否等于 clean 时记录的 UUID

    // ---- 栈深度限制 ----
    void        setMaxSize(std::size_t maxSize); ///< 默认 1000，超出时丢弃最旧命令
    std::size_t maxSize() const;

private:
    std::vector<std::unique_ptr<Command>> undoStack_;
    std::vector<std::unique_ptr<Command>> redoStack_;
    std::unique_ptr<MacroCommand>         pendingMacro_;
    bool                                  macroOpen_ = false;

    std::size_t      maxSize_    = 1000;
    foundation::UUID cleanTopId_; ///< markClean 时记录的栈顶命令 UUID（isNull = 初始 clean）

    /// 遍历 lastResult_.deletedIds 逐一 emit sceneRemoveRequested
    void emitSceneRemove(const CommandResult& result);

    /// 超出 maxSize_ 时丢弃最旧命令
    void trimToMaxSize();
};

} // namespace command
