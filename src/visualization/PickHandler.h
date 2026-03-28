#pragma once

#include <vsg/app/Camera.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Node.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace visualization {

class SceneManager;

class PickHandler {
public:
    using SelectionCallback = std::function<void(const std::optional<std::string>&)>;
    using ContextMenuCallback = std::function<void(const vsg::dvec3&, const std::optional<std::string>&)>;

    explicit PickHandler(const SceneManager* sceneManager = nullptr);

    void setSceneManager(const SceneManager* sceneManager);
    const SceneManager* sceneManager() const;

    void setSelectionCallback(SelectionCallback callback);
    void setContextMenuCallback(ContextMenuCallback callback);

    std::optional<std::string> pick(const vsg::Camera& camera,
                                    vsg::ref_ptr<vsg::Node> sceneRoot,
                                    int32_t x,
                                    int32_t y,
                                    vsg::dvec3* worldPosition = nullptr) const;

    void handleLeftClick(const vsg::Camera& camera,
                         vsg::ref_ptr<vsg::Node> sceneRoot,
                         int32_t x,
                         int32_t y) const;

    void handleRightClick(const vsg::Camera& camera,
                          vsg::ref_ptr<vsg::Node> sceneRoot,
                          int32_t x,
                          int32_t y) const;

private:
    const SceneManager* sceneManager_ = nullptr;
    SelectionCallback selectionCallback_;
    ContextMenuCallback contextMenuCallback_;
};

} // namespace visualization
