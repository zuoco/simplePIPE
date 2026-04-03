// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "geometry/ShapeBuilder.h"
#include "geometry/BooleanOps.h"
#include "geometry/ShapeTransform.h"

#include <BRep_Builder.hxx>
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <gp_Circ.hxx>
#include <gp_Ax2.hxx>
#include <TopoDS_Shape.hxx>

// 辅助：计算形体体积
static double shapeVolume(const TopoDS_Shape& s) {
    GProp_GProps props;
    BRepGProp::VolumeProperties(s, props);
    return props.Mass();
}

// 辅助：获取形体包围盒 Z 范围
static std::pair<double, double> bboxZ(const TopoDS_Shape& s) {
    Bnd_Box box;
    BRepBndLib::Add(s, box);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    return {zmin, zmax};
}

// ============================================================
// ShapeBuilder Tests
// ============================================================

TEST(ShapeBuilder, MakeCylinder_NotNull) {
    auto s = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    EXPECT_FALSE(s.IsNull());
}

TEST(ShapeBuilder, MakeCylinder_Volume) {
    const double r = 5.0, h = 20.0;
    auto s = geometry::ShapeBuilder::makeCylinder(r, h);
    double expected = M_PI * r * r * h;
    EXPECT_NEAR(shapeVolume(s), expected, expected * 1e-6);
}

TEST(ShapeBuilder, MakeTorus_NotNull) {
    auto s = geometry::ShapeBuilder::makeTorus(20.0, 5.0);
    EXPECT_FALSE(s.IsNull());
}

TEST(ShapeBuilder, MakeCone_NotNull) {
    auto s = geometry::ShapeBuilder::makeCone(10.0, 5.0, 15.0);
    EXPECT_FALSE(s.IsNull());
}

TEST(ShapeBuilder, MakeCone_Volume) {
    const double r1 = 10.0, r2 = 5.0, h = 15.0;
    auto s = geometry::ShapeBuilder::makeCone(r1, r2, h);
    // V = π*h/3 * (r1² + r1*r2 + r2²)
    double expected = M_PI * h / 3.0 * (r1*r1 + r1*r2 + r2*r2);
    EXPECT_NEAR(shapeVolume(s), expected, expected * 1e-6);
}

TEST(ShapeBuilder, MakePipeShell_NotNull) {
    // 构造一段直线脊线（沿 Z 轴）作为 wire
    gp_Pnt p1(0, 0, 0), p2(0, 0, 50);
    BRepBuilderAPI_MakeEdge edgeMaker(p1, p2);
    ASSERT_TRUE(edgeMaker.IsDone());
    BRepBuilderAPI_MakeWire wireMaker(edgeMaker.Edge());
    ASSERT_TRUE(wireMaker.IsDone());
    auto s = geometry::ShapeBuilder::makePipeShell(wireMaker.Wire(), 5.0);
    EXPECT_FALSE(s.IsNull());
}

// ============================================================
// BooleanOps Tests
// ============================================================

TEST(BooleanOps, Cut_NotNull) {
    auto outer = geometry::ShapeBuilder::makeCylinder(10.0, 30.0);
    auto inner = geometry::ShapeBuilder::makeCylinder(8.0, 30.0);
    auto result = geometry::BooleanOps::cut(outer, inner);
    EXPECT_FALSE(result.IsNull());
}

TEST(BooleanOps, Cut_VolumeReduced) {
    auto outer = geometry::ShapeBuilder::makeCylinder(10.0, 30.0);
    auto inner = geometry::ShapeBuilder::makeCylinder(8.0, 30.0);
    double volOuter = shapeVolume(outer);
    double volResult = shapeVolume(geometry::BooleanOps::cut(outer, inner));
    EXPECT_GT(volOuter, volResult);
    EXPECT_GT(volResult, 0.0);
}

TEST(BooleanOps, Fuse_NotNull) {
    auto box1 = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    auto box2 = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    // 沿 X 轴偏移第二个以确保有实质合并
    auto shifted = geometry::ShapeTransform::translate(box2, gp_Vec(6.0, 0, 0));
    auto result = geometry::BooleanOps::fuse(box1, shifted);
    EXPECT_FALSE(result.IsNull());
}

TEST(BooleanOps, Fuse_VolumeIncreased) {
    auto c1 = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    auto c2 = geometry::ShapeTransform::translate(
        geometry::ShapeBuilder::makeCylinder(5.0, 20.0), gp_Vec(20.0, 0, 0));
    double volSingle = shapeVolume(c1);
    double volFuse = shapeVolume(geometry::BooleanOps::fuse(c1, c2));
    EXPECT_NEAR(volFuse, 2.0 * volSingle, volSingle * 1e-6);
}

// ============================================================
// ShapeTransform Tests
// ============================================================

TEST(ShapeTransform, Translate_BBoxShifts) {
    auto s = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    auto [z0min, z0max] = bboxZ(s);
    auto translated = geometry::ShapeTransform::translate(s, gp_Vec(0, 0, 100.0));
    auto [z1min, z1max] = bboxZ(translated);
    EXPECT_NEAR(z1min - z0min, 100.0, 1e-6);
    EXPECT_NEAR(z1max - z0max, 100.0, 1e-6);
}

TEST(ShapeTransform, Rotate_VolumePreserved) {
    auto s = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    double volBefore = shapeVolume(s);
    gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
    auto rotated = geometry::ShapeTransform::rotate(s, axis, M_PI / 4.0);
    EXPECT_NEAR(shapeVolume(rotated), volBefore, volBefore * 1e-6);
}

TEST(ShapeTransform, Transform_Identity) {
    auto s = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    double volBefore = shapeVolume(s);
    gp_Trsf identity;
    auto s2 = geometry::ShapeTransform::transform(s, identity);
    EXPECT_NEAR(shapeVolume(s2), volBefore, volBefore * 1e-6);
}
