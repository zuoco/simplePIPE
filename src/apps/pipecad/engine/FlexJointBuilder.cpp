// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/FlexJointBuilder.h"
#include "geometry/BooleanOps.h"

#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>

namespace engine {

TopoDS_Shape FlexJointBuilder::build(
    const gp_Pnt& startPoint,
    const gp_Pnt& endPoint,
    double outerDiameter,
    double wallThickness,
    int segmentCount)
{
    gp_Vec dir(startPoint, endPoint);
    double totalLen = dir.Magnitude();
    if (totalLen < 1e-9) return {};
    if (segmentCount < 1) segmentCount = 1;

    double outerR   = outerDiameter / 2.0;
    double innerR   = outerR - wallThickness;
    if (innerR < 1e-9 || outerR < 1e-9) return {};

    dir.Normalize();
    gp_Dir gdir(dir);

    // 波纹管膨胀半径 = 外径的 1.5 倍
    double expandedR = outerR * 1.5;

    // 每个波纹由 2 段锥体组成（扩 + 缩），共 2*segmentCount 段
    int coneCount = 2 * segmentCount;
    double coneLen = totalLen / coneCount;

    // 逐段构建锥体并 fuse
    auto buildCone = [&](int i) -> TopoDS_Shape {
        gp_Pnt pos = startPoint.Translated(gp_Vec(gdir) * (i * coneLen));
        gp_Ax2 ax(pos, gdir);
        // 偶数段：outerR → expandedR；奇数段：expandedR → outerR
        double r1 = (i % 2 == 0) ? outerR : expandedR;
        double r2 = (i % 2 == 0) ? expandedR : outerR;
        return BRepPrimAPI_MakeCone(ax, r1, r2, coneLen).Shape();
    };

    TopoDS_Shape outer = buildCone(0);
    for (int i = 1; i < coneCount; ++i) {
        outer = geometry::BooleanOps::fuse(outer, buildCone(i));
    }

    // 内腔：从 startPoint 到 endPoint 的统一内径柱
    BRepPrimAPI_MakeCylinder inner(gp_Ax2(startPoint, gdir), innerR, totalLen);
    return geometry::BooleanOps::cut(outer, inner.Shape());
}

} // namespace engine
