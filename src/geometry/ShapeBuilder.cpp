#include "geometry/ShapeBuilder.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <GC_MakeCircle.hxx>
#include <Geom_Circle.hxx>
#include <gp_Circ.hxx>
#include <gp_Ax2.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace geometry {

TopoDS_Shape ShapeBuilder::makeCylinder(double radius, double height) {
    BRepPrimAPI_MakeCylinder maker(radius, height);
    return maker.Shape();
}

TopoDS_Shape ShapeBuilder::makeTorus(double majorR, double minorR, double angle) {
    BRepPrimAPI_MakeTorus maker(majorR, minorR, angle);
    return maker.Shape();
}

TopoDS_Shape ShapeBuilder::makeCone(double r1, double r2, double height) {
    BRepPrimAPI_MakeCone maker(r1, r2, height);
    return maker.Shape();
}

TopoDS_Shape ShapeBuilder::makePipeShell(const TopoDS_Wire& spine, double radius) {
    // 构造圆截面路径
    BRepOffsetAPI_MakePipeShell pipeBuilder(spine);

    // 在路径起点处构造圆形截面 Wire
    gp_Ax2 circAx(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    Handle(Geom_Circle) circle = new Geom_Circle(circAx, radius);
    TopoDS_Edge circEdge = BRepBuilderAPI_MakeEdge(circle);
    TopoDS_Wire circWire = BRepBuilderAPI_MakeWire(circEdge);

    pipeBuilder.Add(circWire);
    pipeBuilder.Build();

    if (!pipeBuilder.IsDone()) {
        return TopoDS_Shape();
    }
    return pipeBuilder.Shape();
}

} // namespace geometry
