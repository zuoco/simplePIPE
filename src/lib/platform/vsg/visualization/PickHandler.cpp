// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "visualization/PickHandler.h"

#include "visualization/SceneManager.h"

#include <vsg/utils/LineSegmentIntersector.h>
#include <vsg/utils/ComputeBounds.h>

#include <algorithm>
#include <cmath>

namespace visualization {

PickHandler::PickHandler(const SceneManager* sceneManager)
    : sceneManager_{sceneManager}
{
}

void PickHandler::setSceneManager(const SceneManager* sceneManager)
{
    sceneManager_ = sceneManager;
}

const SceneManager* PickHandler::sceneManager() const
{
    return sceneManager_;
}

void PickHandler::setSelectionCallback(SelectionCallback callback)
{
    selectionCallback_ = std::move(callback);
}

void PickHandler::setContextMenuCallback(ContextMenuCallback callback)
{
    contextMenuCallback_ = std::move(callback);
}

std::optional<std::string> PickHandler::pick(const vsg::Camera& camera,
                                             vsg::ref_ptr<vsg::Node> sceneRoot,
                                             int32_t x,
                                             int32_t y,
                                             vsg::dvec3* worldPosition) const
{
    if (!sceneRoot || !sceneManager_) {
        return std::nullopt;
    }

    auto intersector = vsg::LineSegmentIntersector::create(camera, x, y);
    sceneRoot->accept(*intersector);

    vsg::ref_ptr<vsg::LineSegmentIntersector::Intersection> nearest;
    for (const auto& hit : intersector->intersections) {
        if (!hit || !(*hit)) {
            continue;
        }

        if (!nearest || hit->ratio < nearest->ratio) {
            nearest = hit;
        }
    }

    if (!nearest) {
        return std::nullopt;
    }

    if (worldPosition) {
        *worldPosition = nearest->worldIntersection;
    }

    for (auto it = nearest->nodePath.rbegin(); it != nearest->nodePath.rend(); ++it) {
        const std::string uuid = sceneManager_->findUuidByNode(*it);
        if (!uuid.empty()) {
            return uuid;
        }
    }

    return std::nullopt;
}

void PickHandler::handleLeftClick(const vsg::Camera& camera,
                                  vsg::ref_ptr<vsg::Node> sceneRoot,
                                  int32_t x,
                                  int32_t y) const
{
    if (!selectionCallback_) {
        return;
    }

    selectionCallback_(pick(camera, sceneRoot, x, y));
}

void PickHandler::handleRightClick(const vsg::Camera& camera,
                                   vsg::ref_ptr<vsg::Node> sceneRoot,
                                   int32_t x,
                                   int32_t y) const
{
    if (!contextMenuCallback_) {
        return;
    }

    vsg::dvec3 worldPos{0.0, 0.0, 0.0};
    auto uuid = pick(camera, sceneRoot, x, y, &worldPos);
    contextMenuCallback_(worldPos, uuid);
}

// Helper: project a world point to screen pixel coordinates
static bool projectToScreen(const vsg::dmat4& viewMatrix,
                            const vsg::dmat4& projMatrix,
                            int32_t vpWidth, int32_t vpHeight,
                            const vsg::dvec3& worldPt,
                            double& sx, double& sy)
{
    vsg::dvec4 clip = projMatrix * (viewMatrix * vsg::dvec4(worldPt.x, worldPt.y, worldPt.z, 1.0));
    if (std::abs(clip.w) < 1e-12) {
        return false;
    }
    double ndcX = clip.x / clip.w;
    double ndcY = clip.y / clip.w;
    sx = (ndcX * 0.5 + 0.5) * vpWidth;
    sy = (1.0 - (ndcY * 0.5 + 0.5)) * vpHeight;  // flip Y for screen coords
    return true;
}

std::vector<std::string> PickHandler::boxSelect(const vsg::Camera& camera,
                                                 vsg::ref_ptr<vsg::Node> sceneRoot,
                                                 int32_t x0, int32_t y0,
                                                 int32_t x1, int32_t y1,
                                                 BoxSelectMode mode) const
{
    std::vector<std::string> result;
    if (!sceneManager_ || !sceneRoot) {
        return result;
    }

    // Normalize rectangle
    int32_t minX = std::min(x0, x1);
    int32_t maxX = std::max(x0, x1);
    int32_t minY = std::min(y0, y1);
    int32_t maxY = std::max(y0, y1);

    // Get camera matrices
    auto viewMatrix = camera.viewMatrix->transform();
    auto projMatrix = camera.projectionMatrix->transform();

    auto vpState = camera.viewportState;
    int32_t vpWidth = 800;
    int32_t vpHeight = 600;
    if (vpState && !vpState->viewports.empty()) {
        vpWidth = static_cast<int32_t>(vpState->viewports[0].width);
        vpHeight = static_cast<int32_t>(vpState->viewports[0].height);
    }

    auto allUuids = sceneManager_->allUuids();
    for (const auto& uuid : allUuids) {
        auto node = sceneManager_->findNode(uuid);
        if (!node) {
            continue;
        }

        // Compute the bounding box of this node
        vsg::ComputeBounds computeBounds;
        node->accept(computeBounds);
        auto& bounds = computeBounds.bounds;

        if (!bounds.valid()) {
            continue;
        }

        // Generate 8 corners of the bounding box
        vsg::dvec3 corners[8] = {
            {bounds.min.x, bounds.min.y, bounds.min.z},
            {bounds.max.x, bounds.min.y, bounds.min.z},
            {bounds.min.x, bounds.max.y, bounds.min.z},
            {bounds.max.x, bounds.max.y, bounds.min.z},
            {bounds.min.x, bounds.min.y, bounds.max.z},
            {bounds.max.x, bounds.min.y, bounds.max.z},
            {bounds.min.x, bounds.max.y, bounds.max.z},
            {bounds.max.x, bounds.max.y, bounds.max.z},
        };

        // Project corners to screen and compute screen bounding rect
        double scrMinX = 1e9, scrMaxX = -1e9;
        double scrMinY = 1e9, scrMaxY = -1e9;
        bool anyProjected = false;

        for (const auto& corner : corners) {
            double sx, sy;
            if (projectToScreen(viewMatrix, projMatrix, vpWidth, vpHeight, corner, sx, sy)) {
                scrMinX = std::min(scrMinX, sx);
                scrMaxX = std::max(scrMaxX, sx);
                scrMinY = std::min(scrMinY, sy);
                scrMaxY = std::max(scrMaxY, sy);
                anyProjected = true;
            }
        }

        if (!anyProjected) {
            continue;
        }

        if (mode == BoxSelectMode::Window) {
            // Fully inside: screen bounding rect must be entirely within selection rect
            if (scrMinX >= minX && scrMaxX <= maxX &&
                scrMinY >= minY && scrMaxY <= maxY) {
                result.push_back(uuid);
            }
        } else {
            // Crossing: any overlap between screen bounding rect and selection rect
            if (scrMaxX >= minX && scrMinX <= maxX &&
                scrMaxY >= minY && scrMinY <= maxY) {
                result.push_back(uuid);
            }
        }
    }

    return result;
}

BoxSelectMode PickHandler::modeFromDrag(int32_t startX, int32_t endX)
{
    return (endX >= startX) ? BoxSelectMode::Window : BoxSelectMode::Crossing;
}

} // namespace visualization
