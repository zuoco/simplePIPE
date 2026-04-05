// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/ReducerBuilder.h"
#include "geometry/BooleanOps.h"

#include <BRepPrimAPI_MakeCone.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>

namespace engine {

TopoDS_Shape ReducerBuilder::build(
    const gp_Pnt& startPoint,
    const gp_Pnt& endPoint,
    double startOD,
    double endOD,
    double wallThickness)
{
    gp_Vec dir(startPoint, endPoint);
    double length = dir.Magnitude();
    if (length < 1e-9) return {};

    double r1outer = startOD / 2.0;
    double r2outer = endOD / 2.0;
    double r1inner = r1outer - wallThickness;
    double r2inner = r2outer - wallThickness;

    if (r1outer < 1e-9 || r2outer < 1e-9 || r1inner < 1e-9 || r2inner < 1e-9)
        return {};

    dir.Normalize();
    gp_Ax2 ax(startPoint, gp_Dir(dir));

    BRepPrimAPI_MakeCone outerCone(ax, r1outer, r2outer, length);
    BRepPrimAPI_MakeCone innerCone(ax, r1inner, r2inner, length);

    return geometry::BooleanOps::cut(outerCone.Shape(), innerCone.Shape());
}

} // namespace engine
