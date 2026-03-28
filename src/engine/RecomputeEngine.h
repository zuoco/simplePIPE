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
class RecomputeEngine {
public:
    /// 场景更新回调类型：(对象UUID字符串, 新几何体)
    using SceneUpdateCallback = std::function<void(const std::string&, const TopoDS_Shape&)>;

    RecomputeEngine(app::Document& doc, app::DependencyGraph& graph);

    /// 设置场景更新回调（可选，不设置时仅推导几何不更新场景）
    void setSceneUpdateCallback(SceneUpdateCallback cb);

    /// 重算脏对象（由 TransactionManager 的 recomputeCallback 调用）
    void recompute(const std::vector<foundation::UUID>& dirtyIds);

    /// 重算文档中所有 PipePoint（全量刷新）
    void recomputeAll();

private:
    app::Document&        doc_;
    app::DependencyGraph& graph_;
    SceneUpdateCallback   sceneCb_;

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

