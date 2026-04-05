// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "visualization/OcctToVsg.h"

#include "geometry/ShapeMesher.h"

#include <vsg/core/Array.h>
#include <vsg/maths/vec3.h>

namespace visualization {

vsg::ref_ptr<vsg::VertexIndexDraw> toVsgGeometry(const TopoDS_Shape& shape,
                                                  double deflection) {
    // Step 1: OCCT 三角化
    geometry::MeshData mesh = geometry::ShapeMesher::mesh(shape, deflection);
    if (mesh.vertices.empty() || mesh.indices.empty())
        return {};

    const uint32_t numVerts = static_cast<uint32_t>(mesh.vertices.size());
    const uint32_t numIdx   = static_cast<uint32_t>(mesh.indices.size());

    // Step 2: 构建 VSG 顶点和法线数组（binding 0 = 顶点, binding 1 = 法线）
    auto vsgVertices = vsg::vec3Array::create(numVerts);
    auto vsgNormals  = vsg::vec3Array::create(numVerts);

    for (uint32_t i = 0; i < numVerts; ++i) {
        const auto& v = mesh.vertices[i];
        const auto& n = mesh.normals[i];
        (*vsgVertices)[i] = vsg::vec3{v[0], v[1], v[2]};
        (*vsgNormals)[i]  = vsg::vec3{n[0], n[1], n[2]};
    }

    // Step 3: 构建 VSG 索引数组（uint32，支持大网格）
    auto vsgIndices = vsg::uintArray::create(numIdx);
    for (uint32_t i = 0; i < numIdx; ++i) {
        (*vsgIndices)[i] = mesh.indices[i];
    }

    // Step 4: 组装 VertexIndexDraw
    auto vid = vsg::VertexIndexDraw::create();
    vid->assignArrays({vsgVertices, vsgNormals});
    vid->assignIndices(vsgIndices);
    vid->indexCount    = numIdx;
    vid->instanceCount = 1;

    return vid;
}

} // namespace visualization
