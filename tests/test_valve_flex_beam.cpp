// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "engine/ValveBuilder.h"
#include "engine/FlexJointBuilder.h"
#include "engine/BeamBuilder.h"
#include "engine/AccessoryBuilder.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Dir.hxx>
#include <cmath>

// ===================================================================
// ValveBuilder 测试
// ===================================================================

TEST(ValveBuilder, GateValve_NonNull) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(0, 0, 500);
    auto shape = engine::ValveBuilder::build(start, end, 114.3, 6.0, "gate");
    EXPECT_FALSE(shape.IsNull());
}

TEST(ValveBuilder, BallValve_NonNull) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(0, 0, 400);
    auto shape = engine::ValveBuilder::build(start, end, 88.9, 5.5, "ball");
    EXPECT_FALSE(shape.IsNull());
}

TEST(ValveBuilder, CheckValve_NonNull) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(200, 0, 0);
    auto shape = engine::ValveBuilder::build(start, end, 60.3, 4.0, "check");
    EXPECT_FALSE(shape.IsNull());
}

TEST(ValveBuilder, DefaultType_NonNull) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(0, 0, 300);
    // 默认 valveType = "gate"
    auto shape = engine::ValveBuilder::build(start, end, 114.3, 6.0);
    EXPECT_FALSE(shape.IsNull());
}

TEST(ValveBuilder, Degenerate_SamePoint_ReturnsNull) {
    gp_Pnt p(0, 0, 0);
    auto shape = engine::ValveBuilder::build(p, p, 114.3, 6.0, "gate");
    EXPECT_TRUE(shape.IsNull());
}

// ===================================================================
// FlexJointBuilder 测试
// ===================================================================

TEST(FlexJointBuilder, TwoSegments_NonNull) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(0, 0, 400);
    auto shape = engine::FlexJointBuilder::build(start, end, 114.3, 6.0, 2);
    EXPECT_FALSE(shape.IsNull());
}

TEST(FlexJointBuilder, ThreeSegments_NonNull) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(0, 0, 600);
    auto shape = engine::FlexJointBuilder::build(start, end, 88.9, 5.5, 3);
    EXPECT_FALSE(shape.IsNull());
}

TEST(FlexJointBuilder, DiagonalDirection_NonNull) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(200, 200, 0);
    auto shape = engine::FlexJointBuilder::build(start, end, 60.3, 4.0, 2);
    EXPECT_FALSE(shape.IsNull());
}

TEST(FlexJointBuilder, Degenerate_SamePoint_ReturnsNull) {
    gp_Pnt p(0, 0, 0);
    auto shape = engine::FlexJointBuilder::build(p, p, 114.3, 6.0, 3);
    EXPECT_TRUE(shape.IsNull());
}

// ===================================================================
// BeamBuilder 测试
// ===================================================================

TEST(BeamBuilder, Rectangular_NonNull) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(0, 0, 1000);
    auto shape = engine::BeamBuilder::build(
        start, end, engine::BeamBuilder::SectionType::Rectangular, 100, 150);
    EXPECT_FALSE(shape.IsNull());
}

TEST(BeamBuilder, Rectangular_BoundingBoxLength) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(0, 0, 1000);
    auto shape = engine::BeamBuilder::build(
        start, end, engine::BeamBuilder::SectionType::Rectangular, 100, 100);
    ASSERT_FALSE(shape.IsNull());

    Bnd_Box bbox;
    BRepBndLib::Add(shape, bbox);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    // Z 方向长度应为 1000 mm，宽容差 ±1 mm
    double zLen = zmax - zmin;
    EXPECT_NEAR(zLen, 1000.0, 1.0);
}

TEST(BeamBuilder, HSection_NonNull) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(0, 0, 2000);
    auto shape = engine::BeamBuilder::build(
        start, end, engine::BeamBuilder::SectionType::HSection, 200, 200);
    EXPECT_FALSE(shape.IsNull());
}

TEST(BeamBuilder, HSection_CorrectLength) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(0, 0, 1500);
    auto shape = engine::BeamBuilder::build(
        start, end, engine::BeamBuilder::SectionType::HSection, 150, 200);
    ASSERT_FALSE(shape.IsNull());

    Bnd_Box bbox;
    BRepBndLib::Add(shape, bbox);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    double zLen = zmax - zmin;
    EXPECT_NEAR(zLen, 1500.0, 1.0);
}

TEST(BeamBuilder, DiagonalBeam_NonNull) {
    gp_Pnt start(0, 0, 0);
    gp_Pnt end(300, 400, 0);
    auto shape = engine::BeamBuilder::build(
        start, end, engine::BeamBuilder::SectionType::Rectangular, 80, 120);
    EXPECT_FALSE(shape.IsNull());
}

TEST(BeamBuilder, Degenerate_SamePoint_ReturnsNull) {
    gp_Pnt p(0, 0, 0);
    auto shape = engine::BeamBuilder::build(
        p, p, engine::BeamBuilder::SectionType::Rectangular, 100, 100);
    EXPECT_TRUE(shape.IsNull());
}

// ===================================================================
// AccessoryBuilder 测试
// ===================================================================

TEST(AccessoryBuilder, Flange_NonNull) {
    gp_Pnt center(0, 0, 0);
    gp_Dir normal(0, 0, 1);
    auto shape = engine::AccessoryBuilder::buildFlange(center, normal, 114.3, 20.0);
    EXPECT_FALSE(shape.IsNull());
}

TEST(AccessoryBuilder, Flange_DiagonalNormal_NonNull) {
    gp_Pnt center(100, 200, 300);
    gp_Dir normal(1, 0, 0);
    auto shape = engine::AccessoryBuilder::buildFlange(center, normal, 88.9, 15.0);
    EXPECT_FALSE(shape.IsNull());
}

TEST(AccessoryBuilder, Bracket_NonNull) {
    gp_Pnt base(0, 0, 0);
    gp_Pnt top(0, 0, 300);
    auto shape = engine::AccessoryBuilder::buildBracket(base, top, 50.0);
    EXPECT_FALSE(shape.IsNull());
}

TEST(AccessoryBuilder, Bracket_DiagonalDirection_NonNull) {
    gp_Pnt base(0, 0, 0);
    gp_Pnt top(100, 100, 200);
    auto shape = engine::AccessoryBuilder::buildBracket(base, top, 40.0);
    EXPECT_FALSE(shape.IsNull());
}
