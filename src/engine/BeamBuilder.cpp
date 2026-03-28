#include "engine/BeamBuilder.h"

#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>

namespace engine {

namespace {

/// 构造矩形截面 Wire（4 个顶点，顺序为 CCW 当从正法线方向看）
/// 截面在 start 处，以 (xd, yd) 为基底
TopoDS_Wire makeRectWire(const gp_Pnt& origin,
                          const gp_Dir& xd, const gp_Dir& yd,
                          double w, double h)
{
    double hw = w / 2.0, hh = h / 2.0;
    gp_Vec vx(xd), vy(yd);
    gp_Pnt p1 = origin.Translated(vx * (-hw) + vy * (-hh));
    gp_Pnt p2 = origin.Translated(vx * ( hw) + vy * (-hh));
    gp_Pnt p3 = origin.Translated(vx * ( hw) + vy * ( hh));
    gp_Pnt p4 = origin.Translated(vx * (-hw) + vy * ( hh));

    BRepBuilderAPI_MakeWire wm;
    wm.Add(BRepBuilderAPI_MakeEdge(p1, p2).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p2, p3).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p3, p4).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p4, p1).Edge());
    return wm.Wire();
}

/// 构造 H 型截面 Wire（12 个顶点）
/// 翼缘厚 tf = h*0.20，腹板厚 tw = w*0.20
TopoDS_Wire makeHSectionWire(const gp_Pnt& origin,
                              const gp_Dir& xd, const gp_Dir& yd,
                              double w, double h)
{
    double w2  = w / 2.0;
    double h2  = h / 2.0;
    double tf  = h * 0.20; // flange thickness
    double tw2 = w * 0.10; // half web thickness

    gp_Vec vx(xd), vy(yd);
    // 12 顶点（从左上角顺时针绕行 H 轮廓）
    gp_Pnt p1  = origin.Translated(vx * (-w2)  + vy * ( h2));       // 顶-左
    gp_Pnt p2  = origin.Translated(vx * ( w2)  + vy * ( h2));       // 顶-右
    gp_Pnt p3  = origin.Translated(vx * ( w2)  + vy * ( h2 - tf));  // 内-右-上
    gp_Pnt p4  = origin.Translated(vx * ( tw2) + vy * ( h2 - tf));  // 腹板-右-上
    gp_Pnt p5  = origin.Translated(vx * ( tw2) + vy * (-h2 + tf));  // 腹板-右-下
    gp_Pnt p6  = origin.Translated(vx * ( w2)  + vy * (-h2 + tf));  // 内-右-下
    gp_Pnt p7  = origin.Translated(vx * ( w2)  + vy * (-h2));       // 底-右
    gp_Pnt p8  = origin.Translated(vx * (-w2)  + vy * (-h2));       // 底-左
    gp_Pnt p9  = origin.Translated(vx * (-w2)  + vy * (-h2 + tf));  // 内-左-下
    gp_Pnt p10 = origin.Translated(vx * (-tw2) + vy * (-h2 + tf));  // 腹板-左-下
    gp_Pnt p11 = origin.Translated(vx * (-tw2) + vy * ( h2 - tf));  // 腹板-左-上
    gp_Pnt p12 = origin.Translated(vx * (-w2)  + vy * ( h2 - tf));  // 内-左-上

    BRepBuilderAPI_MakeWire wm;
    wm.Add(BRepBuilderAPI_MakeEdge(p1,  p2).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p2,  p3).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p3,  p4).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p4,  p5).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p5,  p6).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p6,  p7).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p7,  p8).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p8,  p9).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p9,  p10).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p10, p11).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p11, p12).Edge());
    wm.Add(BRepBuilderAPI_MakeEdge(p12, p1).Edge());
    return wm.Wire();
}

} // anonymous namespace

TopoDS_Shape BeamBuilder::build(
    const gp_Pnt& startPoint,
    const gp_Pnt& endPoint,
    SectionType sectionType,
    double width,
    double height)
{
    gp_Vec extrudeVec(startPoint, endPoint);
    double length = extrudeVec.Magnitude();
    if (length < 1e-9 || width < 1e-9 || height < 1e-9) return {};

    gp_Dir gdir(extrudeVec);

    // 在 startPoint 建立垂直于轴的局部坐标系
    gp_Ax2 localAx(startPoint, gdir);
    gp_Dir xd = localAx.XDirection();
    gp_Dir yd = localAx.YDirection();

    // 构造截面 Wire
    TopoDS_Wire profileWire;
    if (sectionType == SectionType::HSection) {
        profileWire = makeHSectionWire(startPoint, xd, yd, width, height);
    } else {
        profileWire = makeRectWire(startPoint, xd, yd, width, height);
    }

    if (profileWire.IsNull()) return {};

    // 将 Wire 转换为平面 Face
    BRepBuilderAPI_MakeFace faceMaker(profileWire, /*OnlyPlane=*/Standard_True);
    if (!faceMaker.IsDone()) return {};
    TopoDS_Face profileFace = faceMaker.Face();

    // 沿 extrudeVec 方向挤出
    BRepPrimAPI_MakePrism prism(profileFace, extrudeVec);
    if (!prism.IsDone()) return {};
    return prism.Shape();
}

} // namespace engine
