#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

namespace engine {

/// 三通管件几何生成器
/// 主管 + 支管 fuse 结果
class TeeBuilder {
public:
    /// @param mainStart   主管起点
    /// @param mainEnd     主管终点
    /// @param branchPoint 支管连接点(在主管中心线上)
    /// @param branchEnd   支管终点
    /// @param mainOD      主管外径 (mm)
    /// @param branchOD    支管外径 (mm)
    /// @param wallThickness 壁厚 (mm)
    /// @return 三通管件 TopoDS_Shape
    static TopoDS_Shape build(
        const gp_Pnt& mainStart,
        const gp_Pnt& mainEnd,
        const gp_Pnt& branchPoint,
        const gp_Pnt& branchEnd,
        double mainOD,
        double branchOD,
        double wallThickness);
};

} // namespace engine
