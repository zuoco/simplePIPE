// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vsg/app/Camera.h>
#include <vsg/app/Trackball.h>
#include <vsg/nodes/Group.h>
#include <vsg/maths/box.h>

namespace visualization {

/// 视图预设枚举
enum class ViewPreset {
    Front,      ///< 正视图：沿 +Y 方向看，Z 向上
    Right,      ///< 右视图：沿 -X 方向看，Z 向上
    Top,        ///< 俯视图：沿 -Z 方向看，Y 向前
    Isometric   ///< 轴测图：(-1,-1,-1) 方向看原点
};

/// @brief 相机控制器
///
/// 封装 VSG Trackball，提供工程 CAD 软件常见的相机交互模式：
///   - 滚轮缩放
///   - 中键拖动平移
///   - Ctrl + 左键拖动旋转
///
/// 同时提供视图预设切换和 Fit All 功能。
///
/// 典型用法：
/// @code
///   auto camera    = vsg::Camera::create(perspective, lookAt, viewport);
///   auto trackball = vsg::Trackball::create(camera);
///   CameraController ctrl(camera, trackball);
///   ctrl.setViewPreset(ViewPreset::Isometric);
///   viewer->addEventHandler(ctrl.trackball());
/// @endcode
class CameraController {
public:
    /// @param camera    VSG 相机对象（持有 LookAt 和 Perspective）
    /// @param trackball VSG Trackball 事件处理器
    CameraController(
        vsg::ref_ptr<vsg::Camera>    camera,
        vsg::ref_ptr<vsg::Trackball> trackball);

    /// 访问底层 Trackball（注册到 Viewer 的事件处理链）
    vsg::ref_ptr<vsg::Trackball> trackball() const;

    /// 访问相机
    vsg::ref_ptr<vsg::Camera> camera() const;

    /// 切换视图预设，以 duration 秒平滑过渡（默认 0.2s）
    void setViewPreset(ViewPreset preset, double duration = 0.2);

    /// 调整相机使场景中所有对象可见
    /// @param sceneRoot 场景根节点，用于计算包围盒
    void fitAll(vsg::ref_ptr<vsg::Node> sceneRoot);

    /// 从包围盒直接设置 Fit All（适合单元测试）
    void fitAll(const vsg::dbox& bounds);

    /// 获取当前 LookAt（只读快照）
    vsg::ref_ptr<vsg::LookAt> lookAt() const;

    /// 计算给定预设的目标 LookAt
    /// @param preset    视图预设
    /// @param center    场景中心点
    /// @param distance  相机到目标的距离
    static vsg::ref_ptr<vsg::LookAt> computePresetLookAt(
        ViewPreset          preset,
        const vsg::dvec3&   center,
        double              distance);

private:
    vsg::ref_ptr<vsg::Camera>    camera_;
    vsg::ref_ptr<vsg::Trackball> trackball_;
};

} // namespace visualization
