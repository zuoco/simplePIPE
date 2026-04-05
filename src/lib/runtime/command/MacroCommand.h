// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "command/Command.h"

#include <memory>
#include <string>
#include <vector>

namespace command {

/// 宏命令：将多个子命令组合为一个原子操作
///
/// execute 语义：顺序执行所有子命令，累积影响范围到 lastResult_
/// undo 语义：逆序撤销所有子命令
/// 回滚语义：若子命令 execute 失败，自动逆序 undo 已执行的子命令，再重新抛出异常
class MacroCommand : public Command {
public:
    explicit MacroCommand(std::string desc);

    /// 追加子命令（建议在 execute 之前调用）
    void addCommand(std::unique_ptr<Command> cmd);

    /// 顺序执行所有子命令，失败时自动回滚
    void execute(CommandContext& ctx) override;

    /// 逆序撤销所有子命令
    void undo(CommandContext& ctx) override;

    std::string description() const override;

    /// 始终返回 CommandType::Macro
    CommandType type() const override;

    nlohmann::json toJson() const override;

    const std::vector<std::unique_ptr<Command>>& children() const;

private:
    std::string description_;
    std::vector<std::unique_ptr<Command>> children_;
};

} // namespace command
