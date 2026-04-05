// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/BendBuilder.h"
#include "geometry/BooleanOps.h"

#include <BRepPrimAPI_MakeTorus.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <cmath>

namespace engine {

TopoDS_Shape BendBuilder::build(
    const BendResult& bendResult,
    double outerDiameter,
    double wallThickness)
{
    double outerR = outerDiameter / 2.0;
    double innerR = outerR - wallThickness;
    if (innerR < 1e-9 || outerR < 1e-9) return {};

    double majorR = bendResult.bendRadius;
    double angle  = bendResult.bendAngle;
    if (majorR < 1e-9 || angle < 1e-9) return {};

    // The torus arc center is bendResult.arcCenter
    // The torus axis is perpendicular to the bend plane
    // Bend plane normal = cross(C→N, C→F)
    gp_Vec cN(bendResult.arcCenter, bendResult.nearPoint);
    gp_Vec cF(bendResult.arcCenter, bendResult.farPoint);
    gp_Vec axisVec = cN.Crossed(cF);

    if (axisVec.Magnitude() < 1e-12) return {};
    axisVec.Normalize();

    // The torus is built with its starting direction along cN
    // OCCT MakeTorus builds in XY plane starting from X-axis direction
    // We need to set the coordinate system so that:
    //   - Origin = arcCenter
    //   - Z-axis = bend plane normal (axisVec)
    //   - X-axis = direction from center to nearPoint (cN normalized)
    gp_Dir zDir(axisVec);
    gp_Dir xDir(cN);
    gp_Ax2 ax(bendResult.arcCenter, zDir, xDir);

    BRepPrimAPI_MakeTorus outerTorus(ax, majorR, outerR, angle);
    BRepPrimAPI_MakeTorus innerTorus(ax, majorR, innerR, angle);

    return geometry::BooleanOps::cut(outerTorus.Shape(), innerTorus.Shape());
}

} // namespace engine
