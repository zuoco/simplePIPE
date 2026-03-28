#include "engine/ValveBuilder.h"
#include "geometry/BooleanOps.h"

#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>

namespace engine {

TopoDS_Shape ValveBuilder::build(
    const gp_Pnt& startPoint,
    const gp_Pnt& endPoint,
    double outerDiameter,
    double wallThickness,
    const std::string& valveType)
{
    gp_Vec dir(startPoint, endPoint);
    double totalLen = dir.Magnitude();
    if (totalLen < 1e-9) return {};

    double outerR = outerDiameter / 2.0;
    double innerR = outerR - wallThickness;
    if (innerR < 1e-9 || outerR < 1e-9) return {};

    dir.Normalize();
    gp_Dir gdir(dir);

    // 阀体尺寸：根据阀门类型选择不同扩径比例
    double bodyR, bodyLen;
    if (valveType == "ball") {
        bodyR = outerR * 1.6;
        bodyLen = outerDiameter * 1.5;
    } else if (valveType == "check") {
        bodyR = outerR * 1.6;
        bodyLen = outerDiameter * 1.5;
    } else {
        // gate (default)
        bodyR = outerR * 1.8;
        bodyLen = outerDiameter * 2.0;
    }

    // 阀体长度不能超过总长的 70%
    if (bodyLen > totalLen * 0.7) {
        bodyLen = totalLen * 0.7;
    }
    double stubLen = (totalLen - bodyLen) / 2.0;

    TopoDS_Shape outer;
    if (stubLen < 1e-6) {
        // 无管段短截，直接用阀体柱替代整段
        outer = BRepPrimAPI_MakeCylinder(gp_Ax2(startPoint, gdir), bodyR, totalLen).Shape();
    } else {
        gp_Pnt bodyStart = startPoint.Translated(gp_Vec(gdir) * stubLen);
        gp_Pnt bodyEnd   = bodyStart.Translated(gp_Vec(gdir) * bodyLen);

        BRepPrimAPI_MakeCylinder stub1(gp_Ax2(startPoint, gdir), outerR, stubLen);
        BRepPrimAPI_MakeCylinder body (gp_Ax2(bodyStart,   gdir), bodyR,  bodyLen);
        BRepPrimAPI_MakeCylinder stub2(gp_Ax2(bodyEnd,     gdir), outerR, stubLen);

        outer = geometry::BooleanOps::fuse(
                    stub1.Shape(),
                    geometry::BooleanOps::fuse(body.Shape(), stub2.Shape()));
    }

    // 内腔：从 startPoint 到 endPoint 的单一内径柱
    BRepPrimAPI_MakeCylinder inner(gp_Ax2(startPoint, gdir), innerR, totalLen);
    return geometry::BooleanOps::cut(outer, inner.Shape());
}

} // namespace engine
