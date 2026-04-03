// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "visualization/CameraController.h"
#include "visualization/SceneFurniture.h"
#include "visualization/SceneManager.h"

#include <vsg/app/Camera.h>
#include <vsg/app/Trackball.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/maths/box.h>

// ══════════════════════════════════════════════════════════════════════════════
// 辅助函数
// ══════════════════════════════════════════════════════════════════════════════
namespace {

/// 创建一个最小化 Camera + Trackball 组合用于测试
std::pair<vsg::ref_ptr<vsg::Camera>, vsg::ref_ptr<vsg::Trackball>> makeCamera() {
    auto perspective = vsg::Perspective::create(60.0, 1.0, 1.0, 100000.0);
    auto lookAt      = vsg::LookAt::create(
        vsg::dvec3(0.0, -1000.0, 500.0),  // eye
        vsg::dvec3(0.0,  0.0,   0.0),     // center
        vsg::dvec3(0.0,  0.0,   1.0));    // up

    auto camera    = vsg::Camera::create(perspective, lookAt);
    auto trackball = vsg::Trackball::create(camera);
    return {camera, trackball};
}

} // anonymous namespace

// ══════════════════════════════════════════════════════════════════════════════
// T13: CameraController
// ══════════════════════════════════════════════════════════════════════════════

/// 构造函数正常，不抛异常
TEST(CameraController, ConstructOk) {
    auto [camera, trackball] = makeCamera();
    EXPECT_NO_THROW(visualization::CameraController ctrl(camera, trackball));
}

/// 构造 null camera 应抛出
TEST(CameraController, NullCameraThrows) {
    auto [camera, trackball] = makeCamera();
    EXPECT_THROW(
        visualization::CameraController(nullptr, trackball),
        std::invalid_argument);
}

/// 构造 null trackball 应抛出
TEST(CameraController, NullTrackballThrows) {
    auto [camera, trackball] = makeCamera();
    EXPECT_THROW(
        visualization::CameraController(camera, nullptr),
        std::invalid_argument);
}

/// camera() 返回构造时传入的相机
TEST(CameraController, CameraAccessor) {
    auto [camera, trackball] = makeCamera();
    visualization::CameraController ctrl(camera, trackball);
    EXPECT_EQ(ctrl.camera(), camera);
}

/// trackball() 返回构造时传入的 trackball
TEST(CameraController, TrackballAccessor) {
    auto [camera, trackball] = makeCamera();
    visualization::CameraController ctrl(camera, trackball);
    EXPECT_EQ(ctrl.trackball(), trackball);
}

/// lookAt() 返回非空指针
TEST(CameraController, LookAtNonNull) {
    auto [camera, trackball] = makeCamera();
    visualization::CameraController ctrl(camera, trackball);
    EXPECT_NE(ctrl.lookAt(), nullptr);
}

/// computePresetLookAt: Front 视图 eye 在 +Y 方向
TEST(CameraController, PresetFrontEyeInPosY) {
    using visualization::CameraController;
    using visualization::ViewPreset;
    vsg::dvec3 center(0.0, 0.0, 0.0);
    double dist = 1000.0;
    auto la = CameraController::computePresetLookAt(ViewPreset::Front, center, dist);
    ASSERT_NE(la, nullptr);
    EXPECT_NEAR(la->eye.x, 0.0, 1e-9);
    EXPECT_GT(la->eye.y, 0.0);  // eye 在 +Y 方向
    EXPECT_NEAR(la->eye.z, 0.0, 1e-9);
    EXPECT_NEAR(la->up.z, 1.0, 1e-9);  // Z 向上
}

/// computePresetLookAt: Right 视图 eye 在 +X 方向
TEST(CameraController, PresetRightEyeInPosX) {
    using visualization::CameraController;
    using visualization::ViewPreset;
    auto la = CameraController::computePresetLookAt(ViewPreset::Right, {}, 1000.0);
    ASSERT_NE(la, nullptr);
    EXPECT_GT(la->eye.x, 0.0);
}

/// computePresetLookAt: Top 视图 eye 在 +Z 方向
TEST(CameraController, PresetTopEyeInPosZ) {
    using visualization::CameraController;
    using visualization::ViewPreset;
    auto la = CameraController::computePresetLookAt(ViewPreset::Top, {}, 1000.0);
    ASSERT_NE(la, nullptr);
    EXPECT_GT(la->eye.z, 0.0);
}

