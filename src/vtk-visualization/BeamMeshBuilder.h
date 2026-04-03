// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "geometry/OcctTypes.h"

#include <vtkSmartPointer.h>

#include <vector>

class vtkPolyData;

namespace vtk_vis {

/// 根据中心线构建梁单元线模型（vtkPolyLine）
/// @param centerline 按顺序排列的中心线点
/// @return 若点数不足 2，返回 nullptr
vtkSmartPointer<vtkPolyData> buildBeamMesh(const std::vector<gp_Pnt>& centerline);

} // namespace vtk_vis
