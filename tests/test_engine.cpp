// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include "engine/BendCalculator.h"
#include "foundation/Math.h"

#include <gp_Pnt.hxx>
#include <cmath>

using engine::BendCalculator;
using engine::BendResult;

static constexpr double kTol = 1e-6;
static constexpr double PI   = foundation::math::PI;

// Helper: verify all three arc points are at distance R from center
static void verifyArcPoints(const BendResult& r) {
    EXPECT_NEAR(r.nearPoint.Distance(r.arcCenter), r.bendRadius, kTol);
    EXPECT_NEAR(r.farPoint.Distance(r.arcCenter),  r.bendRadius, kTol);
    EXPECT_NEAR(r.midPoint.Distance(r.arcCenter),  r.bendRadius, kTol);
}

// ============================================================
// 1. 90° 弯头 — XY 平面
// ============================================================
TEST(BendCalculator, Bend90_XY) {
    // A05→A06 along +X, A06→A07 along +Y => 90° bend
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),     // A05
        gp_Pnt(100, 0, 0),   // A06
        gp_Pnt(100, 100, 0), // A07
        50.0, 1.5);          // R = 75

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->bendAngle,  PI / 2.0, kTol);
    EXPECT_NEAR(result->bendRadius, 75.0, kTol);

    // tanLen = 75 * tan(45°) = 75
    EXPECT_NEAR(result->nearPoint.X(), 25.0, kTol);
    EXPECT_NEAR(result->nearPoint.Y(),  0.0, kTol);
    EXPECT_NEAR(result->farPoint.X(), 100.0, kTol);
    EXPECT_NEAR(result->farPoint.Y(),  75.0, kTol);

    verifyArcPoints(*result);
}

// ============================================================
// 2. 45° 弯头 — XY 平面
// ============================================================
TEST(BendCalculator, Bend45_XY) {
    double cos45 = std::cos(foundation::math::degToRad(45.0));
    double sin45 = std::sin(foundation::math::degToRad(45.0));
    // d2 = (cos45, sin45, 0) makes 45° angle with d1 = (1,0,0)
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),
        gp_Pnt(100, 0, 0),
        gp_Pnt(100 + 100 * cos45, 100 * sin45, 0),
        50.0, 1.5);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->bendAngle,  PI / 4.0, kTol);
    EXPECT_NEAR(result->bendRadius, 75.0, kTol);
    verifyArcPoints(*result);
}

// ============================================================
// 3. 60° 弯头 — XZ 平面 (3D 验证)
// ============================================================
TEST(BendCalculator, Bend60_XZ) {
    double cos60 = std::cos(foundation::math::degToRad(60.0));
    double sin60 = std::sin(foundation::math::degToRad(60.0));
    // d1 = (1,0,0), d2 = (cos60, 0, sin60) — bend in XZ plane
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),
        gp_Pnt(100, 0, 0),
        gp_Pnt(100 + 100 * cos60, 0, 100 * sin60),
        60.0, 2.0);  // R = 120

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->bendAngle,  PI / 3.0, kTol);
    EXPECT_NEAR(result->bendRadius, 120.0, kTol);
    verifyArcPoints(*result);
}

// ============================================================
// 4. 120° 弯头 — XY 平面
// ============================================================
TEST(BendCalculator, Bend120_XY) {
    double cos120 = std::cos(foundation::math::degToRad(120.0));
    double sin120 = std::sin(foundation::math::degToRad(120.0));
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),
        gp_Pnt(100, 0, 0),
        gp_Pnt(100 + 100 * cos120, 100 * sin120, 0),
        50.0, 1.5); // R = 75

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->bendAngle,  2.0 * PI / 3.0, kTol);
    EXPECT_NEAR(result->bendRadius, 75.0, kTol);
    verifyArcPoints(*result);
}

// ============================================================
// 5. 135° 弯头 — 大角度
// ============================================================
TEST(BendCalculator, Bend135_XY) {
    double cos135 = std::cos(foundation::math::degToRad(135.0));
    double sin135 = std::sin(foundation::math::degToRad(135.0));
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),
        gp_Pnt(200, 0, 0),
        gp_Pnt(200 + 100 * cos135, 100 * sin135, 0),
        50.0, 5.0); // R = 250

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->bendAngle,  3.0 * PI / 4.0, kTol);
    EXPECT_NEAR(result->bendRadius, 250.0, kTol);
    verifyArcPoints(*result);
}

