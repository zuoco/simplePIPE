#pragma once

#include <vsg/maths/vec4.h>
#include <vsg/nodes/Node.h>

#include <string>

namespace visualization {

class SceneManager;

class SelectionHighlight {
public:
    static constexpr const char* kHighlightColorKey = "pipecad.highlightColor";

    explicit SelectionHighlight(SceneManager* sceneManager = nullptr);

    void setSceneManager(SceneManager* sceneManager);
    SceneManager* sceneManager() const;

    bool setSelected(const std::string& uuid);
    bool clear();

    const std::string& selectedUuid() const;

    void setHighlightColor(const vsg::vec4& color);
    const vsg::vec4& highlightColor() const;

private:
    bool applyToNode(vsg::Node* node);
    bool restoreNode(vsg::Node* node);

    SceneManager* sceneManager_ = nullptr;
    std::string selectedUuid_;
    vsg::vec4 highlightColor_{0.0f, 120.0f / 255.0f, 212.0f / 255.0f, 1.0f};

    bool hasOriginalColor_ = false;
    vsg::vec4 originalColor_{};
};

} // namespace visualization
