#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

namespace engine {

/// 异径管(锥壳)几何生成器
/// 从 startPoint (大端) 到 endPoint (小端), 外径渐变
class ReducerBuilder {
public:
    /// @param startPoint    大端中心点
    /// @param endPoint      小端中心点
    /// @param startOD       大端外径 (mm)
    /// @param endOD         小端外径 (mm)
    /// @param wallThickness 壁厚 (mm), 两端一致
    /// @return 锥壳 TopoDS_Shape
    static TopoDS_Shape build(
        const gp_Pnt& startPoint,
        const gp_Pnt& endPoint,
        double startOD,
        double endOD,
        double wallThickness);
};

} // namespace engine
