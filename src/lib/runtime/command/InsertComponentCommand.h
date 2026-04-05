// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "command/MacroCommand.h"
#include "model/PipePoint.h"
#include "foundation/Types.h"

#include <nlohmann/json.hpp>
#include <string>

namespace command {

/// 插入组件命令：MacroCommand 子类，type() 统一返回 Macro
///
/// componentType 字符串到 PipePointType 映射：
///   "insert-pipe"     → Run
///   "insert-elbow"    → Bend
///   "insert-tee"      → Tee
///   "insert-reducer"  → Reducer
///   "insert-valve"    → Valve
///
/// 构造时自动组合 CreatePipePointCommand 作为子命令。
/// toJson() 额外输出 componentType 字段，反序列化时由 CommandRegistry 检查此字段路由。
class InsertComponentCommand : public MacroCommand {
public:
    /// 工厂方法：创建 InsertComponentCommand 并组合子命令
    /// @param componentType  组件类型字符串（如 "insert-pipe"）
    /// @param routeId        目标路由 UUID
    /// @param segmentId      目标段 UUID
    /// @param x, y, z        插入位置坐标
    /// @param pipeSpecId     PipeSpec UUID 字符串（空 = 无关联）
    /// @param insertIndex    插入位置（SIZE_MAX = 追加到末尾）
    static std::unique_ptr<InsertComponentCommand> create(
        const std::string& componentType,
        const foundation::UUID& routeId,
        const foundation::UUID& segmentId,
        double x, double y, double z,
        const std::string& pipeSpecId = "",
        std::size_t insertIndex = std::numeric_limits<std::size_t>::max());

    /// 从完整 JSON 反序列化（CommandRegistry 工厂用）
    static std::unique_ptr<InsertComponentCommand> fromJson(const nlohmann::json& j);

    /// toJson() 额外输出 componentType 字段
    nlohmann::json toJson() const override;

    // ---- 访问器（测试用）----
    const std::string& componentType() const { return componentType_; }

    /// componentType → PipePointType 映射
    static model::PipePointType mapComponentType(const std::string& componentType);

private:
    explicit InsertComponentCommand(std::string componentType, std::string desc);

    std::string componentType_;
};

} // namespace command
