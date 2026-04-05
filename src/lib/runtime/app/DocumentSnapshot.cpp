// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "app/DocumentSnapshot.h"

#include "model/PipeSpec.h"
#include "model/Route.h"
#include "model/Segment.h"

#include <algorithm>

namespace app {

namespace {

template<typename T>
const T* findById(const std::vector<T>& items, const foundation::UUID& id) {
    auto it = std::find_if(items.begin(), items.end(), [&](const T& item) {
        return item.id == id;
    });
    return it != items.end() ? &(*it) : nullptr;
}

template<typename T>
void sortByUuid(std::vector<T*>& items) {
    std::sort(items.begin(), items.end(), [](const T* lhs, const T* rhs) {
        return lhs->id().toString() < rhs->id().toString();
    });
}

} // namespace

const DependencyNodeSnapshot* DependencyGraphSnapshot::findNode(const foundation::UUID& id) const {
    return findById(nodes, id);
}

const PipeSpecSnapshot* DocumentSnapshot::findPipeSpec(const foundation::UUID& id) const {
    return findById(pipeSpecs, id);
}

const PipePointSnapshot* DocumentSnapshot::findPipePoint(const foundation::UUID& id) const {
    return findById(pipePoints, id);
}

const SegmentSnapshot* DocumentSnapshot::findSegment(const foundation::UUID& id) const {
    return findById(segments, id);
}

const RouteSnapshot* DocumentSnapshot::findRoute(const foundation::UUID& id) const {
    return findById(routes, id);
}

DocumentSnapshot makeDocumentSnapshot(const Document& document,
                                      const DependencyGraph& dependencyGraph) {
    DocumentSnapshot snapshot;
    snapshot.version = document.currentVersion();
    snapshot.name = document.name();

    auto pipeSpecs = document.findByType<model::PipeSpec>();
    sortByUuid(pipeSpecs);
    snapshot.pipeSpecs.reserve(pipeSpecs.size());
    for (const auto* spec : pipeSpecs) {
        snapshot.pipeSpecs.push_back(PipeSpecSnapshot{
            spec->id(),
            spec->name(),
            spec->fields()
        });
    }

    auto pipePoints = document.findByType<model::PipePoint>();
    sortByUuid(pipePoints);
    snapshot.pipePoints.reserve(pipePoints.size());
    for (const auto* point : pipePoints) {
        PipePointSnapshot pointSnapshot;
        pointSnapshot.id = point->id();
        pointSnapshot.name = point->name();
        pointSnapshot.type = point->type();
        pointSnapshot.position = point->position();
        pointSnapshot.typeParams = point->typeParams();
        if (auto spec = point->pipeSpec()) {
            pointSnapshot.pipeSpecId = spec->id();
        }
        snapshot.pipePoints.push_back(std::move(pointSnapshot));
    }

    auto segments = document.findByType<model::Segment>();
    sortByUuid(segments);
    snapshot.segments.reserve(segments.size());
    for (const auto* segment : segments) {
        SegmentSnapshot segmentSnapshot;
        segmentSnapshot.id = segment->id();
        segmentSnapshot.name = segment->name();
        segmentSnapshot.pointIds.reserve(segment->points().size());
        for (const auto& point : segment->points()) {
            segmentSnapshot.pointIds.push_back(point->id());
        }
        snapshot.segments.push_back(std::move(segmentSnapshot));
    }

    auto routes = document.findByType<model::Route>();
    sortByUuid(routes);
    snapshot.routes.reserve(routes.size());
    for (const auto* route : routes) {
        RouteSnapshot routeSnapshot;
        routeSnapshot.id = route->id();
        routeSnapshot.name = route->name();
        routeSnapshot.segmentIds.reserve(route->segments().size());
        for (const auto& segment : route->segments()) {
            routeSnapshot.segmentIds.push_back(segment->id());
        }
        snapshot.routes.push_back(std::move(routeSnapshot));
    }

    const auto nodes = dependencyGraph.allNodes();
    snapshot.dependencyGraph.nodes.reserve(nodes.size());
    for (const auto& id : nodes) {
        snapshot.dependencyGraph.nodes.push_back(DependencyNodeSnapshot{
            id,
            dependencyGraph.directDependencies(id),
            dependencyGraph.directDependents(id)
        });
    }
    snapshot.dependencyGraph.dirtyIds = dependencyGraph.collectDirty();

    return snapshot;
}

} // namespace app