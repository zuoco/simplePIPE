// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "command/Command.h"
#include "foundation/Types.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace command {

/// 属性修改命令：对单个文档对象的单个属性进行修改，支持 undo/redo 和连续输入合并
///
/// 通过两个静态工厂方法创建：
/// - createAutoCapture: execute 时自动从对象读取 oldValue
/// - createWithOldValue: 调用方已知 oldValue（反序列化场景）
class SetPropertyCommand : public Command {
public:
    /// 构造工厂：execute 时自动捕获 oldValue
    static std::unique_ptr<SetPropertyCommand> createAutoCapture(
        const foundation::UUID& objectId,
        const std::string& key,
        const foundation::Variant& newValue);

    /// 构造工厂：调用方显式提供 oldValue（反序列化场景）
    static std::unique_ptr<SetPropertyCommand> createWithOldValue(
        const foundation::UUID& objectId,
        const std::string& key,
        const foundation::Variant& oldValue,
        const foundation::Variant& newValue);

    // ---- Command 接口 ----
    void execute(CommandContext& ctx) override;
    void undo(CommandContext& ctx) override;
    std::string description() const override;
    CommandType type() const override;
    nlohmann::json toJson() const override;

    /// tryMerge：objectId + key 相同且时间差 < 500ms 则合并（更新 newValue_，保留最旧 oldValue）
    bool tryMerge(const Command& next) override;

    // ---- 访问器（测试和序列化用）----
    const foundation::UUID& objectId() const { return objectId_; }
    const std::string& key() const { return key_; }
    const std::optional<foundation::Variant>& oldValue() const { return oldValue_; }
    const foundation::Variant& newValue() const { return newValue_; }

private:
    SetPropertyCommand() = default;

    foundation::UUID              objectId_;
    std::string                   key_;
    std::optional<foundation::Variant> oldValue_;   ///< 空 = 待 execute 时自动捕获
    foundation::Variant           newValue_;
};

} // namespace command
