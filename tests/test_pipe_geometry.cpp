#include <gtest/gtest.h>
#include "engine/RunBuilder.h"
#include "engine/BendBuilder.h"
#include "engine/BendCalculator.h"
#include "engine/ReducerBuilder.h"
#include "engine/TeeBuilder.h"
#include "engine/GeometryDeriver.h"
#include "geometry/ShapeProperties.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"

#include <gp_Pnt.hxx>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================
// RunBuilder Tests
// ============================================================

TEST(RunBuilder, BasicCylinderShell) {
    // 1000mm length, OD=168.3, WT=7.11
    auto shape = engine::RunBuilder::build(
        gp_Pnt(0, 0, 0), gp_Pnt(1000, 0, 0), 168.3, 7.11);

    ASSERT_FALSE(shape.IsNull());

    // Volume of hollow cylinder = π × (R² - r²) × L
    double outerR = 168.3 / 2.0;
    double innerR = outerR - 7.11;
    double expectedVol = M_PI * (outerR * outerR - innerR * innerR) * 1000.0;
    double actualVol = geometry::ShapeProperties::volume(shape);
    EXPECT_NEAR(actualVol, expectedVol, expectedVol * 0.01); // 1% tolerance
}

TEST(RunBuilder, ShortPipe) {
    auto shape = engine::RunBuilder::build(
        gp_Pnt(0, 0, 0), gp_Pnt(100, 0, 0), 50.0, 5.0);
    ASSERT_FALSE(shape.IsNull());
}

TEST(RunBuilder, DiagonalPipe) {
    // Pipe along diagonal direction
    auto shape = engine::RunBuilder::build(
        gp_Pnt(0, 0, 0), gp_Pnt(500, 500, 500), 100.0, 10.0);
    ASSERT_FALSE(shape.IsNull());
}

TEST(RunBuilder, CoincidentPoints_ReturnsEmpty) {
    auto shape = engine::RunBuilder::build(
        gp_Pnt(100, 0, 0), gp_Pnt(100, 0, 0), 50.0, 5.0);
    EXPECT_TRUE(shape.IsNull());
}

TEST(RunBuilder, ZeroWallThickness_ReturnsEmpty) {
    auto shape = engine::RunBuilder::build(
        gp_Pnt(0, 0, 0), gp_Pnt(100, 0, 0), 50.0, 25.0); // innerR = 0
    EXPECT_TRUE(shape.IsNull());
}

// ============================================================
// BendBuilder Tests
// ============================================================

TEST(BendBuilder, Bend90) {
    auto bend = engine::BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0), gp_Pnt(500, 0, 0), gp_Pnt(500, 500, 0),
        168.3, 1.5);
    ASSERT_TRUE(bend.has_value());

    auto shape = engine::BendBuilder::build(*bend, 168.3, 7.11);
    ASSERT_FALSE(shape.IsNull());

    double vol = geometry::ShapeProperties::volume(shape);
    EXPECT_GT(vol, 0.0);
}

TEST(BendBuilder, Bend45) {
    double cos45 = std::cos(M_PI / 4.0);
    double sin45 = std::sin(M_PI / 4.0);
    auto bend = engine::BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0), gp_Pnt(500, 0, 0),
        gp_Pnt(500 + 500 * cos45, 500 * sin45, 0),
        100.0, 2.0);
    ASSERT_TRUE(bend.has_value());

    auto shape = engine::BendBuilder::build(*bend, 100.0, 5.0);
    ASSERT_FALSE(shape.IsNull());
}

TEST(BendBuilder, Bend120) {
    double cos120 = std::cos(2.0 * M_PI / 3.0);
    double sin120 = std::sin(2.0 * M_PI / 3.0);
    auto bend = engine::BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0), gp_Pnt(500, 0, 0),
        gp_Pnt(500 + 500 * cos120, 500 * sin120, 0),
        100.0, 1.5);
    ASSERT_TRUE(bend.has_value());

    auto shape = engine::BendBuilder::build(*bend, 100.0, 5.0);
    ASSERT_FALSE(shape.IsNull());
}

// ============================================================
// ReducerBuilder Tests
// ============================================================

TEST(ReducerBuilder, BasicReducer) {
    // Large end OD=168.3, small end OD=114.3
    auto shape = engine::ReducerBuilder::build(
        gp_Pnt(0, 0, 0), gp_Pnt(200, 0, 0),
        168.3, 114.3, 7.11);

    ASSERT_FALSE(shape.IsNull());
    double vol = geometry::ShapeProperties::volume(shape);
    EXPECT_GT(vol, 0.0);
}

TEST(ReducerBuilder, VerticalReducer) {
    auto shape = engine::ReducerBuilder::build(
        gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 300),
        200.0, 100.0, 10.0);
    ASSERT_FALSE(shape.IsNull());
}

TEST(ReducerBuilder, CoincidentPoints_ReturnsEmpty) {
    auto shape = engine::ReducerBuilder::build(
        gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0),
        168.3, 114.3, 7.11);
    EXPECT_TRUE(shape.IsNull());
}

// ============================================================
// TeeBuilder Tests
// ============================================================

