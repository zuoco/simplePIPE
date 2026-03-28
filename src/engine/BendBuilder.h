#pragma once

#include "engine/BendCalculator.h"
#include <TopoDS_Shape.hxx>

namespace engine {

/// 弯头 BRep 几何生成器
/// 使用 BendResult 参数生成弯管壳(圆环截段的壳体)
class BendBuilder {
public:
    /// @param bendResult  BendCalculator 的计算结果
    /// @param outerDiameter 外径 (mm)
    /// @param wallThickness 壁厚 (mm)
    /// @return 弯管壳 TopoDS_Shape
    static TopoDS_Shape build(
        const BendResult& bendResult,
        double outerDiameter,
        double wallThickness);
};

} // namespace engine
