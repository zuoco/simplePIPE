#pragma once

#include "geometry/OcctTypes.h"
#include <gp_Ax1.hxx>
#include <gp_Vec.hxx>
#include <gp_Trsf.hxx>

namespace geometry {

class ShapeTransform {
public:
    /// 沿向量 vec 平移
    static TopoDS_Shape translate(const TopoDS_Shape& shape, const gp_Vec& vec);

    /// 绕轴 axis 旋转 angle（弧度）
    static TopoDS_Shape rotate(const TopoDS_Shape& shape, const gp_Ax1& axis, double angle);

    /// 应用任意变换
    static TopoDS_Shape transform(const TopoDS_Shape& shape, const gp_Trsf& trsf);
};

} // namespace geometry
