// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "visualization/PickHandler.h"
#include "visualization/PipePointNode.h"
#include "visualization/SceneManager.h"
#include "visualization/SelectionHighlight.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/state/ViewportState.h>

namespace {

vsg::ref_ptr<vsg::Camera> makeTestCamera()
{
    auto perspective = vsg::Perspective::create(45.0, 800.0 / 600.0, 1.0, 5000.0);
    auto lookAt = vsg::LookAt::create(
        vsg::dvec3(0.0, -300.0, 0.0),
        vsg::dvec3(0.0, 0.0, 0.0),
        vsg::dvec3(0.0, 0.0, 1.0));
    auto viewport = vsg::ViewportState::create(0, 0, 800, 600);
    return vsg::Camera::create(perspective, lookAt, viewport);
}

} // namespace

TEST(PickHandler, PickHitReturnsUuid)
{
    visualization::SceneManager sceneManager;
    sceneManager.addNode("uuid-hit", visualization::createPipePointNode(0.0, 0.0, 0.0, 80.0f));

    visualization::PickHandler pickHandler(&sceneManager);
    auto camera = makeTestCamera();

    vsg::dvec3 hitPos{};
    auto hit = pickHandler.pick(*camera, sceneManager.root(), 400, 300, &hitPos);

    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(*hit, "uuid-hit");
    EXPECT_NEAR(hitPos.x, 0.0, 20.0);
    EXPECT_NEAR(hitPos.y, -40.0, 5.0);
}

TEST(PickHandler, PickMissReturnsNullopt)
{
    visualization::SceneManager sceneManager;
    sceneManager.addNode("uuid-hit", visualization::createPipePointNode(0.0, 0.0, 0.0, 80.0f));

    visualization::PickHandler pickHandler(&sceneManager);
    auto camera = makeTestCamera();

    auto miss = pickHandler.pick(*camera, sceneManager.root(), 0, 0);
    EXPECT_FALSE(miss.has_value());
}

TEST(PickHandler, LeftClickCallbackCanClearSelection)
{
    visualization::SceneManager sceneManager;
    sceneManager.addNode("uuid-hit", visualization::createPipePointNode(0.0, 0.0, 0.0, 80.0f));

    visualization::PickHandler pickHandler(&sceneManager);
    auto camera = makeTestCamera();

    std::optional<std::string> lastSelection;
    pickHandler.setSelectionCallback([&lastSelection](const std::optional<std::string>& selected) {
        lastSelection = selected;
    });

    pickHandler.handleLeftClick(*camera, sceneManager.root(), 400, 300);
    ASSERT_TRUE(lastSelection.has_value());
    EXPECT_EQ(*lastSelection, "uuid-hit");

    pickHandler.handleLeftClick(*camera, sceneManager.root(), 0, 0);
    EXPECT_FALSE(lastSelection.has_value());
}

TEST(PickHandler, RightClickCallbackReturnsWorldPosition)
{
    visualization::SceneManager sceneManager;
    sceneManager.addNode("uuid-hit", visualization::createPipePointNode(0.0, 0.0, 0.0, 80.0f));

    visualization::PickHandler pickHandler(&sceneManager);
    auto camera = makeTestCamera();

    vsg::dvec3 contextPos{};
    std::optional<std::string> contextId;
    pickHandler.setContextMenuCallback(
        [&contextPos, &contextId](const vsg::dvec3& worldPos, const std::optional<std::string>& id) {
            contextPos = worldPos;
            contextId = id;
        });

    pickHandler.handleRightClick(*camera, sceneManager.root(), 400, 300);

    ASSERT_TRUE(contextId.has_value());
    EXPECT_EQ(*contextId, "uuid-hit");
    EXPECT_NEAR(contextPos.y, -40.0, 5.0);
}

TEST(SelectionHighlight, HighlightAndClear)
{
    visualization::SceneManager sceneManager;
    sceneManager.addNode("uuid-a", visualization::createPipePointNode(0.0, 0.0, 0.0, 40.0f));

    visualization::SelectionHighlight highlight(&sceneManager);

    ASSERT_TRUE(highlight.setSelected("uuid-a"));
    EXPECT_EQ(highlight.selectedUuid(), "uuid-a");

    vsg::vec4 color{};
    auto node = sceneManager.findNode("uuid-a");
    ASSERT_NE(node, nullptr);
    ASSERT_TRUE(node->getValue(visualization::SelectionHighlight::kHighlightColorKey, color));

    EXPECT_NEAR(color.r, 0.0f, 1e-6f);
    EXPECT_NEAR(color.g, 120.0f / 255.0f, 1e-6f);
    EXPECT_NEAR(color.b, 212.0f / 255.0f, 1e-6f);
    EXPECT_NEAR(color.a, 1.0f, 1e-6f);

    EXPECT_TRUE(highlight.clear());
    EXPECT_TRUE(highlight.selectedUuid().empty());

    vsg::vec4 afterClear{};
    EXPECT_FALSE(node->getValue(visualization::SelectionHighlight::kHighlightColorKey, afterClear));
}

TEST(SelectionHighlight, RestoreOriginalColor)
{
    visualization::SceneManager sceneManager;
    sceneManager.addNode("uuid-b", visualization::createPipePointNode(0.0, 0.0, 0.0, 40.0f));

    auto node = sceneManager.findNode("uuid-b");
    ASSERT_NE(node, nullptr);
    node->setValue(visualization::SelectionHighlight::kHighlightColorKey, vsg::vec4(0.1f, 0.2f, 0.3f, 1.0f));

    visualization::SelectionHighlight highlight(&sceneManager);
    ASSERT_TRUE(highlight.setSelected("uuid-b"));
    ASSERT_TRUE(highlight.clear());

    vsg::vec4 restored{};
    ASSERT_TRUE(node->getValue(visualization::SelectionHighlight::kHighlightColorKey, restored));
    EXPECT_NEAR(restored.r, 0.1f, 1e-6f);
    EXPECT_NEAR(restored.g, 0.2f, 1e-6f);
    EXPECT_NEAR(restored.b, 0.3f, 1e-6f);
    EXPECT_NEAR(restored.a, 1.0f, 1e-6f);
}

TEST(SelectionHighlight, UnknownUuidReturnsFalse)
{
    visualization::SceneManager sceneManager;
    visualization::SelectionHighlight highlight(&sceneManager);

    EXPECT_FALSE(highlight.setSelected("missing"));
    EXPECT_TRUE(highlight.selectedUuid().empty());
}
