// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/VertexIndexDraw.h>

namespace visualization {

/// @brief 创建管件组件的 VSG 场景子图
///
/// 场景层次：MatrixTransform → StateGroup → VertexIndexDraw
/// StateGroup 的 stateCommands 为空，由外层 SceneManager 在渲染准备阶段填充 Pipeline。
///
/// @param vid     由 toVsgGeometry() 创建的几何节点（可为 nullptr，此时返回仅含变换的节点）
/// @param matrix  世界变换矩阵（默认单位矩阵）
/// @return        MatrixTransform 根节点（包含 StateGroup 和几何节点）
vsg::ref_ptr<vsg::MatrixTransform> createComponentNode(
    vsg::ref_ptr<vsg::VertexIndexDraw> vid,
    const vsg::dmat4& matrix = vsg::dmat4{});

} // namespace visualization
