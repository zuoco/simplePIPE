#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

namespace engine {

/// 附件简化几何生成器
/// 提供法兰、支架等常见管道附件的简化几何表示
class AccessoryBuilder {
public:
    /// 构建法兰（平面圆盘）
    /// @param center         法兰中心点
    /// @param normal         法兰法向（轴线方向）
    /// @param pipeDiameter   连接管道外径 (mm)；法兰外径 = pipeDiameter * 1.5
    /// @param thickness      法兰厚度 (mm)
    /// @return 法兰 TopoDS_Shape
    static TopoDS_Shape buildFlange(
        const gp_Pnt& center,
        const gp_Dir& normal,
        double pipeDiameter,
        double thickness);

    /// 构建支架（矩形截面盒体）
    /// @param base   支架底部中心点
    /// @param top    支架顶部中心点
    /// @param width  截面边长 (mm)（正方形截面）
    /// @return 支架 TopoDS_Shape
    static TopoDS_Shape buildBracket(
        const gp_Pnt& base,
        const gp_Pnt& top,
        double width);
};

} // namespace engine