/// computePresetLookAt: Isometric 视图三个分量均不为零
TEST(CameraController, PresetIsoEyeAllNonZero) {
    using visualization::CameraController;
    using visualization::ViewPreset;
    auto la = CameraController::computePresetLookAt(ViewPreset::Isometric, {}, 1000.0);
    ASSERT_NE(la, nullptr);
    EXPECT_NE(la->eye.x, 0.0);
    EXPECT_NE(la->eye.y, 0.0);
    EXPECT_NE(la->eye.z, 0.0);
}

/// setViewPreset 不崩溃（调用 trackball->setViewpoint）
TEST(CameraController, SetViewPresetDoesNotCrash) {
    auto [camera, trackball] = makeCamera();
    visualization::CameraController ctrl(camera, trackball);
    for (auto preset : {
            visualization::ViewPreset::Front,
            visualization::ViewPreset::Right,
            visualization::ViewPreset::Top,
            visualization::ViewPreset::Isometric})
    {
        EXPECT_NO_THROW(ctrl.setViewPreset(preset, 0.0));
    }
}

/// fitAll(bounds): 有效包围盒不崩溃
TEST(CameraController, FitAllBoundsDoesNotCrash) {
    auto [camera, trackball] = makeCamera();
    visualization::CameraController ctrl(camera, trackball);

    vsg::dbox bounds;
    bounds.add(vsg::dvec3(-500.0, -500.0, -500.0));
    bounds.add(vsg::dvec3( 500.0,  500.0,  500.0));
    EXPECT_NO_THROW(ctrl.fitAll(bounds));
}

/// fitAll(bounds): 无效包围盒（空场景）不崩溃
TEST(CameraController, FitAllInvalidBoundsDoesNotCrash) {
    auto [camera, trackball] = makeCamera();
    visualization::CameraController ctrl(camera, trackball);
    vsg::dbox emptyBounds; // valid() == false
    EXPECT_NO_THROW(ctrl.fitAll(emptyBounds));
}

/// fitAll(sceneRoot): sceneRoot 为空时不崩溃
TEST(CameraController, FitAllNullRootDoesNotCrash) {
    auto [camera, trackball] = makeCamera();
    visualization::CameraController ctrl(camera, trackball);
    EXPECT_NO_THROW(ctrl.fitAll(vsg::ref_ptr<vsg::Node>{}));
}

/// fitAll(sceneRoot): 带几何的场景根节点不崩溃
TEST(CameraController, FitAllWithSceneRoot) {
    auto [camera, trackball] = makeCamera();
    visualization::CameraController ctrl(camera, trackball);

    auto root = vsg::Group::create();
    auto child = vsg::MatrixTransform::create();
    child->matrix = vsg::translate(100.0, 200.0, 300.0);
    root->addChild(child);

    EXPECT_NO_THROW(ctrl.fitAll(vsg::ref_ptr<vsg::Node>(root)));
}

// ══════════════════════════════════════════════════════════════════════════════
// T13: SceneFurniture
// ══════════════════════════════════════════════════════════════════════════════

/// 默认构造后 axisNode, gridNode, gridSwitch 均非空
TEST(SceneFurniture, DefaultConstruct) {
    visualization::SceneFurniture f;
    EXPECT_NE(f.axisNode(),   nullptr);
    EXPECT_NE(f.gridNode(),   nullptr);
    EXPECT_NE(f.gridSwitch(), nullptr);
}

/// axisNode 含三个子节点（X/Y/Z 轴各一个 VertexDraw）
TEST(SceneFurniture, AxisNodeHasThreeChildren) {
    visualization::SceneFurniture f;
    auto axis = f.axisNode();
    ASSERT_NE(axis, nullptr);
    EXPECT_EQ(axis->children.size(), 3u);
}

/// 每个轴子节点是 VertexDraw，含 2 个顶点
TEST(SceneFurniture, AxisVertexDraw2Points) {
    visualization::SceneFurniture f(100.0);
    auto axis = f.axisNode();
    ASSERT_EQ(axis->children.size(), 3u);
    for (size_t i = 0; i < 3; ++i) {
        auto vd = axis->children[i].cast<vsg::VertexDraw>();
        ASSERT_NE(vd, nullptr) << "axis child " << i << " is not VertexDraw";
        EXPECT_EQ(vd->vertexCount, 2u) << "axis child " << i << " should have 2 vertices";
    }
}

