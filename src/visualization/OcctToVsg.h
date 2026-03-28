#pragma once

#include "geometry/OcctTypes.h"

#include <vsg/nodes/VertexIndexDraw.h>

namespace visualization {

/// 将 OCCT TopoDS_Shape 转换为 VSG 几何节点
/// @param shape      OCCT BREP 形体
/// @param deflection 三角化线性偏差（越小越精细，默认 0.1）
/// @return           vsg::VertexIndexDraw（binding 0=顶点, binding 1=法线）；
///                   若形体为空或三角化失败，返回 nullptr
vsg::ref_ptr<vsg::VertexIndexDraw> toVsgGeometry(const TopoDS_Shape& shape,
                                                  double deflection = 0.1);

} // namespace visualization
