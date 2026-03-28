#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

namespace engine {

/// 柔性接头（波纹管）几何生成器
/// 生成多段交替锥面组成的手风琴状波纹管几何，内腔用圆柱镂空
class FlexJointBuilder {
public:
    /// @param startPoint    接头上游端中心
    /// @param endPoint      接头下游端中心
    /// @param outerDiameter 连接管道外径 (mm)
    /// @param wallThickness 管壁厚度 (mm)
    /// @param segmentCount  波纹数量（每个波纹 = 一扩一缩两段锥体）
    /// @return 波纹管 TopoDS_Shape；输入无效时返回空 Shape
    static TopoDS_Shape build(
        const gp_Pnt& startPoint,
        const gp_Pnt& endPoint,
        double outerDiameter,
        double wallThickness,
        int segmentCount = 3);
};

} // namespace engine
