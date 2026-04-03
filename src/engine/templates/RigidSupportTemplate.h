// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "engine/ComponentTemplate.h"
#include "geometry/BooleanOps.h"

#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>

namespace engine {

/// 刚性支撑模板 — 底板 + 立柱
/// 底板: 正方形截面 (edge = OD×1.2), 厚度 = OD×0.15
/// 立柱: 圆柱 (直径 = OD×0.4, 高 = OD×2.0)
class RigidSupportTemplate : public ComponentTemplate {
public:
    std::string templateId() const override { return "RigidSupport"; }

    ComponentParams deriveParams(double od, double wt) const override {
        ComponentParams p;
        p.od = od;
        p.wallThickness = wt;
        p.set("basePlateEdge", od * 1.2);
        p.set("basePlateThick", od * 0.15);
        p.set("columnDia", od * 0.4);
        p.set("columnHeight", od * 2.0);
        p.bodyLength = od * 1.2;
        p.bodyWidth = od * 1.2;
        p.bodyHeight = od * 2.0 + od * 0.15;
        return p;
    }

    TopoDS_Shape buildShape(const ComponentParams& p) const override {
        double edge = p.get("basePlateEdge", p.od * 1.2);
        double thick = p.get("basePlateThick", p.od * 0.15);
        double colDia = p.get("columnDia", p.od * 0.4);
        double colR = colDia / 2.0;
        double colH = p.get("columnHeight", p.od * 2.0);

        if (edge < 1e-9 || thick < 1e-9 || colR < 1e-9 || colH < 1e-9)
            return {};

        // 底板: 正方形截面挤出
        double hw = edge / 2.0;
        gp_Pnt p1(-hw, -hw, 0);
        gp_Pnt p2( hw, -hw, 0);
        gp_Pnt p3( hw,  hw, 0);
        gp_Pnt p4(-hw,  hw, 0);

        BRepBuilderAPI_MakeWire wm;
        wm.Add(BRepBuilderAPI_MakeEdge(p1, p2).Edge());
        wm.Add(BRepBuilderAPI_MakeEdge(p2, p3).Edge());
        wm.Add(BRepBuilderAPI_MakeEdge(p3, p4).Edge());
        wm.Add(BRepBuilderAPI_MakeEdge(p4, p1).Edge());
        if (!wm.IsDone()) return {};

        BRepBuilderAPI_MakeFace faceMaker(wm.Wire(), Standard_True);
        if (!faceMaker.IsDone()) return {};

        BRepPrimAPI_MakePrism prism(faceMaker.Face(), gp_Vec(0, 0, thick));
        if (!prism.IsDone()) return {};
        auto basePlate = prism.Shape();

        // 立柱: 圆柱
        gp_Ax2 colAx(gp_Pnt(0, 0, thick), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeCylinder column(colAx, colR, colH);

        return geometry::BooleanOps::fuse(basePlate, column.Shape());
    }
};

} // namespace engine
