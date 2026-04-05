// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "geometry/OcctTypes.h"

#include <array>
#include <vector>

namespace geometry {

/// 三角化网格数据（世界坐标系）
struct MeshData {
    std::vector<std::array<float, 3>> vertices; ///< XYZ 顶点坐标
    std::vector<std::array<float, 3>> normals;  ///< XYZ 法线（与 vertices 一一对应）
    std::vector<uint32_t>             indices;  ///< 三角面片索引（每组 3 个）
};

class ShapeMesher {
public:
    /// 对 Shape 进行三角化，返回网格数据（世界坐标系）
    /// @param shape      要三角化的 BREP 形体
    /// @param deflection 线性偏差（越小越精细，默认 0.1）
    static MeshData mesh(const TopoDS_Shape& shape, double deflection = 0.1);
};

} // namespace geometry
