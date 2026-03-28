#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <string>

namespace engine {

/// 阀门几何生成器
/// 生成简化阀门几何：管段 + 膨大阀体 + 管段（外形 cut 内腔）
class ValveBuilder {
public:
    /// @param startPoint    阀门上游管口中心
    /// @param endPoint      阀门下游管口中心
    /// @param outerDiameter 连接管道外径 (mm)
    /// @param wallThickness 管壁厚度 (mm)
    /// @param valveType     阀门类型: "gate" / "ball" / "check"
    /// @return 阀门 TopoDS_Shape；输入无效时返回空 Shape
    static TopoDS_Shape build(
        const gp_Pnt& startPoint,
        const gp_Pnt& endPoint,
        double outerDiameter,
        double wallThickness,
        const std::string& valveType = "gate");
};

} // namespace engine
