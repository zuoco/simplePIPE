// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/BendCalculator.h"

#include <gp_Vec.hxx>
#include <cmath>

namespace engine {

std::optional<BendResult> BendCalculator::calculateBend(
    const gp_Pnt& prevPoint,
    const gp_Pnt& intersectPoint,
    const gp_Pnt& nextPoint,
    double outerDiameter,
    double bendMultiplier)
{
    constexpr double kMinAngle  = 1e-6;  // rad — below this, treat as straight
    constexpr double kMaxAngle  = 3.14159265358979323846 - 1e-6; // near π → U-turn
    constexpr double kMinLength = 1e-9;  // mm — coincident point threshold

    // Incoming and outgoing direction vectors
    gp_Vec v1(prevPoint, intersectPoint);  // A05 → A06
    gp_Vec v2(intersectPoint, nextPoint);  // A06 → A07

    double len1 = v1.Magnitude();
    double len2 = v2.Magnitude();
    if (len1 < kMinLength || len2 < kMinLength)
        return std::nullopt;

    v1.Normalize();
    v2.Normalize();

    // Bend angle = angle between incoming and outgoing directions
    // θ = 0 → straight (no bend), θ = π → U-turn (degenerate)
    double cosTheta = v1.Dot(v2);
    if (cosTheta > 1.0)  cosTheta = 1.0;
    if (cosTheta < -1.0) cosTheta = -1.0;
    double theta = std::acos(cosTheta);

    if (theta < kMinAngle || theta > kMaxAngle)
        return std::nullopt;

    // Bend radius and tangent length
    double R = outerDiameter * bendMultiplier;
    double tanLen = R * std::tan(theta / 2.0);

    // Near point: back along incoming direction from intersection
    gp_Pnt N = intersectPoint.Translated(v1 * (-tanLen));
    // Far point: forward along outgoing direction from intersection
    gp_Pnt F = intersectPoint.Translated(v2 * tanLen);

    // Arc center: perpendicular to v1 at N, in the bend plane
    gp_Vec normal = v1.Crossed(v2);
    normal.Normalize();

    gp_Vec perpToV1 = normal.Crossed(v1);
    perpToV1.Normalize();

    gp_Pnt C = N.Translated(perpToV1 * R);

    // Midpoint: on the arc, in the direction from center toward intersection point
    gp_Vec centerToIntersect(C, intersectPoint);
    centerToIntersect.Normalize();
    gp_Pnt M = C.Translated(centerToIntersect * R);

    return BendResult{N, M, F, C, theta, R};
}

} // namespace engine
