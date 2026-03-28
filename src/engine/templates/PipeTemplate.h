#pragma once

#include "engine/ComponentTemplate.h"
#include "geometry/BooleanOps.h"

#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>

namespace engine {

/// 直管模板 — 圆柱壳 (外柱 cut 内柱)
/// 沿 Z 轴生成，长度 = bodyLength
class PipeTemplate : public ComponentTemplate {
public:
    std::string templateId() const override { return "Pipe"; }

    ComponentParams deriveParams(double od, double wt) const override {
        ComponentParams p;
        p.od = od;
        p.wallThickness = wt;
        p.bodyLength = od * 3.0;  // 默认长度 = 3×OD (可被外部覆盖)
        p.bodyWidth = od;
        p.bodyHeight = od;
        return p;
    }

    TopoDS_Shape buildShape(const ComponentParams& p) const override {
        double outerR = p.od / 2.0;
        double innerR = outerR - p.wallThickness;
        double length = p.bodyLength;
        if (outerR < 1e-9 || innerR < 1e-9 || length < 1e-9) return {};

        gp_Ax2 ax(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeCylinder outerCyl(ax, outerR, length);
        BRepPrimAPI_MakeCylinder innerCyl(ax, innerR, length);
        return geometry::BooleanOps::cut(outerCyl.Shape(), innerCyl.Shape());
    }
};

} // namespace engine
