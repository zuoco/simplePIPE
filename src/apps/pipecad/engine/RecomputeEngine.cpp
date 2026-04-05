// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/RecomputeEngine.h"

#include <algorithm>

namespace engine {

// ——— RecomputeEngine 成员实现 ———

RecomputeEngine::RecomputeEngine(app::Document& doc, app::DependencyGraph& graph)
    : doc_(doc), graph_(graph) {}

void RecomputeEngine::setSceneUpdateCallback(SceneUpdateCallback cb) {
    sceneCb_ = std::move(cb);
}

void RecomputeEngine::enableAsyncMode(AsyncFn asyncFn, DrainFn drainFn) {
    asyncFn_ = std::move(asyncFn);
    drainFn_ = std::move(drainFn);
}

void RecomputeEngine::recompute(const std::vector<foundation::UUID>& dirtyIds) {
    for (const auto& id : dirtyIds) {
        model::DocumentObject* obj = doc_.findObject(id);
        if (!obj) continue;

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

void RecomputeEngine::asyncRecompute() {
    // 异步模式未启用时退化为同步路径
    if (!asyncFn_) {
        auto dirtyIds = graph_.collectDirty();
        recompute(dirtyIds);
        graph_.clearDirty();
        return;
    }

    // 有脏对象时调用注入的异步管线函数
    // asyncFn_() 由 main.cpp 提供，负责：
    //   1. makeDocumentSnapshot(doc, graph) → 构建只读快照（捕获脏 ID）
    //   2. graph.clearDirty()              → 清除脏标记
    //   3. workers.submit(task)            → 提交后台几何推导
    //   4. [后台] channel.post(v, fn)      → 结果回投
    if (!graph_.collectDirty().empty()) {
        asyncFn_();
    }
}

std::size_t RecomputeEngine::drainResults() {
    if (!drainFn_) return 0;
    return drainFn_();
}

void RecomputeEngine::recomputeAll() {
    auto segments = doc_.allSegments();
    for (auto* seg : segments) {
        for (auto& pp : seg->points()) {
            recomputePoint(pp);
        }
    }
}

void RecomputeEngine::asyncRecomputeAll() {
    // 若异步模式未启用，退化为同步全量重算
    if (!asyncFn_) {
        recomputeAll();
        return;
    }

    // 将所有 PipePoint 标记为脏（主线程执行，T70 快照窗口协议前置步骤）
    auto segments = doc_.allSegments();
    for (auto* seg : segments) {
        for (auto& pp : seg->points()) {
            graph_.markDirty(pp->id());
        }
    }

    // 触发异步重算管线（快照构建 → 后台提交 → 结果回投）
    asyncRecompute();
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
