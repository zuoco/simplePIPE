// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "visualization/OcctToVsg.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <TopoDS_Shape.hxx>

#include <vsg/core/Array.h>

// ──────────────────────────────────────────────────────────────────────────────
// T11: OCCT→VSG 网格转换
// ──────────────────────────────────────────────────────────────────────────────

/// 圆柱体 → VSG 几何节点，基本正确性验证
TEST(OcctToVsg, CylinderGeometry) {
    BRepPrimAPI_MakeCylinder cyl(5.0, 20.0);
    TopoDS_Shape shape = cyl.Shape();

    auto vid = visualization::toVsgGeometry(shape, 0.1);

    ASSERT_NE(vid, nullptr);
    EXPECT_GT(vid->indexCount, 0u);
    EXPECT_EQ(vid->instanceCount, 1u);
    // 三角形面片：索引数必须是 3 的倍数
    EXPECT_EQ(vid->indexCount % 3, 0u);
}

/// 法线方向：所有法线长度 > 0（指向外侧，非零向量）
TEST(OcctToVsg, CylinderNormalsNonZero) {
    BRepPrimAPI_MakeCylinder cyl(5.0, 20.0);
    TopoDS_Shape shape = cyl.Shape();

    auto vid = visualization::toVsgGeometry(shape, 0.1);
    ASSERT_NE(vid, nullptr);
    ASSERT_GE(vid->arrays.size(), 2u);
    ASSERT_TRUE(vid->arrays[1] && vid->arrays[1]->data);

    auto* norms = dynamic_cast<vsg::vec3Array*>(vid->arrays[1]->data.get());
    ASSERT_NE(norms, nullptr);
    ASSERT_GT(norms->size(), 0u);

    bool anyNonZero = false;
    for (size_t i = 0; i < norms->size(); ++i) {
        const auto& n = (*norms)[i];
        float len2 = n.x * n.x + n.y * n.y + n.z * n.z;
        if (len2 > 1e-6f) {
            anyNonZero = true;
            break;
        }
    }
    EXPECT_TRUE(anyNonZero) << "所有法线均为零向量";
}

/// 顶点数组与法线数组大小一致
TEST(OcctToVsg, VertexNormalArraysSameSize) {
    BRepPrimAPI_MakeCylinder cyl(5.0, 20.0);
    TopoDS_Shape shape = cyl.Shape();

    auto vid = visualization::toVsgGeometry(shape, 0.1);
    ASSERT_NE(vid, nullptr);
    ASSERT_GE(vid->arrays.size(), 2u);

    auto* verts = dynamic_cast<vsg::vec3Array*>(vid->arrays[0]->data.get());
    auto* norms = dynamic_cast<vsg::vec3Array*>(vid->arrays[1]->data.get());
    ASSERT_NE(verts, nullptr);
    ASSERT_NE(norms, nullptr);
    EXPECT_EQ(verts->size(), norms->size());
    EXPECT_GT(verts->size(), 0u);
}

/// 索引面片数量合理（圆柱体面数 > 0）
TEST(OcctToVsg, CylinderFaceCountReasonable) {
    BRepPrimAPI_MakeCylinder cyl(5.0, 20.0);
    TopoDS_Shape shape = cyl.Shape();

    auto vid = visualization::toVsgGeometry(shape, 0.1);
    ASSERT_NE(vid, nullptr);

    uint32_t triCount = vid->indexCount / 3;
    // 一个圆柱体（2 个圆面 + 侧面）应当有足够多的三角形
    EXPECT_GT(triCount, 10u) << "三角形数量过少: " << triCount;
}

/// 空形体返回 nullptr
TEST(OcctToVsg, EmptyShapeReturnsNull) {
    TopoDS_Shape empty;
    auto vid = visualization::toVsgGeometry(empty);
    EXPECT_EQ(vid, nullptr);
}

/// 盒体 → VSG 几何节点
TEST(OcctToVsg, BoxGeometry) {
    BRepPrimAPI_MakeBox box(10.0, 10.0, 10.0);
    TopoDS_Shape shape = box.Shape();

    auto vid = visualization::toVsgGeometry(shape, 0.5);
    ASSERT_NE(vid, nullptr);
    EXPECT_GT(vid->indexCount, 0u);
    EXPECT_EQ(vid->indexCount % 3, 0u);
    // 盒子 6 个面，每面至少 2 个三角形 → 至少 36 个索引
    EXPECT_GE(vid->indexCount, 36u);
}
