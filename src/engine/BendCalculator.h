#pragma once

#include <gp_Pnt.hxx>
#include <optional>

namespace engine {

/// 弯头计算结果
struct BendResult {
    gp_Pnt nearPoint;   ///< N点 — 弯弧起点(靠近 prevPoint)
    gp_Pnt midPoint;    ///< M点 — 弯弧中点
    gp_Pnt farPoint;    ///< F点 — 弯弧终点(靠近 nextPoint)
    gp_Pnt arcCenter;   ///< 弯弧圆心
    double bendAngle;   ///< 弯曲角度 (rad), 范围 (0, π)
    double bendRadius;  ///< 弯曲半径 (mm) = OD × multiplier
};

/// 弯头几何计算器 — 管道系统的核心算法
///
/// 给定三个管点 (A05→A06→A07)，计算 A06 处弯头的几何参数。
/// A06 是两管段中心线延长交点（虚拟点），弯弧连接 N 和 F。
///
/// 算法:
///   R = OD × bendMultiplier
///   θ = angle(d1, d2), 其中 d1 = A05→A06, d2 = A06→A07
///   N = A06 − d1 × R × tan(θ/2)
///   F = A06 + d2 × R × tan(θ/2)
///   M = 弧中点
class BendCalculator {
public:
    /// 计算弯头几何。
    /// @return 弯头结果; θ ≈ 0 (直线) 或 θ ≈ π (U-turn) 或输入点重合时返回 nullopt
    static std::optional<BendResult> calculateBend(
        const gp_Pnt& prevPoint,       // A05
        const gp_Pnt& intersectPoint,  // A06 (交点)
        const gp_Pnt& nextPoint,       // A07
        double outerDiameter,          // OD (mm)
        double bendMultiplier          // 1.5 | 2.0 | 5.0
    );
};

} // namespace engine
