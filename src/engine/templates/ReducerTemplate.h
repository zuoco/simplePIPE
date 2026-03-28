#pragma once

#include "engine/ComponentTemplate.h"
#include "geometry/BooleanOps.h"

#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>

namespace engine {

/// 异径管模板 — 锥壳 (大端→小端)
/// 大端 OD = od, 小端OD = od * 0.75 (默认)
class ReducerTemplate : public ComponentTemplate {
public:
    std::string templateId() const override { return "Reducer"; }

    ComponentParams deriveParams(double od, double wt) const override {
        ComponentParams p;
        p.od = od;
        p.wallThickness = wt;
        p.bodyLength = od * 1.5;      // 异径管长度
        p.bodyWidth = od;
        p.bodyHeight = od;
        p.set("endOD", od * 0.75);     // 小端外径
        return p;
    }

    TopoDS_Shape buildShape(const ComponentParams& p) const override {
        double outerR1 = p.od / 2.0;          // 大端外半径
        double endOD = p.get("endOD", p.od * 0.75);
        double outerR2 = endOD / 2.0;          // 小端外半径
        double innerR1 = outerR1 - p.wallThickness;
        double innerR2 = outerR2 - p.wallThickness;
        double length = p.bodyLength;

        if (outerR1 < 1e-9 || outerR2 < 1e-9 || length < 1e-9) return {};
        if (innerR1 < 1e-9 || innerR2 < 1e-9) return {};

        gp_Ax2 ax(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeCone outerCone(ax, outerR1, outerR2, length);
        BRepPrimAPI_MakeCone innerCone(ax, innerR1, innerR2, length);
        return geometry::BooleanOps::cut(outerCone.Shape(), innerCone.Shape());
    }
};

} // namespace engine
