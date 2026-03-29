#pragma once

#include <vsg/app/Camera.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Node.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace visualization {

class SceneManager;

/// Box selection mode: Window (fully inside) vs Crossing (partially inside)
enum class BoxSelectMode {
    Window,   ///< Left-to-right: objects must be fully inside the rectangle
    Crossing  ///< Right-to-left: objects partially inside are selected
};

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

    /// Perform box selection. Returns UUIDs of nodes whose screen projections
    /// intersect (Crossing) or are fully contained (Window) in the given rectangle.
    /// Rectangle corners are (x0,y0) top-left and (x1,y1) bottom-right in screen coords.
    std::vector<std::string> boxSelect(const vsg::Camera& camera,
                                       vsg::ref_ptr<vsg::Node> sceneRoot,
                                       int32_t x0, int32_t y0,
                                       int32_t x1, int32_t y1,
                                       BoxSelectMode mode) const;

    /// Determine box selection mode from drag direction
    static BoxSelectMode modeFromDrag(int32_t startX, int32_t endX);

private:
    const SceneManager* sceneManager_ = nullptr;
    SelectionCallback selectionCallback_;
    ContextMenuCallback contextMenuCallback_;
};

} // namespace visualization
