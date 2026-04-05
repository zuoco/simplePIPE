// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "command/CommandContext.h"
#include "command/CommandResult.h"
#include "command/CommandType.h"
#include "foundation/Types.h"

#include <nlohmann/json.hpp>
#include <chrono>
#include <string>

namespace command {

/// 命令抽象基类 — 所有可撤销操作的统一接口
///
/// 设计原则：
/// - 命令只存储 UUID 和参数，不持有 Document/DependencyGraph 指针
/// - 执行时通过 CommandContext 获取运行时依赖，确保可序列化、可重放
/// - undo() 必须在 lastResult_ 中设置 deletedIds 和 affectedIds
class Command {
public:
    virtual ~Command() = default;

    /// 执行命令（正向变更）
    virtual void execute(CommandContext& ctx) = 0;

    /// 撤销命令（恢复到执行前状态）
    virtual void undo(CommandContext& ctx) = 0;

    /// 人类可读描述（显示在 undo 栈 UI 中）
    virtual std::string description() const = 0;

    /// 命令类型标识（用于序列化分派）
    virtual CommandType type() const = 0;

    /// 序列化为 JSON（包含 params + 执行后 state）
    virtual nlohmann::json toJson() const = 0;

    /// 尝试将后续命令合并到本命令（用于连续输入合并，如拖拽坐标修改）
    /// 返回 true 表示合并成功，CommandStack 不再 push 后续命令
    virtual bool tryMerge(const Command& /*next*/) { return false; }

    /// 命令实例唯一 ID
    const foundation::UUID& id() const { return id_; }

    /// 最近一次执行/撤销的结果（仅 execute/undo 后有效）
    const CommandResult& lastResult() const { return lastResult_; }

    /// 创建时间戳（用于 tryMerge 时间窗口判断）
    const std::chrono::steady_clock::time_point& timestamp() const { return timestamp_; }

protected:
    Command() = default;

    foundation::UUID id_             = foundation::UUID::generate();
    CommandResult    lastResult_;
    std::chrono::steady_clock::time_point timestamp_ = std::chrono::steady_clock::now();
};

} // namespace command
