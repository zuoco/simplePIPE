#pragma once

#include "model/Route.h"
#include "model/Segment.h"
#include "model/PipePoint.h"
#include "foundation/Types.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

/// 维护 Route 内的段间连接关系，管点增删时保持 Segment/Route 的一致性。
/// 三通 (Tee) 管点添加时自动在 Route 中创建空白分支 Segment。
class TopologyManager {
public:
    TopologyManager() = default;

    /// 向 seg 末尾追加管点。
    /// 若 point 类型为 Tee，在 route 中创建以该 Tee 命名的空白分支 Segment，
    /// 并返回该分支 Segment 的 shared_ptr；否则返回 nullptr。
    std::shared_ptr<model::Segment> appendPoint(
        model::Route& route,
        model::Segment& seg,
        std::shared_ptr<model::PipePoint> point);

    /// 在 seg 的 index 位置插入管点。若 point 为 Tee，创建分支 Segment。
    std::shared_ptr<model::Segment> insertPoint(
        model::Route& route,
        model::Segment& seg,
        std::size_t index,
        std::shared_ptr<model::PipePoint> point);

    /// 从 route 的所有 Segment 中删除指定管点。
    /// 若该点是 Tee 且有关联分支 Segment，同时从 route 中删除该分支。
    /// 空 Segment（除保留至少一个）会被自动清除。
    /// @return 是否找到并删除
    bool removePoint(model::Route& route, const foundation::UUID& pointId);

    /// 查询 route 中所有包含指定管点的 Segment（原始指针，生命周期由 route 管理）
    std::vector<model::Segment*> segmentsContaining(
        const model::Route& route,
        const foundation::UUID& pointId) const;

    /// 获取 Tee 管点对应的分支 Segment ID 字符串（未找到则返回空字符串）
    std::string branchSegmentId(const foundation::UUID& teeId) const;

private:
    /// Tee UUID.toString() → branch Segment UUID.toString()
    std::unordered_map<std::string, std::string> teeToBranch_;

    std::shared_ptr<model::Segment> createBranchForTee(
        model::Route& route,
        const model::PipePoint& tee);
};

} // namespace engine