// ============================================================
// 6. 90° 弯头 — 不同倍率 (multiplier = 2.0)
// ============================================================
TEST(BendCalculator, Bend90_Multiplier2) {
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),
        gp_Pnt(200, 0, 0),
        gp_Pnt(200, 200, 0),
        40.0, 2.0); // R = 80

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->bendAngle,  PI / 2.0, kTol);
    EXPECT_NEAR(result->bendRadius, 80.0, kTol);
    verifyArcPoints(*result);
}

// ============================================================
// 7. 3D 斜面弯头 — 任意平面
// ============================================================
TEST(BendCalculator, Bend90_Arbitrary3D) {
    // d1 along (1,1,0)/√2, d2 along (0,0,1) — 90° bend in 3D
    double s = 100.0 / std::sqrt(2.0);
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),
        gp_Pnt(s, s, 0),
        gp_Pnt(s, s, 100),
        50.0, 1.5); // R = 75

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->bendAngle,  PI / 2.0, kTol);
    EXPECT_NEAR(result->bendRadius, 75.0, kTol);
    verifyArcPoints(*result);
}

// ============================================================
// 8. 退化: 接近 0° (直线) => nullopt
// ============================================================
TEST(BendCalculator, Degenerate_Straight) {
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),
        gp_Pnt(100, 0, 0),
        gp_Pnt(200, 0, 0),  // collinear
        50.0, 1.5);

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// 9. 退化: 接近 180° (U-turn) => nullopt
// ============================================================
TEST(BendCalculator, Degenerate_UTurn) {
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),
        gp_Pnt(100, 0, 0),
        gp_Pnt(0, 0, 0),  // folds back
        50.0, 1.5);

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// 10. 退化: 重合点 => nullopt
// ============================================================
TEST(BendCalculator, Degenerate_CoincidentPoints) {
    auto result = BendCalculator::calculateBend(
        gp_Pnt(100, 0, 0),
        gp_Pnt(100, 0, 0),  // same as prevPoint
        gp_Pnt(100, 100, 0),
        50.0, 1.5);

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// 11. N/F 在弯弧切线方向上 (切点验证)
// ============================================================
TEST(BendCalculator, TangentPointVerification) {
    // For a 90° bend, the tangent at N should be along d1,
    // and the tangent at F should be along d2.
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),
        gp_Pnt(100, 0, 0),
        gp_Pnt(100, 100, 0),
        50.0, 1.5);

    ASSERT_TRUE(result.has_value());

    // Vector from center to N should be perpendicular to d1 = (1,0,0)
    gp_Vec centerToN(result->arcCenter, result->nearPoint);
    gp_Vec d1(gp_Pnt(0, 0, 0), gp_Pnt(100, 0, 0));
    d1.Normalize();
    EXPECT_NEAR(std::abs(centerToN.Dot(d1)), 0.0, kTol);

    // Vector from center to F should be perpendicular to d2 = (0,1,0)
    gp_Vec centerToF(result->arcCenter, result->farPoint);
    gp_Vec d2(gp_Pnt(100, 0, 0), gp_Pnt(100, 100, 0));
    d2.Normalize();
    EXPECT_NEAR(std::abs(centerToF.Dot(d2)), 0.0, kTol);
}

// ============================================================
// 12. M 点在 N-F 弧的中间 (角度等分验证)
// ============================================================
TEST(BendCalculator, MidpointAngleBisection) {
    auto result = BendCalculator::calculateBend(
        gp_Pnt(0, 0, 0),
        gp_Pnt(100, 0, 0),
        gp_Pnt(100, 100, 0),
        50.0, 1.5);

    ASSERT_TRUE(result.has_value());

    gp_Vec cN(result->arcCenter, result->nearPoint);
    gp_Vec cM(result->arcCenter, result->midPoint);
    gp_Vec cF(result->arcCenter, result->farPoint);

    // Angle from center: N→M and M→F should be equal (both = θ/2)
    double angleNM = cN.Angle(cM);
    double angleMF = cM.Angle(cF);
    EXPECT_NEAR(angleNM, angleMF, kTol);
    EXPECT_NEAR(angleNM + angleMF, result->bendAngle, kTol);
}
