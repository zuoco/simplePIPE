// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/RecomputeEngine.h"

namespace engine {

RecomputeEngine::RecomputeEngine(app::Document& doc, app::DependencyGraph& graph)
    : doc_(doc), graph_(graph) {}

void RecomputeEngine::setSceneUpdateCallback(SceneUpdateCallback cb) {
    sceneCb_ = std::move(cb);
}

void RecomputeEngine::recompute(const std::vector<foundation::UUID>& dirtyIds) {
    for (const auto& id : dirtyIds) {
        model::DocumentObject* obj = doc_.findObject(id);
        if (!obj) continue;

        // 通过 Document 找到对应的 shared_ptr
        auto segments = doc_.allSegments();
        for (auto* seg : segments) {
            for (auto& pp : seg->points()) {
                if (pp->id() == id) {
                    recomputePoint(pp);
                    break;
                }
            }
        }
    }
}

void RecomputeEngine::recomputeAll() {
    auto segments = doc_.allSegments();
    for (auto* seg : segments) {
        for (auto& pp : seg->points()) {
            recomputePoint(pp);
        }
    }
}

RecomputeEngine::Neighbors RecomputeEngine::findNeighbors(
    const model::PipePoint* pp) const {
    Neighbors nb;
    auto segments = doc_.allSegments();
    for (auto* seg : segments) {
        const auto& pts = seg->points();
        for (std::size_t i = 0; i < pts.size(); ++i) {
            if (pts[i].get() == pp) {
                nb.prev = (i > 0) ? pts[i - 1]->position() : pp->position();
                nb.next = (i + 1 < pts.size()) ? pts[i + 1]->position() : pp->position();
                nb.valid = true;
                return nb;
            }
        }
    }
    return nb;
}

void RecomputeEngine::recomputePoint(const std::shared_ptr<model::PipePoint>& pp) {
    Neighbors nb = findNeighbors(pp.get());
    if (!nb.valid) {
        nb.prev = pp->position();
        nb.next = pp->position();
    }

    TopoDS_Shape shape = GeometryDeriver::deriveGeometry(nb.prev, pp, nb.next);

    if (sceneCb_ && !shape.IsNull()) {
        sceneCb_(pp->id().toString(), shape);
    }
}

} // namespace engine
