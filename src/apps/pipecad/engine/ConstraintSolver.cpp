// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/ConstraintSolver.h"
#include "model/PipeSpec.h"
#include "foundation/Math.h"

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <algorithm>
#include <cmath>
#include <sstream>

namespace engine {

namespace {

/// 计算三点 A-B-C 在 B 处的夹角（弧度），取值 [0, π]。
double angleBetween(const gp_Pnt& a, const gp_Pnt& b, const gp_Pnt& c)
{
    gp_Vec v1(b, a);  // B→A (incoming reverse)
    gp_Vec v2(b, c);  // B→C (outgoing)
    double m1 = v1.Magnitude();
    double m2 = v2.Magnitude();
    if (m1 < 1e-12 || m2 < 1e-12) return 0.0;
    v1.Normalize();
    v2.Normalize();
    double dot = v1.Dot(v2);
    dot = std::max(-1.0, std::min(1.0, dot));
    return std::acos(dot);
}

} // namespace

std::vector<ConstraintError> ConstraintSolver::checkDiameterConsistency(
    const model::Segment& seg) const
{
    std::vector<ConstraintError> errors;
    const auto& pts = seg.points();
    for (std::size_t i = 0; i + 1 < pts.size(); ++i) {
        const auto& p1 = pts[i];
        const auto& p2 = pts[i + 1];
        // Reducer handles OD transitions by design — skip adjacent pairs containing it
        if (p1->type() == model::PipePointType::Reducer ||
            p2->type() == model::PipePointType::Reducer) {
            continue;
        }
        auto spec1 = p1->pipeSpec();
        auto spec2 = p2->pipeSpec();
        if (!spec1 || !spec2) continue;
        if (std::abs(spec1->od() - spec2->od()) > 1e-6) {
            std::ostringstream msg;
            msg << "OD mismatch between points[" << i << "] (OD=" << spec1->od()
                << ") and points[" << (i + 1) << "] (OD=" << spec2->od()
                << "); a Reducer is required.";
            errors.push_back({p2->id().toString(), msg.str()});
        }
    }
    return errors;
}

std::vector<ConstraintError> ConstraintSolver::checkBendAngles(
    const model::Segment& seg) const
{
    std::vector<ConstraintError> errors;
    const auto& pts = seg.points();
    for (std::size_t i = 0; i < pts.size(); ++i) {
        if (pts[i]->type() != model::PipePointType::Bend) continue;

        if (i == 0 || i + 1 >= pts.size()) {
            errors.push_back({pts[i]->id().toString(),
                "Bend at index " + std::to_string(i) +
                " lacks a predecessor or successor; angle cannot be computed."});
            continue;
        }

        const gp_Pnt& prev = pts[i - 1]->position();
        const gp_Pnt& bend = pts[i]->position();
        const gp_Pnt& next = pts[i + 1]->position();

        // theta: angle at the bend point between incoming and outgoing arms
        // bendAngle = PI - theta  → valid range means 0 < theta < PI
        double theta = angleBetween(prev, bend, next);

        if (theta < 1e-6) {
            errors.push_back({pts[i]->id().toString(),
                "Bend at index " + std::to_string(i) +
                " has zero included angle (coincident or fold-back points)."});
        } else if (theta > foundation::math::PI - 1e-6) {
            errors.push_back({pts[i]->id().toString(),
                "Bend at index " + std::to_string(i) +
                " has 180° included angle (collinear points; no effective bend)."});
        }
    }
    return errors;
}

std::vector<ConstraintError> ConstraintSolver::checkAll(const model::Route& route) const
{
    std::vector<ConstraintError> errors;
    for (const auto& seg : route.segments()) {
        auto diamErrs = checkDiameterConsistency(*seg);
        auto bendErrs = checkBendAngles(*seg);
        errors.insert(errors.end(), diamErrs.begin(), diamErrs.end());
        errors.insert(errors.end(), bendErrs.begin(), bendErrs.end());
    }
    return errors;
}

} // namespace engine