/// gridNode 含一个子节点（批量 VertexDraw）
TEST(SceneFurniture, GridNodeHasChild) {
    visualization::SceneFurniture f;
    auto grid = f.gridNode();
    ASSERT_NE(grid, nullptr);
    EXPECT_GE(grid->children.size(), 1u);
}

/// gridNode 的 VertexDraw 顶点数与 divisions 一致：divisions=N → (N+1)*4 顶点
TEST(SceneFurniture, GridVertexCount) {
    constexpr uint32_t N = 10;
    visualization::SceneFurniture f(100.0, 1000.0, N);
    auto grid = f.gridNode();
    ASSERT_EQ(grid->children.size(), 1u);
    auto vd = grid->children[0].cast<vsg::VertexDraw>();
    ASSERT_NE(vd, nullptr);
    EXPECT_EQ(vd->vertexCount, (N + 1) * 4u);
}

/// gridSwitch 绑定了 gridNode
TEST(SceneFurniture, GridSwitchHasChild) {
    visualization::SceneFurniture f;
    auto sw = f.gridSwitch();
    ASSERT_NE(sw, nullptr);
    EXPECT_EQ(sw->children.size(), 1u);
    EXPECT_EQ(sw->children[0].node, f.gridNode());
}

/// 默认可见
TEST(SceneFurniture, GridDefaultVisible) {
    visualization::SceneFurniture f;
    EXPECT_TRUE(f.isGridVisible());
}

/// setGridVisible(false) → isGridVisible() == false
TEST(SceneFurniture, SetGridInvisible) {
    visualization::SceneFurniture f;
    f.setGridVisible(false);
    EXPECT_FALSE(f.isGridVisible());
}

/// setGridVisible(true) → isGridVisible() == true
TEST(SceneFurniture, SetGridVisible) {
    visualization::SceneFurniture f;
    f.setGridVisible(false);
    f.setGridVisible(true);
    EXPECT_TRUE(f.isGridVisible());
}

/// gridSwitch mask 随 setGridVisible 变化
TEST(SceneFurniture, GridSwitchMaskFollowsVisible) {
    visualization::SceneFurniture f;
    auto sw = f.gridSwitch();

    f.setGridVisible(false);
    EXPECT_EQ(sw->children[0].mask, vsg::MASK_OFF);

    f.setGridVisible(true);
    EXPECT_NE(sw->children[0].mask, vsg::MASK_OFF);
}

/// 背景颜色常量值正确
TEST(SceneFurniture, BackgroundColors) {
    // #E8E8E8 = 232/255 ≈ 0.9098
    EXPECT_NEAR(visualization::SceneFurniture::backgroundTopR(), 232.f/255.f, 0.002f);
    EXPECT_NEAR(visualization::SceneFurniture::backgroundTopG(), 232.f/255.f, 0.002f);
    EXPECT_NEAR(visualization::SceneFurniture::backgroundTopB(), 232.f/255.f, 0.002f);

    // #D0D0D0 = 208/255 ≈ 0.8157
    EXPECT_NEAR(visualization::SceneFurniture::backgroundBotR(), 208.f/255.f, 0.002f);
    EXPECT_NEAR(visualization::SceneFurniture::backgroundBotG(), 208.f/255.f, 0.002f);
    EXPECT_NEAR(visualization::SceneFurniture::backgroundBotB(), 208.f/255.f, 0.002f);
}

/// 自定义参数构建（small axis, large grid, few divisions）
TEST(SceneFurniture, CustomParams) {
    visualization::SceneFurniture f(50.0, 2000.0, 5u);
    auto axis = f.axisNode();
    EXPECT_EQ(axis->children.size(), 3u);

    auto grid = f.gridNode();
    ASSERT_EQ(grid->children.size(), 1u);
    auto vd = grid->children[0].cast<vsg::VertexDraw>();
    ASSERT_NE(vd, nullptr);
    EXPECT_EQ(vd->vertexCount, (5u + 1u) * 4u);
}
