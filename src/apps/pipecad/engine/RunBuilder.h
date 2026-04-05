// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

namespace engine {

/// 直管段几何生成器
/// 生成圆柱壳(外柱 cut 内柱)，从 startPoint 到 endPoint
class RunBuilder {
public:
    /// @param startPoint 管段起点 (中心线)
    /// @param endPoint   管段终点 (中心线)
    /// @param outerDiameter 外径 (mm)
    /// @param wallThickness 壁厚 (mm)
    /// @return 管壳 TopoDS_Shape；输入无效时返回空 Shape
    static TopoDS_Shape build(
        const gp_Pnt& startPoint,
        const gp_Pnt& endPoint,
        double outerDiameter,
        double wallThickness);
};

} // namespace engine
