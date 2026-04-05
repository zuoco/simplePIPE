// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <vector>
#include <memory>

namespace model { class PipePoint; class PipeSpec; }

namespace engine {

/// 统一几何推导入口
/// 根据管点类型分发到对应的 Builder 生成 BRep 几何
class GeometryDeriver {
public:
    /// 为给定管点生成几何体
    /// @param prevPoint 前一个管点位置 (直管段起点 / 弯头的 A05)
    /// @param current   当前管点
    /// @param nextPoint 后一个管点位置 (直管段终点 / 弯头的 A07)；
    ///                  对于 Run 类型，此参数用作终点
    /// @return TopoDS_Shape；无法生成时返回空 Shape
    static TopoDS_Shape deriveGeometry(
        const gp_Pnt& prevPoint,
        const std::shared_ptr<model::PipePoint>& current,
        const gp_Pnt& nextPoint);
};

} // namespace engine
