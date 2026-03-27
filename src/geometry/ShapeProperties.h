#pragma once

#include "geometry/OcctTypes.h"

namespace geometry {

class ShapeProperties {
public:
    /// 计算形体体积（mm³ 或工程单位³）
    static double volume(const TopoDS_Shape& shape);

    /// 计算形体表面积
    static double surfaceArea(const TopoDS_Shape& shape);
};

} // namespace geometry
