// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <vector>
#include <memory>

namespace model { class PipePoint; class PipeSpec; }

// 快照类型前向声明（仅用于 deriveFromSnapshot 接口）
namespace app {
    struct PipePointSnapshot;
    struct PipeSpecSnapshot;
}

namespace engine {

/// 统一几何推导入口
/// 根据管点类型分发到对应的 Builder 生成 BRep 几何
class GeometryDeriver {
public:
    /// 为给定管点生成几何体（同步路径，直接访问文档对象）
    /// @param prevPoint 前一个管点位置
    /// @param current   当前管点
    /// @param nextPoint 后一个管点位置
    /// @return TopoDS_Shape；无法生成时返回空 Shape
    static TopoDS_Shape deriveGeometry(
        const gp_Pnt& prevPoint,
        const std::shared_ptr<model::PipePoint>& current,
        const gp_Pnt& nextPoint);

    /// 为给定管点快照生成几何体（异步路径，仅访问只读快照）
    ///
    /// **[后台线程安全]** 此方法只访问值类型快照，不触及任何共享可变状态。
    ///
    /// @param prevPoint 前一个管点位置（从快照中提取）
    /// @param current   当前管点快照（PipePointSnapshot 值对象）
    /// @param spec      对应 PipeSpec 快照（可为 nullptr）
    /// @param nextPoint 后一个管点位置（从快照中提取）
    /// @return TopoDS_Shape；无法生成时返回空 Shape
    static TopoDS_Shape deriveFromSnapshot(
        const gp_Pnt& prevPoint,
        const app::PipePointSnapshot& current,
        const app::PipeSpecSnapshot* spec,
        const gp_Pnt& nextPoint);
};

} // namespace engine
