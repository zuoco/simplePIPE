#pragma once

#include "engine/ComponentTemplate.h"
#include "geometry/BooleanOps.h"

#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>

namespace engine {

/// 对焊法兰模板 — 圆盘 + 短颈管段
/// 法兰外径 = OD × 1.5, 螺栓孔径 = OD × 1.35, 法兰厚 = OD × 0.3
class WeldNeckFlangeTemplate : public ComponentTemplate {
public:
    std::string templateId() const override { return "WeldNeckFlange"; }

    ComponentParams deriveParams(double od, double wt) const override {
        ComponentParams p;
        p.od = od;
        p.wallThickness = wt;
        p.bodyLength = od * 0.3;        // 法兰盘厚度
        p.bodyWidth = od * 1.5;         // 法兰外径
        p.bodyHeight = od * 1.5;
        p.set("neckLength", od * 0.5);   // 短颈长度
        p.set("boltCircleDia", od * 1.35); // 螺栓孔圆径
        return p;
    }

    TopoDS_Shape buildShape(const ComponentParams& p) const override {
        double flangeR = p.bodyWidth / 2.0;
        double flangeThick = p.bodyLength;
        double outerR = p.od / 2.0;
        double innerR = outerR - p.wallThickness;
        double neckLen = p.get("neckLength", p.od * 0.5);

        if (flangeR < 1e-9 || flangeThick < 1e-9 || outerR < 1e-9 || innerR < 1e-9)
            return {};

        gp_Ax2 zAx(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));

        // 法兰盘
        BRepPrimAPI_MakeCylinder flangeDisk(zAx, flangeR, flangeThick);

        // 短颈管段
        gp_Ax2 neckAx(gp_Pnt(0, 0, flangeThick), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeCylinder neck(neckAx, outerR, neckLen);

        auto outer = geometry::BooleanOps::fuse(flangeDisk.Shape(), neck.Shape());

        // 内腔贯穿
        double totalLen = flangeThick + neckLen;
        BRepPrimAPI_MakeCylinder inner(zAx, innerR, totalLen);
        return geometry::BooleanOps::cut(outer, inner.Shape());
    }
};

} // namespace engine
