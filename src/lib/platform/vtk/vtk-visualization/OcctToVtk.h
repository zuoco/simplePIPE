// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "geometry/OcctTypes.h"

#include <vtkSmartPointer.h>

class vtkPolyData;

namespace vtk_vis {

/// 将 OCCT TopoDS_Shape 转换为 VTK 多边形网格
/// @param shape OCCT BREP 形体
/// @param deflection 三角化线性偏差（越小越精细，默认 0.1）
/// @return 若输入为空或三角化失败，返回 nullptr
vtkSmartPointer<vtkPolyData> toVtkPolyData(const TopoDS_Shape& shape,
                                           double deflection = 0.1);

} // namespace vtk_vis
