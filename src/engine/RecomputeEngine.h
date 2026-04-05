// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0
//
// T62 兼容 shim：物理迁移后此文件作为转发头，内容与 src/apps/pipecad/engine/RecomputeEngine.h 保持同步。

#pragma once

#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "engine/GeometryDeriver.h"
#include "foundation/Types.h"
#include "model/PipePoint.h"
#include "model/Segment.h"

#include <TopoDS_Shape.hxx>
#include <functional>
#include <string>
#include <vector>

namespace engine {

/// 批量重算引擎：收集脏对象 → 推导几何 → 通知场景更新
///
/// 为避免与 visualization 层的循环依赖，RecomputeEngine 通过回调通知场景：
///   setSceneUpdateCallback([&](const std::string& uuid, const TopoDS_Shape& shape) {
///       sceneMgr.updateNode(uuid, ...);
///   });
///
/// ## 线程所有权（T70 同步策略）
///
/// **[主线程独占]** 本类所有公共方法仅允许在主线程调用。
///
/// ## 异步模式（T71）
///
/// 通过 enableAsyncMode() 注入三个类型擦除函数后，
/// asyncRecompute() 遵循 T70 快照窗口协议执行异步管线。
class RecomputeEngine {
public:
    /// 场景更新回调类型：(对象UUID字符串, 新几何体)
    using SceneUpdateCallback = std::function<void(const std::string&, const TopoDS_Shape&)>;

    /// 异步执行函数类型（T71）
    using AsyncFn = std::function<void()>;

    /// 结果消费函数类型（T71）
    using DrainFn = std::function<std::size_t()>;

    RecomputeEngine(app::Document& doc, app::DependencyGraph& graph);

    /// 设置场景更新回调（可选，不设置时仅推导几何不更新场景）
    void setSceneUpdateCallback(SceneUpdateCallback cb);

    /// 启用异步模式（T71）
    void enableAsyncMode(AsyncFn asyncFn, DrainFn drainFn);

    /// 同步重算脏对象（向后兼容路径）
    void recompute(const std::vector<foundation::UUID>& dirtyIds);

    /// 异步重算（T71 新路径）
    void asyncRecompute();

    /// 主线程消费挂起的后台结果
    std::size_t drainResults();

    /// 重算文档中所有 PipePoint（全量刷新，同步降级模式）
    void recomputeAll();

private:
    app::Document&        doc_;
    app::DependencyGraph& graph_;
    SceneUpdateCallback   sceneCb_;

    AsyncFn  asyncFn_;
    DrainFn  drainFn_;

    /// 在所有 Segment 中查找 PipePoint 的前后邻居
    struct Neighbors {
        gp_Pnt prev;
        gp_Pnt next;
        bool   valid = false;
    };

    Neighbors findNeighbors(const model::PipePoint* pp) const;

    /// 对单个 PipePoint 生成几何并（可选地）通知场景
    void recomputePoint(const std::shared_ptr<model::PipePoint>& pp);
};

} // namespace engine

