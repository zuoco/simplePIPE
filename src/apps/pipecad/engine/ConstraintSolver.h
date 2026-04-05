// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/Route.h"
#include "model/Segment.h"
#include "model/PipePoint.h"
#include "foundation/Types.h"

#include <string>
#include <vector>

namespace engine {

/// 约束检查结果
struct ConstraintError {
    std::string pointId;  ///< 问题所在管点的 UUID 字符串
    std::string message;  ///< 错误描述
};

/// 管道约束求解器：检查端口口径匹配和弯头角度合法性。
class ConstraintSolver {
public:
    ConstraintSolver() = default;

    /// 检查段内相邻管点的口径一致性。
    /// 规则: 若相邻两点均非 Reducer，且 OD 差值 > 1e-6，则报告错误。
    std::vector<ConstraintError> checkDiameterConsistency(
        const model::Segment& seg) const;

    /// 检查段内所有 Bend 管点的角度合法性 (0° < bendAngle < 180°)。
    /// bendAngle 为管道折转角，等于 π 减三点夹角。
    /// 夹角接近 0° (重合/折回) 或接近 π (三点共线) 均视为无效。
    std::vector<ConstraintError> checkBendAngles(
        const model::Segment& seg) const;

    /// 对整条管路运行所有约束检查
    std::vector<ConstraintError> checkAll(const model::Route& route) const;
};

} // namespace engine
