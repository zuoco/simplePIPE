#pragma once

#include "engine/ComponentTemplate.h"
#include "geometry/BooleanOps.h"

#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>

namespace engine {

/// 弹簧支吊架模板 — 弹簧体(圆柱) + 吊杆(细圆柱)
/// 弹簧体: 直径 = OD×0.8, 高 = OD×1.2
/// 吊杆:   直径 = OD×0.12, 高 = OD×2.5
class SpringHangerTemplate : public ComponentTemplate {
public:
    std::string templateId() const override { return "SpringHanger"; }

    ComponentParams deriveParams(double od, double wt) const override {
        ComponentParams p;
        p.od = od;
        p.wallThickness = wt;
        p.set("springDia", od * 0.8);
        p.set("springHeight", od * 1.2);
        p.set("rodDia", od * 0.12);
        p.set("rodHeight", od * 2.5);
        p.bodyLength = od * 0.8;
        p.bodyWidth = od * 0.8;
        p.bodyHeight = od * 1.2 + od * 2.5;
        return p;
    }

    TopoDS_Shape buildShape(const ComponentParams& p) const override {
        double springR = p.get("springDia", p.od * 0.8) / 2.0;
        double springH = p.get("springHeight", p.od * 1.2);
        double rodR = p.get("rodDia", p.od * 0.12) / 2.0;
        double rodH = p.get("rodHeight", p.od * 2.5);

        if (springR < 1e-9 || springH < 1e-9 || rodR < 1e-9 || rodH < 1e-9)
            return {};

        // 弹簧体 (底部)
        gp_Ax2 springAx(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeCylinder spring(springAx, springR, springH);

        // 吊杆 (弹簧体上方)
        gp_Ax2 rodAx(gp_Pnt(0, 0, springH), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeCylinder rod(rodAx, rodR, rodH);

        return geometry::BooleanOps::fuse(spring.Shape(), rod.Shape());
    }
};

} // namespace engine
