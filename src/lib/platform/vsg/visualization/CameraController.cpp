// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "visualization/CameraController.h"

#include <vsg/app/ViewMatrix.h>
#include <vsg/maths/transform.h>
#include <vsg/utils/ComputeBounds.h>

#include <cmath>
#include <stdexcept>

namespace visualization {

CameraController::CameraController(
    vsg::ref_ptr<vsg::Camera>    camera,
    vsg::ref_ptr<vsg::Trackball> trackball)
    : camera_(camera)
    , trackball_(trackball)
{
    if (!camera_)    throw std::invalid_argument("CameraController: camera is null");
    if (!trackball_) throw std::invalid_argument("CameraController: trackball is null");
}

vsg::ref_ptr<vsg::Trackball> CameraController::trackball() const { return trackball_; }
vsg::ref_ptr<vsg::Camera>    CameraController::camera()    const { return camera_; }

vsg::ref_ptr<vsg::LookAt> CameraController::lookAt() const {
    return camera_->viewMatrix.cast<vsg::LookAt>();
}

vsg::ref_ptr<vsg::LookAt> CameraController::computePresetLookAt(
    ViewPreset        preset,
    const vsg::dvec3& center,
    double            distance)
{
    vsg::dvec3 eye;
    vsg::dvec3 up;

    switch (preset) {
    case ViewPreset::Front:
        // 正视图：沿 -Y 方向看（相机在 center + Y*dist，向 -Y 看），Z 向上
        eye = center + vsg::dvec3(0.0, distance, 0.0);
        up  = vsg::dvec3(0.0, 0.0, 1.0);
        break;
    case ViewPreset::Right:
        // 右视图：沿 -X 方向看（相机在 center + X*dist），Z 向上
        eye = center + vsg::dvec3(distance, 0.0, 0.0);
        up  = vsg::dvec3(0.0, 0.0, 1.0);
        break;
    case ViewPreset::Top:
        // 俯视图：沿 -Z 方向看（相机在 center + Z*dist），Y 向前
        eye = center + vsg::dvec3(0.0, 0.0, distance);
        up  = vsg::dvec3(0.0, 1.0, 0.0);
        break;
    case ViewPreset::Isometric:
        // 轴测图：(-1,-1,-1) 方向，等距离
        {
            const double invSqrt3 = 1.0 / std::sqrt(3.0);
            eye = center + vsg::dvec3(-distance * invSqrt3,
                                      -distance * invSqrt3,
                                       distance * invSqrt3);
            up  = vsg::dvec3(0.0, 0.0, 1.0);
        }
        break;
    }

    return vsg::LookAt::create(eye, center, up);
}

void CameraController::setViewPreset(ViewPreset preset, double duration) {
    auto currentLookAt = lookAt();

    vsg::dvec3 center(0.0, 0.0, 0.0);
    double     distance = 1000.0; // 默认距离（mm）

    if (currentLookAt) {
        center   = currentLookAt->center;
        distance = vsg::length(currentLookAt->eye - currentLookAt->center);
        if (distance < 1e-6) distance = 1000.0;
    }

    auto targetLookAt = computePresetLookAt(preset, center, distance);
    trackball_->setViewpoint(targetLookAt, duration);
}

void CameraController::fitAll(vsg::ref_ptr<vsg::Node> sceneRoot) {
    if (!sceneRoot) return;

    vsg::ComputeBounds cb;
    sceneRoot->accept(cb);
    fitAll(cb.bounds);
}

void CameraController::fitAll(const vsg::dbox& bounds) {
    if (!bounds.valid()) return;

    // 计算包围盒中心和对角线半长
    vsg::dvec3 center = (bounds.min + bounds.max) * 0.5;
    double     radius = vsg::length(bounds.max - bounds.min) * 0.5;
    if (radius < 1e-6) radius = 1000.0;

    // 相机距离：使包围球在 60° FOV 的视野中完全可见（1.5 倍余量）
    double distance = radius / std::tan(vsg::radians(30.0)) * 1.5;

    auto currentLookAt = lookAt();
    vsg::dvec3 eyeDir(0.0, -1.0, 0.0); // 默认正视图方向
    if (currentLookAt) {
        vsg::dvec3 dir = currentLookAt->eye - currentLookAt->center;
        double len = vsg::length(dir);
        if (len > 1e-6) eyeDir = dir / len;
    }

    auto targetLookAt = vsg::LookAt::create(
        center + eyeDir * distance,
        center,
        currentLookAt ? currentLookAt->up : vsg::dvec3(0.0, 0.0, 1.0));

    trackball_->setViewpoint(targetLookAt, 0.3);
}

} // namespace visualization
