// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "visualization/SelectionHighlight.h"

#include "visualization/SceneManager.h"

#include <vsg/core/Value.h>

namespace visualization {

SelectionHighlight::SelectionHighlight(SceneManager* sceneManager)
    : sceneManager_{sceneManager}
{
}

void SelectionHighlight::setSceneManager(SceneManager* sceneManager)
{
    if (sceneManager_ == sceneManager) {
        return;
    }

    clear();
    sceneManager_ = sceneManager;
}

SceneManager* SelectionHighlight::sceneManager() const
{
    return sceneManager_;
}

bool SelectionHighlight::setSelected(const std::string& uuid)
{
    if (!sceneManager_) {
        return false;
    }

    if (uuid == selectedUuid_) {
        return true;
    }

    clear();

    if (uuid.empty()) {
        return true;
    }

    auto node = sceneManager_->findNode(uuid);
    if (!node) {
        return false;
    }

    if (!applyToNode(node.get())) {
        return false;
    }

    selectedUuid_ = uuid;
    return true;
}

bool SelectionHighlight::clear()
{
    if (selectedUuid_.empty() || !sceneManager_) {
        selectedUuid_.clear();
        hasOriginalColor_ = false;
        return false;
    }

    auto node = sceneManager_->findNode(selectedUuid_);
    if (node) {
        restoreNode(node.get());
    }

    selectedUuid_.clear();
    hasOriginalColor_ = false;
    return true;
}

const std::string& SelectionHighlight::selectedUuid() const
{
    return selectedUuid_;
}

void SelectionHighlight::setHighlightColor(const vsg::vec4& color)
{
    highlightColor_ = color;

    if (!selectedUuid_.empty() && sceneManager_) {
        auto node = sceneManager_->findNode(selectedUuid_);
        if (node) {
            node->setValue(kHighlightColorKey, highlightColor_);
        }
    }
}

const vsg::vec4& SelectionHighlight::highlightColor() const
{
    return highlightColor_;
}

bool SelectionHighlight::applyToNode(vsg::Node* object)
{
    if (!object) {
        return false;
    }

    hasOriginalColor_ = object->getValue(kHighlightColorKey, originalColor_);
    object->setValue(kHighlightColorKey, highlightColor_);
    return true;
}

bool SelectionHighlight::restoreNode(vsg::Node* object)
{
    if (!object) {
        return false;
    }

    if (hasOriginalColor_) {
        object->setValue(kHighlightColorKey, originalColor_);
    } else {
        object->removeObject(kHighlightColorKey);
    }

    return true;
}

} // namespace visualization
