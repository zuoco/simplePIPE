#include "visualization/PickHandler.h"

#include "visualization/SceneManager.h"

#include <vsg/utils/LineSegmentIntersector.h>

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

} // namespace visualization
