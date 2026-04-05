// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "command/Command.h"
#include "foundation/Types.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace command {

/// 批量属性修改命令：将多个属性变更作为一个原子 undo 单元提交
///
/// 适用于已知全部变更的程序化批量写入（如 UI 表格多列同时提交）。
/// 若是交互式逐步构建的多步操作，应使用 openMacro/closeMacro 模式。
///
/// - execute：顺序 apply 各变更；任何步骤失败则回滚已 apply 的修改
/// - undo：逆序恢复各对象旧值
/// - 不实现 tryMerge（批量命令不参与合并）
class BatchSetPropertyCommand : public Command {
public:
    /// 单条变更描述
    struct Change {
        foundation::UUID    objectId;
        std::string         key;
        foundation::Variant oldValue;
        foundation::Variant newValue;
    };

    /// 构造函数
    explicit BatchSetPropertyCommand(std::string description,
                                     std::vector<Change> changes);

    // ---- Command 接口 ----
    void execute(CommandContext& ctx) override;
    void undo(CommandContext& ctx) override;
    std::string description() const override;
    CommandType type() const override;
    nlohmann::json toJson() const override;

    // ---- 访问器（测试用）----
    const std::vector<Change>& changes() const { return changes_; }

private:
    std::string        description_;
    std::vector<Change> changes_;
};

} // namespace command
