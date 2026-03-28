#include <gtest/gtest.h>

#include "visualization/ViewManager.h"

using namespace visualization;

// --- 基础构造 ---

TEST(ViewManager, DefaultState) {
    ViewManager vm;
    EXPECT_EQ(vm.activeViewport(), ViewManager::ActiveViewport::VSG);
    EXPECT_EQ(vm.renderMode(), ViewManager::RenderMode::Solid);
    EXPECT_EQ(vm.lodLevel(), ViewManager::LodLevel::Normal);
    EXPECT_TRUE(vm.isTriadVisible());
}

TEST(ViewManager, DefaultCategoryVisibility) {
    ViewManager vm;
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::PipePoints));
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::Segments));
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::Accessories));
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::Supports));
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::Beams));
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::Annotations));
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::LoadArrows));
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::StressContour));
}

// --- 视口切换 ---

TEST(ViewManager, SetActiveViewport) {
    ViewManager vm;
    vm.setActiveViewport(ViewManager::ActiveViewport::VTK);
    EXPECT_EQ(vm.activeViewport(), ViewManager::ActiveViewport::VTK);
    vm.setActiveViewport(ViewManager::ActiveViewport::VSG);
    EXPECT_EQ(vm.activeViewport(), ViewManager::ActiveViewport::VSG);
}

// --- 渲染模式 ---

TEST(ViewManager, RenderModes) {
    ViewManager vm;
    vm.setRenderMode(ViewManager::RenderMode::Wireframe);
    EXPECT_EQ(vm.renderMode(), ViewManager::RenderMode::Wireframe);
    vm.setRenderMode(ViewManager::RenderMode::SolidWithEdges);
    EXPECT_EQ(vm.renderMode(), ViewManager::RenderMode::SolidWithEdges);
    vm.setRenderMode(ViewManager::RenderMode::Beam);
    EXPECT_EQ(vm.renderMode(), ViewManager::RenderMode::Beam);
    vm.setRenderMode(ViewManager::RenderMode::Solid);
    EXPECT_EQ(vm.renderMode(), ViewManager::RenderMode::Solid);
}

// --- Category 可见性 ---

TEST(ViewManager, CategoryVisibilityToggle) {
    ViewManager vm;
    vm.setCategoryVisible(ViewManager::Category::PipePoints, false);
    EXPECT_FALSE(vm.isCategoryVisible(ViewManager::Category::PipePoints));
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::Segments));

    vm.setCategoryVisible(ViewManager::Category::PipePoints, true);
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::PipePoints));
}

TEST(ViewManager, MultipleCategoryToggle) {
    ViewManager vm;
    vm.setCategoryVisible(ViewManager::Category::LoadArrows, false);
    vm.setCategoryVisible(ViewManager::Category::StressContour, false);
    EXPECT_FALSE(vm.isCategoryVisible(ViewManager::Category::LoadArrows));
    EXPECT_FALSE(vm.isCategoryVisible(ViewManager::Category::StressContour));
    EXPECT_TRUE(vm.isCategoryVisible(ViewManager::Category::Beams));
}

// --- LOD ---

TEST(ViewManager, LodLevels) {
    ViewManager vm;
    vm.setLodLevel(ViewManager::LodLevel::Draft);
    EXPECT_EQ(vm.lodLevel(), ViewManager::LodLevel::Draft);
    vm.setLodLevel(ViewManager::LodLevel::Fine);
    EXPECT_EQ(vm.lodLevel(), ViewManager::LodLevel::Fine);
    vm.setLodLevel(ViewManager::LodLevel::Normal);
    EXPECT_EQ(vm.lodLevel(), ViewManager::LodLevel::Normal);
}

// --- Triad 可见性 ---

TEST(ViewManager, TriadVisibility) {
    ViewManager vm;
    vm.setTriadVisible(false);
    EXPECT_FALSE(vm.isTriadVisible());
    vm.setTriadVisible(true);
    EXPECT_TRUE(vm.isTriadVisible());
}

