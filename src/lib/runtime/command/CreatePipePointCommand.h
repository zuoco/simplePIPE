// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "command/Command.h"
#include "model/PipePoint.h"
#include "foundation/Types.h"

#include <nlohmann/json.hpp>
#include <limits>
#include <string>

namespace command {

/// 创建管点命令：在指定路由/段中创建管点，支持 undo/redo 和 JSON 序列化
///
/// execute：创建 PipePoint → 加入 Document → TopologyManager 追加/插入 → 注册 DependencyGraph 依赖
/// undo：TopologyManager 移除 → Document 移除 → DependencyGraph 移除
/// Tee 类型管点会自动创建分支段，undo 时自动清理
class CreatePipePointCommand : public Command {
public:
    /// 工厂方法：创建命令实例
    /// @param routeId    目标路由 UUID
    /// @param segmentId  目标段 UUID
    /// @param name       管点名称
    /// @param pointType  管点类型
    /// @param x, y, z    坐标
    /// @param pipeSpecId PipeSpec 的 UUID 字符串（空字符串表示无关联）
    /// @param insertIndex 插入位置（SIZE_MAX = 追加到末尾）
    static std::unique_ptr<CreatePipePointCommand> create(
        const foundation::UUID& routeId,
        const foundation::UUID& segmentId,
        const std::string& name,
        model::PipePointType pointType,
        double x, double y, double z,
        const std::string& pipeSpecId = "",
        std::size_t insertIndex = std::numeric_limits<std::size_t>::max());

    /// 从完整 JSON 反序列化（工厂注册用），createdPointId/createdBranchId 已知
    static std::unique_ptr<CreatePipePointCommand> fromJson(const nlohmann::json& j);

    // ---- Command 接口 ----
    void execute(CommandContext& ctx) override;
    void undo(CommandContext& ctx) override;
    std::string description() const override;
    CommandType type() const override;
    nlohmann::json toJson() const override;

    // ---- 访问器（测试用）----
    const foundation::UUID& routeId() const { return routeId_; }
    const foundation::UUID& segmentId() const { return segmentId_; }
    const foundation::UUID& createdPointId() const { return createdPointId_; }
    const foundation::UUID& createdBranchId() const { return createdBranchId_; }
    const std::string& pointName() const { return name_; }
    model::PipePointType pointType() const { return pointType_; }

private:
    CreatePipePointCommand() = default;

    // 输入参数
    foundation::UUID    routeId_;
    foundation::UUID    segmentId_;
    std::string         name_;
    model::PipePointType pointType_ = model::PipePointType::Run;
    double              x_ = 0.0, y_ = 0.0, z_ = 0.0;
    std::string         pipeSpecId_;
    std::size_t         insertIndex_ = std::numeric_limits<std::size_t>::max();

    // 执行后填充
    foundation::UUID    createdPointId_;
    foundation::UUID    createdBranchId_;  ///< Tee 创建的分支段 UUID（空 if not Tee）
    bool                executed_ = false;
};

} // namespace command
