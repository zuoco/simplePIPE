// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/DependencyGraph.h"
#include "app/Document.h"
#include "model/PipePoint.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace app {

/// 冻结的 PipeSpec 值对象，仅保留后台任务需要的稳定字段。
struct PipeSpecSnapshot {
    foundation::UUID id;
    std::string name;
    std::map<std::string, foundation::Variant> fields;
};

/// 冻结的 PipePoint 值对象，避免后台线程直接访问可变文档对象。
struct PipePointSnapshot {
    foundation::UUID id;
    std::string name;
    model::PipePointType type = model::PipePointType::Run;
    gp_Pnt position;
    std::optional<foundation::UUID> pipeSpecId;
    std::map<std::string, foundation::Variant> typeParams;
};

struct SegmentSnapshot {
    foundation::UUID id;
    std::string name;
    std::vector<foundation::UUID> pointIds;
};

struct RouteSnapshot {
    foundation::UUID id;
    std::string name;
    std::vector<foundation::UUID> segmentIds;
};

/// 依赖图的只读节点视图，同时冻结双向边关系。
struct DependencyNodeSnapshot {
    foundation::UUID id;
    std::vector<foundation::UUID> dependsOn;
    std::vector<foundation::UUID> dependents;
};

struct DependencyGraphSnapshot {
    std::vector<DependencyNodeSnapshot> nodes;
    std::vector<foundation::UUID> dirtyIds;

    const DependencyNodeSnapshot* findNode(const foundation::UUID& id) const;
};

/// 后台任务消费的只读文档快照。
struct DocumentSnapshot {
    DocumentVersion version = 0;
    std::string name;
    std::vector<PipeSpecSnapshot> pipeSpecs;
    std::vector<PipePointSnapshot> pipePoints;
    std::vector<SegmentSnapshot> segments;
    std::vector<RouteSnapshot> routes;
    DependencyGraphSnapshot dependencyGraph;

    const PipeSpecSnapshot* findPipeSpec(const foundation::UUID& id) const;
    const PipePointSnapshot* findPipePoint(const foundation::UUID& id) const;
    const SegmentSnapshot* findSegment(const foundation::UUID& id) const;
    const RouteSnapshot* findRoute(const foundation::UUID& id) const;
};

/// 在主线程同步构建只读快照，作为后台任务的唯一输入面。
///
/// **[主线程独占]** 必须在主线程调用，且需在 DependencyGraph::collectDirty()
/// 与 DependencyGraph::clearDirty() 之间完成（原子序列，T70 同步策略）：
///
///   1. dirtyIds = graph.collectDirty()       // 主线程
///   2. snap = makeDocumentSnapshot(doc, graph) // 主线程（捕获 dirtyIds）
///   3. graph.clearDirty()                    // 主线程
///   4. workers.submit(snap, ...)             // 提交后台任务
///
/// 快照完成后，后台线程持有只读副本，主线程不再需要锁保护文档对象。
DocumentSnapshot makeDocumentSnapshot(const Document& document,
                                      const DependencyGraph& dependencyGraph);

} // namespace app