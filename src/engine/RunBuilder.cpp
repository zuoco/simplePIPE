#include "engine/RunBuilder.h"
#include "geometry/ShapeBuilder.h"
#include "geometry/BooleanOps.h"
#include "geometry/ShapeTransform.h"

#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Trsf.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <cmath>

namespace engine {

TopoDS_Shape RunBuilder::build(
    const gp_Pnt& startPoint,
    const gp_Pnt& endPoint,
    double outerDiameter,
    double wallThickness)
{
    gp_Vec dir(startPoint, endPoint);
    double length = dir.Magnitude();
    if (length < 1e-9) return {};

    double outerR = outerDiameter / 2.0;
    double innerR = outerR - wallThickness;
    if (innerR < 1e-9 || outerR < 1e-9) return {};

    dir.Normalize();

    // Build cylinders along the direction from startPoint
    gp_Ax2 ax(startPoint, gp_Dir(dir));
    BRepPrimAPI_MakeCylinder outerCyl(ax, outerR, length);
    BRepPrimAPI_MakeCylinder innerCyl(ax, innerR, length);

    return geometry::BooleanOps::cut(outerCyl.Shape(), innerCyl.Shape());
}

} // namespace engine
