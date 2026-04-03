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

namespace engine {

/// 闸阀模板 — 管段 + 膨大阀体 + 管段 + 阀杆 + 手轮（外形 cut 内腔）
/// 参数化规则: 阀体宽 ≈ 1.8×OD, 阀体长 ≈ 2.0×OD, 含手轮高 ≈ 3.0×OD
class GateValveTemplate : public ComponentTemplate {
public:
    std::string templateId() const override { return "GateValve"; }

    ComponentParams deriveParams(double od, double wt) const override {
        ComponentParams p;
        p.od = od;
        p.wallThickness = wt;
        p.bodyLength = od * 2.0;        // 总阀体轴向长
        p.bodyWidth = od * 1.8;         // 阀体径向宽
        p.bodyHeight = od * 3.0;        // 含手轮总高
        p.set("stemDia", od * 0.15);     // 阀杆直径
        p.set("handwheelDia", od * 1.8); // 手轮直径
        p.set("handwheelThick", od * 0.08); // 手轮厚度
        return p;
    }

    TopoDS_Shape buildShape(const ComponentParams& p) const override {
        double outerR = p.od / 2.0;
        double innerR = outerR - p.wallThickness;
        double bodyLen = p.bodyLength;
        double bodyR = p.bodyWidth / 2.0;
        double stemDia = p.get("stemDia");
        double stemR = stemDia / 2.0;
        double hwDia = p.get("handwheelDia");
        double hwR = hwDia / 2.0;
        double hwThick = p.get("handwheelThick", p.od * 0.08);

        if (outerR < 1e-9 || innerR < 1e-9 || bodyLen < 1e-9) return {};

        // 管段stub长度 = OD (每侧)
        double stubLen = p.od;
        double totalLen = stubLen + bodyLen + stubLen;

        gp_Ax2 zAx(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));

        // 上游管段
        BRepPrimAPI_MakeCylinder stub1(zAx, outerR, stubLen);

        // 阀体 (膨大段)
        gp_Ax2 bodyAx(gp_Pnt(0, 0, stubLen), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeCylinder body(bodyAx, bodyR, bodyLen);

        // 下游管段
        gp_Ax2 stub2Ax(gp_Pnt(0, 0, stubLen + bodyLen), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeCylinder stub2(stub2Ax, outerR, stubLen);

        auto outer = geometry::BooleanOps::fuse(
            stub1.Shape(),
            geometry::BooleanOps::fuse(body.Shape(), stub2.Shape()));

        // 阀杆 (从阀体中心向 Y+ 伸出)
        double stemHeight = p.bodyHeight - bodyR;
        if (stemR > 1e-9 && stemHeight > 1e-9) {
            gp_Pnt stemBase(0, 0, stubLen + bodyLen / 2.0);
            gp_Ax2 stemAx(stemBase, gp_Dir(0, 1, 0));
            BRepPrimAPI_MakeCylinder stem(stemAx, stemR, stemHeight);
            outer = geometry::BooleanOps::fuse(outer, stem.Shape());

            // 手轮 (阀杆顶端的圆盘)
            if (hwR > 1e-9 && hwThick > 1e-9) {
                gp_Pnt hwCenter = stemBase.Translated(gp_Vec(0, stemHeight, 0));
                gp_Ax2 hwAx(hwCenter, gp_Dir(0, 1, 0));
                BRepPrimAPI_MakeCylinder hw(hwAx, hwR, hwThick);
                outer = geometry::BooleanOps::fuse(outer, hw.Shape());
            }
        }

        // 内腔 — 从起点到终点的内径柱
        BRepPrimAPI_MakeCylinder inner(zAx, innerR, totalLen);
        return geometry::BooleanOps::cut(outer, inner.Shape());
    }
};

} // namespace engine
