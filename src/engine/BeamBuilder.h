// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <string>

namespace engine {

/// 型钢梁几何生成器
/// 将截面轮廓沿路径方向挤出，生成矩形截面或 H 型钢实体
class BeamBuilder {
public:
    /// 截面类型
    enum class SectionType {
        Rectangular, ///< 实心矩形截面
        HSection     ///< H 型钢截面（工字钢）
    };

    /// @param startPoint  梁起点
    /// @param endPoint    梁终点
    /// @param sectionType 截面类型
    /// @param width       截面宽度 (mm)
    /// @param height      截面高度 (mm)
    /// @return 梁实体 TopoDS_Shape；输入无效时返回空 Shape
    static TopoDS_Shape build(
        const gp_Pnt& startPoint,
        const gp_Pnt& endPoint,
        SectionType sectionType,
        double width,
        double height);
};

} // namespace engine
