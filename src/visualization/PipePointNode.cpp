#include "visualization/PipePointNode.h"

#include <vsg/core/Array.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/VertexIndexDraw.h>

namespace visualization {

namespace {

/// 构建单位立方体 [-0.5, 0.5]^3 的 VertexIndexDraw（平面法线，无 GPU 依赖）
/// 24 顶点（每面 4 个），36 索引（12 个三角形）
vsg::ref_ptr<vsg::VertexIndexDraw> buildBoxMesh() {
    constexpr float H = 0.5f;

    // 每面 4 顶点，顺序：右下后, 右上后, 右上前, 右下前 (视面法线方向)
    const vsg::vec3 rawVtx[24] = {
        // +X 面 (法线 = +X)
        { H, -H, -H}, { H,  H, -H}, { H,  H,  H}, { H, -H,  H},
        // -X 面 (法线 = -X)
        {-H,  H, -H}, {-H, -H, -H}, {-H, -H,  H}, {-H,  H,  H},
        // +Y 面 (法线 = +Y)
        {-H,  H, -H}, { H,  H, -H}, { H,  H,  H}, {-H,  H,  H},
        // -Y 面 (法线 = -Y)
        { H, -H, -H}, {-H, -H, -H}, {-H, -H,  H}, { H, -H,  H},
        // +Z 面 (法线 = +Z)
        {-H, -H,  H}, { H, -H,  H}, { H,  H,  H}, {-H,  H,  H},
        // -Z 面 (法线 = -Z)
        { H, -H, -H}, {-H, -H, -H}, {-H,  H, -H}, { H,  H, -H},
    };

    const vsg::vec3 rawNrm[24] = {
        { 1, 0, 0}, { 1, 0, 0}, { 1, 0, 0}, { 1, 0, 0},  // +X
        {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0},  // -X
        { 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0},  // +Y
        { 0,-1, 0}, { 0,-1, 0}, { 0,-1, 0}, { 0,-1, 0},  // -Y
        { 0, 0, 1}, { 0, 0, 1}, { 0, 0, 1}, { 0, 0, 1},  // +Z
        { 0, 0,-1}, { 0, 0,-1}, { 0, 0,-1}, { 0, 0,-1},  // -Z
    };

    const uint32_t rawIdx[36] = {
         0,  1,  2,   0,  2,  3,   // +X
         4,  5,  6,   4,  6,  7,   // -X
         8,  9, 10,   8, 10, 11,   // +Y
        12, 13, 14,  12, 14, 15,   // -Y
        16, 17, 18,  16, 18, 19,   // +Z
        20, 21, 22,  20, 22, 23,   // -Z
    };

    auto vertices = vsg::vec3Array::create(24);
    auto normals  = vsg::vec3Array::create(24);
    auto indices  = vsg::uintArray::create(36);

    for (int i = 0; i < 24; ++i) {
        (*vertices)[i] = rawVtx[i];
        (*normals)[i]  = rawNrm[i];
    }
    for (int i = 0; i < 36; ++i) {
        (*indices)[i] = rawIdx[i];
    }

    auto vid = vsg::VertexIndexDraw::create();
    vid->assignArrays({vertices, normals});
    vid->assignIndices(indices);
    vid->indexCount    = 36;
    vid->instanceCount = 1;
    return vid;
}

} // anonymous namespace

vsg::ref_ptr<vsg::MatrixTransform> createPipePointNode(
    double x, double y, double z, float size, const vsg::vec4& /*color*/)
{
    auto node = vsg::MatrixTransform::create();

    // 平移到管点坐标，并缩放为 size mm 的立方体
    const double s = static_cast<double>(size);
    node->matrix = vsg::translate(x, y, z) * vsg::scale(s, s, s);

    node->addChild(buildBoxMesh());
    return node;
}

} // namespace visualization
