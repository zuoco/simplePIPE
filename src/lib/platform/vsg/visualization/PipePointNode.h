// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vsg/maths/vec4.h>
#include <vsg/nodes/MatrixTransform.h>

namespace visualization {

/// @brief 创建管点的 VSG 场景节点（渲染为小正方体）
///
/// 返回一个 MatrixTransform 节点，已定位到指定世界坐标并缩放为 size mm 的立方体。
/// 内部子节点是一个手动构建的 VertexIndexDraw（不依赖 Vulkan context）。
/// 着色器 Pipeline 由外层 StateGroup 提供（SceneManager 负责设置）。
///
/// @param x, y, z   世界坐标（mm）
/// @param size      立方体边长（mm），默认 20 mm
/// @param color     RGBA 颜色（预留，用于后续材质传入）
/// @return          已定位的 MatrixTransform 节点
vsg::ref_ptr<vsg::MatrixTransform> createPipePointNode(
    double x, double y, double z,
    float size = 20.0f,
    const vsg::vec4& color = {0.9f, 0.5f, 0.1f, 1.0f});

} // namespace visualization
