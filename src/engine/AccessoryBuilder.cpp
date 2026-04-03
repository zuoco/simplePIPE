// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/AccessoryBuilder.h"

#include <gp_Vec.hxx>
#include <gp_Ax2.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>

namespace engine {

TopoDS_Shape AccessoryBuilder::buildFlange(
    const gp_Pnt& center,
    const gp_Dir& normal,
    double pipeDiameter,
    double thickness)
{
    if (pipeDiameter < 1e-9 || thickness < 1e-9) return {};

    double flangeR = pipeDiameter * 0.75; // 法兰半径 = 管道外径 * 0.75

    // 令圆柱从 center - normal*(thickness/2) 出发，沿 normal 方向生长
    gp_Pnt diskStart = center.Translated(gp_Vec(normal) * (-thickness / 2.0));
    gp_Ax2 ax(diskStart, normal);
    return BRepPrimAPI_MakeCylinder(ax, flangeR, thickness).Shape();
}

TopoDS_Shape AccessoryBuilder::buildBracket(
    const gp_Pnt& base,
    const gp_Pnt& top,
    double width)
{
    gp_Vec extrudeVec(base, top);
    double length = extrudeVec.Magnitude();
    if (length < 1e-9 || width < 1e-9) return {};

    gp_Dir gdir(extrudeVec);

    // 在 base 处建立垂直于轴的坐标系
    gp_Ax2 localAx(base, gdir);
    gp_Dir xd = localAx.XDirection();
    gp_Dir yd = localAx.YDirection();

    // 正方形截面（边长 = width，以 base 为中心）
    double hw = width / 2.0;
    gp_Vec vx(xd), vy(yd);
    gp_Pnt p1 = base.Translated(vx * (-hw) + vy * (-hw));
    gp_Pnt p2 = base.Translated(vx * ( hw) + vy * (-hw));
    gp_Pnt p3 = base.Translated(vx * ( hw) + vy * ( hw));
    gp_Pnt p4 = base.Translated(vx * (-hw) + vy * ( hw));

    BRepBuilderAPI_MakeWire wm;
    wm.Add(BRepBuilderAPI_MakeEdge(p1, p2).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p2, p3).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p3, p4).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p4, p1).Edge());
    TopoDS_Wire profileWire = wm.Wire();
    if (profileWire.IsNull()) return {};

    BRepBuilderAPI_MakeFace faceMaker(profileWire, Standard_True);
    if (!faceMaker.IsDone()) return {};

    BRepPrimAPI_MakePrism prism(faceMaker.Face(), extrudeVec);
    if (!prism.IsDone()) return {};
    return prism.Shape();
}

} // namespace engine
