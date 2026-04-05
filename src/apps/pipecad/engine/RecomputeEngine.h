// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

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
/// 原因：
/// - OCCT 几何运算（BRepAlgo、BRep_Builder 等）非线程安全，
///   不同线程不得共享 OCCT 上下文句柄。
/// - doc_ 与 graph_ 引用的对象由主线程独占写入，RecomputeEngine
///   在主线程读取这些引用时无需额外同步。
/// - SceneUpdateCallback 通常持有 SceneManager 的引用，
///   SceneManager 的 addNode/updateNode/removeNode 也仅主线程调用。
///
/// ## 异步模式（T71）
///
/// 通过 enableAsyncMode() 注入以下三个类型擦除函数后，
/// asyncRecompute() 将遵循 T70 同步策略中的快照窗口协议执行：
///
///   [主线程] collectDirty → makeDocumentSnapshot → clearDirty → submitTask
///   [后台线程] 基于 DocumentSnapshot 推导几何 → postResult
///   [主线程事件循环] drainResults() → applyFn → sceneCb_
///
/// 类型擦除确保 RecomputeEngine.h 不引入对 lib_runtime 的直接 CMake 依赖。
class RecomputeEngine {
public:
    /// 场景更新回调类型：(对象UUID字符串, 新几何体)
    using SceneUpdateCallback = std::function<void(const std::string&, const TopoDS_Shape&)>;

    /// 异步执行函数类型：封装完整的快照构建→后台提交→结果回投管线。
    ///
    /// 由 main.cpp 注入，在 asyncRecompute() 中调用。
    /// 注入者负责在主线程内依次执行：
    ///   1. makeDocumentSnapshot(doc, graph)   → 构建只读快照（捕获脏 ID）
    ///   2. graph.clearDirty()                 → 清除脏标记
    ///   3. workers.submit(task)               → 提交后台几何推导
    ///   4. [后台] channel.post(version, fn)   → 结果回投
    ///
    /// 类型擦除确保 RecomputeEngine.h 不引入对 lib_runtime 的依赖。
    using AsyncFn = std::function<void()>;

    /// 结果消费函数类型：委托 SceneUpdateAdapter::drain()（主线程事件循环调用）
    using DrainFn = std::function<std::size_t()>;

    RecomputeEngine(app::Document& doc, app::DependencyGraph& graph);

    /// 设置场景更新回调（可选，不设置时仅推导几何不更新场景）
    void setSceneUpdateCallback(SceneUpdateCallback cb);

    /// 启用异步模式（T71）
    ///
    /// @param asyncFn  封装完整异步管线的执行函数（快照→提交→回投）
    /// @param drainFn  主线程消费后台结果的函数（版本校验后执行 applyFn）
    void enableAsyncMode(AsyncFn asyncFn, DrainFn drainFn);

    /// 同步重算脏对象（由 CommandStack 执行命令后调用；向后兼容路径）
    void recompute(const std::vector<foundation::UUID>& dirtyIds);

    /// 异步重算（T71 新路径，requires enableAsyncMode() 已调用）
    ///
    /// 若有脏对象且异步模式已启用，调用注入的 asyncFn_() 执行异步管线。
    /// 若异步模式未启用，退化为同步 recompute()。
    void asyncRecompute();

    /// 主线程消费挂起的后台结果（需在事件循环中定期调用）
    ///
    /// 若异步模式未启用则为 no-op。
    ///
    /// @return 本次实际执行的 applyFn 数量
    std::size_t drainResults();

    /// 重算文档中所有 PipePoint（全量刷新，同步降级模式）
    void recomputeAll();

    /// 后台化全量重算（加载恢复阶段使用）
    ///
    /// 将文档中所有 PipePoint 标记为脏，然后调用 asyncRecompute()。
    /// 若异步模式未启用，退化为同步 recomputeAll()。
    ///
    /// 典型调用时机：
    ///   - 工程加载后（ProjectSerializer::load() 返回新 Document 后）
    ///   - 工作台切换触发全量几何刷新时
    void asyncRecomputeAll();

private:
    app::Document&        doc_;
    app::DependencyGraph& graph_;
    SceneUpdateCallback   sceneCb_;

    // 异步模式注入函数（未启用时为 null）
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