// --- Mouse world position ---

TEST(ViewManager, MouseWorldPos) {
    ViewManager vm;
    auto pos = vm.currentMouseWorldPos();
    EXPECT_DOUBLE_EQ(pos.X(), 0.0);
    EXPECT_DOUBLE_EQ(pos.Y(), 0.0);
    EXPECT_DOUBLE_EQ(pos.Z(), 0.0);

    vm.setMouseWorldPos(gp_Pnt(100.5, 200.3, -50.7));
    pos = vm.currentMouseWorldPos();
    EXPECT_DOUBLE_EQ(pos.X(), 100.5);
    EXPECT_DOUBLE_EQ(pos.Y(), 200.3);
    EXPECT_DOUBLE_EQ(pos.Z(), -50.7);
}

// --- captureImage (预留，当前返回 false) ---

TEST(ViewManager, CaptureImageStub) {
    ViewManager vm;
    EXPECT_FALSE(vm.captureImage("/tmp/test.png"));
}

// --- VSG 注入 + 委派测试 ---

TEST(ViewManager, InjectVsgComponents) {
    ViewManager vm;
    // 在无 VSG 组件时调用不崩溃
    vm.fitAll();
    vm.setViewPreset(ViewPreset::Isometric);
    vm.saveViewState("test");
    vm.restoreViewState("test");
    vm.setGridVisible(true);
    EXPECT_TRUE(vm.isGridVisible()); // 无 furniture 返回 true（默认值）
}

TEST(ViewManager, InjectVsgComponentsWithScene) {
    SceneManager scene;
    auto perspective = vsg::Perspective::create(60.0, 1.0, 0.1, 10000.0);
    auto lookAt = vsg::LookAt::create(vsg::dvec3{0, -10, 0}, vsg::dvec3{0, 0, 0}, vsg::dvec3{0, 0, 1});
    auto viewport = vsg::ViewportState::create(0, 0, 800, 600);
    auto camera = vsg::Camera::create(perspective, lookAt, viewport);
    auto trackball = vsg::Trackball::create(camera);
    CameraController ctrl(camera, trackball);
    SceneFurniture furniture;

    ViewManager vm;
    vm.setVsgComponents(&scene, &ctrl, &furniture);

    // fitAll 不崩溃（空场景）
    vm.fitAll();

    // setViewPreset 不崩溃
    vm.setViewPreset(ViewPreset::Front);

    // Grid 控制委派给 SceneFurniture
    vm.setGridVisible(false);
    EXPECT_FALSE(vm.isGridVisible());
    EXPECT_FALSE(furniture.isGridVisible());
    vm.setGridVisible(true);
    EXPECT_TRUE(vm.isGridVisible());
}

// --- 视图状态缓存 ---

TEST(ViewManager, SaveRestoreViewState) {
    auto perspective = vsg::Perspective::create(60.0, 1.0, 0.1, 10000.0);
    auto lookAt = vsg::LookAt::create(vsg::dvec3{10, 20, 30}, vsg::dvec3{1, 2, 3}, vsg::dvec3{0, 0, 1});
    auto viewport = vsg::ViewportState::create(0, 0, 800, 600);
    auto camera = vsg::Camera::create(perspective, lookAt, viewport);
    auto trackball = vsg::Trackball::create(camera);
    CameraController ctrl(camera, trackball);
    SceneManager scene;
    SceneFurniture furniture;

    ViewManager vm;
    vm.setVsgComponents(&scene, &ctrl, &furniture);

    // 保存 Design 工作台的视图状态
    vm.saveViewState("Design");

    // 修改相机
    lookAt->eye = vsg::dvec3{100, 200, 300};
    lookAt->center = vsg::dvec3{10, 20, 30};

    // 保存 Analysis 工作台的视图状态
    vm.saveViewState("Analysis");

    // 恢复 Design 状态
    vm.restoreViewState("Design");
    auto la = ctrl.lookAt();
    EXPECT_DOUBLE_EQ(la->eye.x, 10.0);
    EXPECT_DOUBLE_EQ(la->eye.y, 20.0);
    EXPECT_DOUBLE_EQ(la->eye.z, 30.0);
    EXPECT_DOUBLE_EQ(la->center.x, 1.0);
    EXPECT_DOUBLE_EQ(la->center.y, 2.0);
    EXPECT_DOUBLE_EQ(la->center.z, 3.0);

    // 恢复 Analysis 状态
    vm.restoreViewState("Analysis");
    la = ctrl.lookAt();
    EXPECT_DOUBLE_EQ(la->eye.x, 100.0);
    EXPECT_DOUBLE_EQ(la->eye.y, 200.0);
    EXPECT_DOUBLE_EQ(la->eye.z, 300.0);
}

