#include "engine/TopologyManager.h"

namespace engine {

std::shared_ptr<model::Segment> TopologyManager::appendPoint(
    model::Route& route,
    model::Segment& seg,
    std::shared_ptr<model::PipePoint> point)
{
    seg.addPoint(point);
    if (point->type() == model::PipePointType::Tee) {
        return createBranchForTee(route, *point);
    }
    return nullptr;
}

std::shared_ptr<model::Segment> TopologyManager::insertPoint(
    model::Route& route,
    model::Segment& seg,
    std::size_t index,
    std::shared_ptr<model::PipePoint> point)
{
    seg.insertPoint(index, point);
    if (point->type() == model::PipePointType::Tee) {
        return createBranchForTee(route, *point);
    }
    return nullptr;
}

bool TopologyManager::removePoint(model::Route& route, const foundation::UUID& pointId)
{
    bool removed = false;
    for (const auto& seg : route.segments()) {
        if (seg->removePoint(pointId)) {
            removed = true;
        }
    }
    if (!removed) return false;

    // If the deleted point was a Tee, also remove its branch segment
    auto it = teeToBranch_.find(pointId.toString());
    if (it != teeToBranch_.end()) {
        const std::string& branchIdStr = it->second;
        for (const auto& seg : route.segments()) {
            if (seg->id().toString() == branchIdStr) {
                route.removeSegment(seg->id());
                break;
            }
        }
        teeToBranch_.erase(it);
    }

    // Remove empty segments while keeping at least one
    bool anyRemoved = true;
    while (anyRemoved && route.segmentCount() > 1) {
        anyRemoved = false;
        for (const auto& seg : route.segments()) {
            if (seg->pointCount() == 0) {
                route.removeSegment(seg->id());
                anyRemoved = true;
                break;
            }
        }
    }

    return true;
}

std::vector<model::Segment*> TopologyManager::segmentsContaining(
    const model::Route& route,
    const foundation::UUID& pointId) const
{
    std::vector<model::Segment*> result;
    for (const auto& seg : route.segments()) {
        for (const auto& pt : seg->points()) {
            if (pt->id() == pointId) {
                result.push_back(seg.get());
                break;
            }
        }
    }
    return result;
}

std::string TopologyManager::branchSegmentId(const foundation::UUID& teeId) const
{
    auto it = teeToBranch_.find(teeId.toString());
    return it != teeToBranch_.end() ? it->second : std::string{};
}

std::shared_ptr<model::Segment> TopologyManager::createBranchForTee(
    model::Route& route,
    const model::PipePoint& tee)
{
    auto branch = std::make_shared<model::Segment>("Branch_" + tee.name());
    route.addSegment(branch);
    teeToBranch_[tee.id().toString()] = branch->id().toString();
    return branch;
}

} // namespace engine
