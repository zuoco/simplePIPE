#pragma once

#include "geometry/OcctTypes.h"
#include <TopoDS_Wire.hxx>

namespace geometry {

// ============================================================
// ShapeBuilder — OCCT 基本几何体构造器
// ============================================================

class ShapeBuilder {
public:
    // 圆柱体 (轴向 Z，原点在底面圆心)
    // radius: 半径 (mm), height: 高度 (mm)
    static TopoDS_Shape makeCylinder(double radius, double height);

    // 圆环体
    // majorR: 大环半径 (mm), minorR: 管截面半径 (mm)
    // angle: 扫掠角度 (rad)，2π = 完整圆环
    static TopoDS_Shape makeTorus(double majorR, double minorR, double angle = 2.0 * M_PI);

    // 圆锥/截锥
    // r1: 底面半径 (mm), r2: 顶面半径 (mm，0 = 尖锥), height: 高度 (mm)
    static TopoDS_Shape makeCone(double r1, double r2, double height);

    // 沿 Wire 扫掠圆截面生成管体
    // wire: 路径线, radius: 管截面半径 (mm)
    static TopoDS_Shape makePipeShell(const TopoDS_Wire& wire, double radius);
};

} // namespace geometry
