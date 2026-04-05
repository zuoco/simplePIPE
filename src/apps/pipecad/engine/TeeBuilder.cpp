// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/TeeBuilder.h"
#include "geometry/BooleanOps.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>

namespace engine {

// Helper: build a solid cylinder between two points
static TopoDS_Shape makeSolidCylinder(const gp_Pnt& p1, const gp_Pnt& p2, double radius) {
    gp_Vec dir(p1, p2);
    double length = dir.Magnitude();
    if (length < 1e-9 || radius < 1e-9) return {};
    dir.Normalize();
    gp_Ax2 ax(p1, gp_Dir(dir));
    BRepPrimAPI_MakeCylinder maker(ax, radius, length);
    return maker.Shape();
}

TopoDS_Shape TeeBuilder::build(
    const gp_Pnt& mainStart,
    const gp_Pnt& mainEnd,
    const gp_Pnt& branchPoint,
    const gp_Pnt& branchEnd,
    double mainOD,
    double branchOD,
    double wallThickness)
{
    double mainOuterR   = mainOD / 2.0;
    double branchOuterR = branchOD / 2.0;
    double mainInnerR   = mainOuterR - wallThickness;
    double branchInnerR = branchOuterR - wallThickness;
    if (mainInnerR < 1e-9 || branchInnerR < 1e-9) return {};

    // Step 1: fuse outer solids (fast on solids)
    auto mainOuter   = makeSolidCylinder(mainStart, mainEnd, mainOuterR);
    auto branchOuter = makeSolidCylinder(branchPoint, branchEnd, branchOuterR);
    if (mainOuter.IsNull() || branchOuter.IsNull()) return {};
    auto outerFused = geometry::BooleanOps::fuse(mainOuter, branchOuter);
    if (outerFused.IsNull()) return {};

    // Step 2: fuse inner solids
    auto mainInner   = makeSolidCylinder(mainStart, mainEnd, mainInnerR);
    auto branchInner = makeSolidCylinder(branchPoint, branchEnd, branchInnerR);
    if (mainInner.IsNull() || branchInner.IsNull()) return {};
    auto innerFused = geometry::BooleanOps::fuse(mainInner, branchInner);
    if (innerFused.IsNull()) return {};

    // Step 3: hollow out: outer - inner
    return geometry::BooleanOps::cut(outerFused, innerFused);
}

} // namespace engine
