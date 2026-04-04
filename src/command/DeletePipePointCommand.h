// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "command/Command.h"
#include "model/PipePoint.h"
#include "foundation/Types.h"

#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <vector>

namespace command {

/// 管点快照：用于 DeletePipePointCommand 的 undo 恢复
struct PipePointState {
    foundation::UUID    id;
    std::string         name;
    model::PipePointType type = model::PipePointType::Run;
    double              x = 0.0, y = 0.0, z = 0.0;
    std::string         pipeSpecId;
    std::map<std::string, foundation::Variant> typeParams;
    foundation::UUID    routeId;
    foundation::UUID    segmentId;
    std::size_t         indexInSegment = 0;
    std::string         branchSegmentId;  ///< Tee 关联的分支段 UUID（空字符串 = 无分支）

    /// 附属构件状态
    struct AccessoryState {
        foundation::UUID id;
        std::string      name;
    };
    std::vector<AccessoryState> accessories;
};

/// 删除管点命令：捕获完整管点状态 → 删除 → undo 时用 setIdForDeserialization() 恢复原 UUID
///
/// execute：捕获快照 → TopologyManager 移除 → Document 移除 → DependencyGraph 移除
/// undo：重建 PipePoint（恢复 UUID）→ Document 添加 → 插入回原段位置 → 重建分支/附属构件 → 注册依赖
class DeletePipePointCommand : public Command {
public:
    /// 工厂方法
    static std::unique_ptr<DeletePipePointCommand> create(const foundation::UUID& pointId);

    /// 从完整 JSON 反序列化
    static std::unique_ptr<DeletePipePointCommand> fromJson(const nlohmann::json& j);

    // ---- Command 接口 ----
    void execute(CommandContext& ctx) override;
    void undo(CommandContext& ctx) override;
    std::string description() const override;
    CommandType type() const override;
    nlohmann::json toJson() const override;

    // ---- 访问器（测试用）----
    const foundation::UUID& pointId() const { return pointId_; }
    const PipePointState& savedState() const { return savedState_; }

private:
    DeletePipePointCommand() = default;

    foundation::UUID  pointId_;
    PipePointState    savedState_;
    bool              captured_ = false;
};

} // namespace command