TEST(ViewManager, RestoreNonexistentStateIsNoop) {
    auto perspective = vsg::Perspective::create(60.0, 1.0, 0.1, 10000.0);
    auto lookAt = vsg::LookAt::create(vsg::dvec3{5, 5, 5}, vsg::dvec3{0, 0, 0}, vsg::dvec3{0, 0, 1});
    auto viewport = vsg::ViewportState::create(0, 0, 800, 600);
    auto camera = vsg::Camera::create(perspective, lookAt, viewport);
    auto trackball = vsg::Trackball::create(camera);
    CameraController ctrl(camera, trackball);
    SceneManager scene;

    ViewManager vm;
    vm.setVsgComponents(&scene, &ctrl, nullptr);

    // restoreViewState 对不存在的 key 不崩溃
    vm.restoreViewState("nonexistent");
    auto la = ctrl.lookAt();
    EXPECT_DOUBLE_EQ(la->eye.x, 5.0);
    EXPECT_DOUBLE_EQ(la->eye.y, 5.0);
    EXPECT_DOUBLE_EQ(la->eye.z, 5.0);
}

// --- 工作台切换完整流程 ---

TEST(ViewManager, WorkbenchSwitchFlow) {
    auto perspective = vsg::Perspective::create(60.0, 1.0, 0.1, 10000.0);
    auto lookAt = vsg::LookAt::create(vsg::dvec3{10, 20, 30}, vsg::dvec3{0, 0, 0}, vsg::dvec3{0, 0, 1});
    auto viewport = vsg::ViewportState::create(0, 0, 800, 600);
    auto camera = vsg::Camera::create(perspective, lookAt, viewport);
    auto trackball = vsg::Trackball::create(camera);
    CameraController ctrl(camera, trackball);
    SceneManager scene;
    SceneFurniture furniture;

    ViewManager vm;
    vm.setVsgComponents(&scene, &ctrl, &furniture);

    // 模拟: Design → Analysis 切换流程
    vm.saveViewState("Design");
    vm.setActiveViewport(ViewManager::ActiveViewport::VTK);
    vm.setRenderMode(ViewManager::RenderMode::Beam);

    EXPECT_EQ(vm.activeViewport(), ViewManager::ActiveViewport::VTK);
    EXPECT_EQ(vm.renderMode(), ViewManager::RenderMode::Beam);

    // 模拟: Analysis → Design 切换流程
    vm.saveViewState("Analysis");
    vm.setActiveViewport(ViewManager::ActiveViewport::VSG);
    vm.restoreViewState("Design");
    vm.setRenderMode(ViewManager::RenderMode::Solid);

    EXPECT_EQ(vm.activeViewport(), ViewManager::ActiveViewport::VSG);
    EXPECT_EQ(vm.renderMode(), ViewManager::RenderMode::Solid);

    // 相机确认恢复到 Design 状态
    auto la = ctrl.lookAt();
    EXPECT_DOUBLE_EQ(la->eye.x, 10.0);
    EXPECT_DOUBLE_EQ(la->eye.y, 20.0);
    EXPECT_DOUBLE_EQ(la->eye.z, 30.0);
}