TEST(TeeBuilder, BasicTee) {
    // Small geometry for fast boolean ops in Debug build
    auto shape = engine::TeeBuilder::build(
        gp_Pnt(0, 0, 0),    // mainStart
        gp_Pnt(200, 0, 0),  // mainEnd
        gp_Pnt(100, 0, 0),  // branchPoint
        gp_Pnt(100, 150, 0),// branchEnd
        60.0, 40.0, 5.0);

    ASSERT_FALSE(shape.IsNull());
}

TEST(TeeBuilder, TeeVolumeGreaterThanSinglePipe) {
    auto mainPipe = engine::RunBuilder::build(
        gp_Pnt(0, 0, 0), gp_Pnt(200, 0, 0), 60.0, 5.0);
    auto tee = engine::TeeBuilder::build(
        gp_Pnt(0, 0, 0), gp_Pnt(200, 0, 0),
        gp_Pnt(100, 0, 0), gp_Pnt(100, 150, 0),
        60.0, 40.0, 5.0);

    ASSERT_FALSE(mainPipe.IsNull());
    ASSERT_FALSE(tee.IsNull());

    double mainVol = geometry::ShapeProperties::volume(mainPipe);
    double teeVol  = geometry::ShapeProperties::volume(tee);
    EXPECT_GT(teeVol, mainVol);
}

TEST(TeeBuilder, EqualOdTee) {
    auto shape = engine::TeeBuilder::build(
        gp_Pnt(0, 0, 0), gp_Pnt(200, 0, 0),
        gp_Pnt(100, 0, 0), gp_Pnt(100, 150, 0),
        50.0, 50.0, 5.0);
    ASSERT_FALSE(shape.IsNull());
}

// ============================================================
// GeometryDeriver Tests
// ============================================================

TEST(GeometryDeriver, DeriveRun) {
    auto spec = std::make_shared<model::PipeSpec>("TestSpec");
    spec->setOd(168.3);
    spec->setWallThickness(7.11);

    auto point = std::make_shared<model::PipePoint>("P1", model::PipePointType::Run);
    point->setPipeSpec(spec);

    auto shape = engine::GeometryDeriver::deriveGeometry(
        gp_Pnt(0, 0, 0), point, gp_Pnt(1000, 0, 0));
    ASSERT_FALSE(shape.IsNull());
}

TEST(GeometryDeriver, DeriveBend) {
    auto spec = std::make_shared<model::PipeSpec>("TestSpec");
    spec->setOd(168.3);
    spec->setWallThickness(7.11);

    auto point = std::make_shared<model::PipePoint>(
        "B1", model::PipePointType::Bend, gp_Pnt(500, 0, 0));
    point->setPipeSpec(spec);
    point->setParam("bendMultiplier", 1.5);

    auto shape = engine::GeometryDeriver::deriveGeometry(
        gp_Pnt(0, 0, 0), point, gp_Pnt(500, 500, 0));
    ASSERT_FALSE(shape.IsNull());
}

TEST(GeometryDeriver, DeriveReducer) {
    auto spec = std::make_shared<model::PipeSpec>("TestSpec");
    spec->setOd(168.3);
    spec->setWallThickness(7.11);

    auto point = std::make_shared<model::PipePoint>("R1", model::PipePointType::Reducer);
    point->setPipeSpec(spec);
    point->setParam("endOD", 114.3);

    auto shape = engine::GeometryDeriver::deriveGeometry(
        gp_Pnt(0, 0, 0), point, gp_Pnt(200, 0, 0));
    ASSERT_FALSE(shape.IsNull());
}

TEST(GeometryDeriver, DeriveTee) {
    auto spec = std::make_shared<model::PipeSpec>("TestSpec");
    spec->setOd(60.0);
    spec->setWallThickness(5.0);

    auto point = std::make_shared<model::PipePoint>(
        "T1", model::PipePointType::Tee, gp_Pnt(100, 0, 0));
    point->setPipeSpec(spec);
    point->setParam("branchEndX", 100.0);
    point->setParam("branchEndY", 150.0);
    point->setParam("branchEndZ", 0.0);
    point->setParam("branchOD", 40.0);

    auto shape = engine::GeometryDeriver::deriveGeometry(
        gp_Pnt(0, 0, 0), point, gp_Pnt(200, 0, 0));
    ASSERT_FALSE(shape.IsNull());
}

TEST(GeometryDeriver, NoPipeSpec_ReturnsEmpty) {
    auto point = std::make_shared<model::PipePoint>("P1", model::PipePointType::Run);
    // No PipeSpec set
    auto shape = engine::GeometryDeriver::deriveGeometry(
        gp_Pnt(0, 0, 0), point, gp_Pnt(100, 0, 0));
    EXPECT_TRUE(shape.IsNull());
}

TEST(GeometryDeriver, NullPoint_ReturnsEmpty) {
    auto shape = engine::GeometryDeriver::deriveGeometry(
        gp_Pnt(0, 0, 0), nullptr, gp_Pnt(100, 0, 0));
    EXPECT_TRUE(shape.IsNull());
}
