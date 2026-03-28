#pragma once

#include "engine/ComponentTemplate.h"
#include "geometry/BooleanOps.h"

#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <cmath>

namespace engine {

/// 弯头模板 — 圆环截段壳体
/// 默认 90° 弯，弯曲半径 = 1.5×OD (长半径弯头)
class ElbowTemplate : public ComponentTemplate {
public:
    std::string templateId() const override { return "Elbow"; }

    ComponentParams deriveParams(double od, double wt) const override {
        ComponentParams p;
        p.od = od;
        p.wallThickness = wt;
        double bendMultiplier = 1.5;
        p.set("bendRadius", od * bendMultiplier);
        p.set("bendAngle", M_PI / 2.0); // 默认 90°
        p.bodyLength = od * bendMultiplier; // 弯曲半径
        p.bodyWidth = od;
        p.bodyHeight = od;
        return p;
    }

    TopoDS_Shape buildShape(const ComponentParams& p) const override {
        double outerR = p.od / 2.0;
        double innerR = outerR - p.wallThickness;
        double majorR = p.get("bendRadius", p.od * 1.5);
        double angle = p.get("bendAngle", M_PI / 2.0);

        if (outerR < 1e-9 || innerR < 1e-9 || majorR < 1e-9 || angle < 1e-9)
            return {};

        gp_Ax2 ax(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeTorus outerTorus(ax, majorR, outerR, angle);
        BRepPrimAPI_MakeTorus innerTorus(ax, majorR, innerR, angle);
        return geometry::BooleanOps::cut(outerTorus.Shape(), innerTorus.Shape());
    }
};

} // namespace engine
