#pragma once

#include "engine/ComponentTemplate.h"
#include "geometry/BooleanOps.h"

#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>

namespace engine {

/// 三通模板 — 主管 + 支管 fuse
/// 支管垂直于主管中点向 Y 方向伸出
class TeeTemplate : public ComponentTemplate {
public:
    std::string templateId() const override { return "Tee"; }

    ComponentParams deriveParams(double od, double wt) const override {
        ComponentParams p;
        p.od = od;
        p.wallThickness = wt;
        p.bodyLength = od * 3.0;       // 主管总长
        p.bodyWidth = od;
        p.bodyHeight = od;
        p.set("branchOD", od);          // 支管外径默认等于主管
        p.set("branchLength", od * 1.5); // 支管长度
        return p;
    }

    TopoDS_Shape buildShape(const ComponentParams& p) const override {
        double outerR = p.od / 2.0;
        double innerR = outerR - p.wallThickness;
        double mainLen = p.bodyLength;
        // slightly reduce branch OD to avoid exact tangent singularity in OpenCASCADE boolean fuse
        double branchOD = p.get("branchOD", p.od) * 0.999;
        double branchOuterR = branchOD / 2.0;
        double branchInnerR = branchOuterR - p.wallThickness;
        double branchLen = p.get("branchLength", p.od * 1.5) + outerR; // extend slightly into the main pipe

        if (outerR < 1e-9 || innerR < 1e-9 || mainLen < 1e-9) return {};
        if (branchOuterR < 1e-9 || branchInnerR < 1e-9 || branchLen < 1e-9) return {};

        // 主管沿 Z 轴
        gp_Ax2 mainAx(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeCylinder mainOuter(mainAx, outerR, mainLen);
        BRepPrimAPI_MakeCylinder mainInner(mainAx, innerR, mainLen);

        // 支管从主管轴心下方一点点开始，穿过一半主管，避免 face 在 Z=0 重合导致的非流形
        gp_Pnt branchOrigin(0, -outerR * 0.1, mainLen / 2.0);
        gp_Ax2 branchAx(branchOrigin, gp_Dir(0, 1, 0));
        BRepPrimAPI_MakeCylinder branchOuter(branchAx, branchOuterR, branchLen);
        BRepPrimAPI_MakeCylinder branchInner(branchAx, branchInnerR, branchLen);

        // 外壳: main fuse branch
        auto outerShell = geometry::BooleanOps::fuse(mainOuter.Shape(), branchOuter.Shape());
        auto innerShell = geometry::BooleanOps::fuse(mainInner.Shape(), branchInner.Shape());
        return geometry::BooleanOps::cut(outerShell, innerShell);
    }
};

} // namespace engine
