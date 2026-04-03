// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QGuiApplication>
#include <QSignalSpy>

#include "ui/VsgQuickItem.h"
#include "visualization/CameraController.h"
#include "visualization/SceneFurniture.h"
#include "visualization/SceneManager.h"

#include <vsg/app/Camera.h>
#include <vsg/app/Trackball.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/state/ViewportState.h>

// Helper: deliver event via QCoreApplication (event() is protected in QQuickItem)
static void sendEvent(ui::VsgQuickItem* item, QEvent* event)
{
    QCoreApplication::sendEvent(item, event);
}

// ---------------------------------------------------------------------------
// Global QGuiApplication (required for QQuickItem)
// ---------------------------------------------------------------------------

static QGuiApplication* qApp_ = nullptr;

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qApp_ = new QGuiApplication(argc, argv);
    int result = RUN_ALL_TESTS();
    delete qApp_;
    return result;
}

// ===================================================================
// Event conversion tests (pure utility, no Qt objects needed)
// ===================================================================

TEST(EventConversion, QtButtonToVsgMask)
{
    using ui::VsgQuickItem;
    EXPECT_EQ(VsgQuickItem::qtButtonToVsgMask(Qt::LeftButton),   vsg::BUTTON_MASK_1);
    EXPECT_EQ(VsgQuickItem::qtButtonToVsgMask(Qt::MiddleButton), vsg::BUTTON_MASK_2);
    EXPECT_EQ(VsgQuickItem::qtButtonToVsgMask(Qt::RightButton),  vsg::BUTTON_MASK_3);
    EXPECT_EQ(VsgQuickItem::qtButtonToVsgMask(Qt::NoButton),     vsg::BUTTON_MASK_OFF);
}

TEST(EventConversion, QtButtonsToVsgMask)
{
    using ui::VsgQuickItem;
    auto mask = VsgQuickItem::qtButtonsToVsgMask(Qt::LeftButton | Qt::RightButton);
    EXPECT_TRUE(mask & vsg::BUTTON_MASK_1);
    EXPECT_TRUE(mask & vsg::BUTTON_MASK_3);
    EXPECT_FALSE(mask & vsg::BUTTON_MASK_2);
}

TEST(EventConversion, QtModifiersToVsg)
{
    using ui::VsgQuickItem;
    auto mod = VsgQuickItem::qtModifiersToVsg(Qt::ShiftModifier | Qt::ControlModifier);
    EXPECT_TRUE(mod & vsg::MODKEY_Shift);
    EXPECT_TRUE(mod & vsg::MODKEY_Control);
    EXPECT_FALSE(mod & vsg::MODKEY_Alt);
}

TEST(EventConversion, QtKeyToVsg_Letters)
{
    using ui::VsgQuickItem;
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_A), vsg::KEY_a);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Z), vsg::KEY_z);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_F), vsg::KEY_f);
}

TEST(EventConversion, QtKeyToVsg_Digits)
{
    using ui::VsgQuickItem;
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_0), vsg::KEY_0);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_9), vsg::KEY_9);
}

TEST(EventConversion, QtKeyToVsg_SpecialKeys)
{
    using ui::VsgQuickItem;
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Space),     vsg::KEY_Space);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Return),    vsg::KEY_Return);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Escape),    vsg::KEY_Escape);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Delete),    vsg::KEY_Delete);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Tab),       vsg::KEY_Tab);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Backspace), vsg::KEY_BackSpace);
}

TEST(EventConversion, QtKeyToVsg_ArrowKeys)
{
    using ui::VsgQuickItem;
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Left),  vsg::KEY_Left);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Right), vsg::KEY_Right);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Up),    vsg::KEY_Up);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Down),  vsg::KEY_Down);
}

TEST(EventConversion, QtKeyToVsg_FunctionKeys)
{
    using ui::VsgQuickItem;
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_F1),  vsg::KEY_F1);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_F12), vsg::KEY_F12);
}

TEST(EventConversion, QtKeyToVsg_Modifiers)
{
    using ui::VsgQuickItem;
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Shift),   vsg::KEY_Shift_L);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Control), vsg::KEY_Control_L);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Alt),     vsg::KEY_Alt_L);
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(Qt::Key_Meta),    vsg::KEY_Meta_L);
}

TEST(EventConversion, QtKeyToVsg_Unknown)
{
    using ui::VsgQuickItem;
    EXPECT_EQ(VsgQuickItem::qtKeyToVsg(0xFFFF), vsg::KEY_Undefined);
}

// ===================================================================
// VsgQuickItem construction and state tests
// ===================================================================

class VsgQuickItemTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        item_ = std::make_unique<ui::VsgQuickItem>();
    }

    std::unique_ptr<ui::VsgQuickItem> item_;
};

TEST_F(VsgQuickItemTest, DefaultConstruction)
{
    EXPECT_EQ(item_->sceneManager(), nullptr);
    EXPECT_EQ(item_->cameraController(), nullptr);
    EXPECT_EQ(item_->sceneFurniture(), nullptr);
    EXPECT_TRUE(item_->acceptedMouseButtons() == Qt::AllButtons);
}

TEST_F(VsgQuickItemTest, SetSceneManager)
{
    visualization::SceneManager mgr;
    item_->setSceneManager(&mgr);
    EXPECT_EQ(item_->sceneManager(), &mgr);
}

TEST_F(VsgQuickItemTest, SetCameraController)
{
    auto perspective = vsg::Perspective::create(60.0, 1.0, 0.1, 10000.0);
    auto lookAt      = vsg::LookAt::create(
        vsg::dvec3(0, -500, 500), vsg::dvec3(0, 0, 0), vsg::dvec3(0, 0, 1));
    auto viewport = vsg::ViewportState::create(0, 0, 800, 600);
    auto camera   = vsg::Camera::create(perspective, lookAt, viewport);
    auto trackball = vsg::Trackball::create(camera);
    visualization::CameraController ctrl(camera, trackball);

    item_->setCameraController(&ctrl);
    EXPECT_EQ(item_->cameraController(), &ctrl);
}

TEST_F(VsgQuickItemTest, SetSceneFurniture)
{
    visualization::SceneFurniture furniture;
    item_->setSceneFurniture(&furniture);
    EXPECT_EQ(item_->sceneFurniture(), &furniture);
}

// ===================================================================
// Grid visibility tests
// ===================================================================

TEST_F(VsgQuickItemTest, GridVisibleDefault)
{
    // No furniture → defaults to true
    EXPECT_TRUE(item_->isGridVisible());
}

TEST_F(VsgQuickItemTest, GridVisibleWithFurniture)
{
    visualization::SceneFurniture furniture;
    item_->setSceneFurniture(&furniture);

    EXPECT_TRUE(item_->isGridVisible());
    item_->setGridVisible(false);
    EXPECT_FALSE(item_->isGridVisible());
    EXPECT_FALSE(furniture.isGridVisible());
}

TEST_F(VsgQuickItemTest, GridVisibleSignal)
{
    visualization::SceneFurniture furniture;
    item_->setSceneFurniture(&furniture);

    QSignalSpy spy(item_.get(), &ui::VsgQuickItem::gridVisibleChanged);
    item_->setGridVisible(false);
    EXPECT_EQ(spy.count(), 1);

    // Setting same value should not signal
    item_->setGridVisible(false);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(VsgQuickItemTest, ToggleGrid)
{
    visualization::SceneFurniture furniture;
    item_->setSceneFurniture(&furniture);

    EXPECT_TRUE(item_->isGridVisible());
    item_->toggleGrid();
    EXPECT_FALSE(item_->isGridVisible());
    item_->toggleGrid();
    EXPECT_TRUE(item_->isGridVisible());
}

// ===================================================================
// View preset tests
// ===================================================================

TEST_F(VsgQuickItemTest, SetViewPresetBoundsCheck)
{
    auto perspective = vsg::Perspective::create(60.0, 1.0, 0.1, 10000.0);
    auto lookAt      = vsg::LookAt::create(
        vsg::dvec3(0, -500, 500), vsg::dvec3(0, 0, 0), vsg::dvec3(0, 0, 1));
    auto viewport = vsg::ViewportState::create(0, 0, 800, 600);
    auto camera   = vsg::Camera::create(perspective, lookAt, viewport);
    auto trackball = vsg::Trackball::create(camera);
    visualization::CameraController ctrl(camera, trackball);
    item_->setCameraController(&ctrl);

    // Valid presets should not crash
    item_->setViewPreset(0); // Front
    item_->setViewPreset(1); // Right
    item_->setViewPreset(2); // Top
    item_->setViewPreset(3); // Isometric

    // Invalid presets should be silently ignored
    item_->setViewPreset(-1);
    item_->setViewPreset(99);
}

// ===================================================================
// FitAll test
// ===================================================================

TEST_F(VsgQuickItemTest, FitAllNoCrashWhenNoController)
{
    // Should not crash when no controller/scene set
    item_->fitAll();
}

TEST_F(VsgQuickItemTest, FitAllWithScene)
{
    visualization::SceneManager mgr;
    auto perspective = vsg::Perspective::create(60.0, 1.0, 0.1, 10000.0);
    auto lookAt      = vsg::LookAt::create(
        vsg::dvec3(0, -500, 500), vsg::dvec3(0, 0, 0), vsg::dvec3(0, 0, 1));
    auto viewport = vsg::ViewportState::create(0, 0, 800, 600);
    auto camera   = vsg::Camera::create(perspective, lookAt, viewport);
    auto trackball = vsg::Trackball::create(camera);
    visualization::CameraController ctrl(camera, trackball);

    item_->setSceneManager(&mgr);
    item_->setCameraController(&ctrl);

    // Should not crash with empty scene
    item_->fitAll();
}

// ===================================================================
// Signal tests
// ===================================================================

TEST_F(VsgQuickItemTest, RenderRequestedSignal)
{
    visualization::SceneFurniture furniture;
    item_->setSceneFurniture(&furniture);

    QSignalSpy spy(item_.get(), &ui::VsgQuickItem::renderRequested);
    item_->requestRender();
    EXPECT_GE(spy.count(), 1);
}

TEST_F(VsgQuickItemTest, DeleteRequestedSignal)
{
    QSignalSpy spy(item_.get(), &ui::VsgQuickItem::deleteRequested);
    // Simulate Delete key press
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    sendEvent(item_.get(), &event);
    EXPECT_EQ(spy.count(), 1);
}

// ===================================================================
// Keyboard shortcut dispatch tests
// ===================================================================

TEST_F(VsgQuickItemTest, KeyF_TriggersFitAll)
{
    visualization::SceneManager mgr;
    auto perspective = vsg::Perspective::create(60.0, 1.0, 0.1, 10000.0);
    auto lookAt      = vsg::LookAt::create(
        vsg::dvec3(0, -500, 500), vsg::dvec3(0, 0, 0), vsg::dvec3(0, 0, 1));
    auto viewport = vsg::ViewportState::create(0, 0, 800, 600);
    auto camera   = vsg::Camera::create(perspective, lookAt, viewport);
    auto trackball = vsg::Trackball::create(camera);
    visualization::CameraController ctrl(camera, trackball);

    item_->setSceneManager(&mgr);
    item_->setCameraController(&ctrl);

    QSignalSpy spy(item_.get(), &ui::VsgQuickItem::renderRequested);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_F, Qt::NoModifier);
    sendEvent(item_.get(), &event);
    EXPECT_GE(spy.count(), 1);
}

TEST_F(VsgQuickItemTest, KeyG_TogglesGrid)
{
    visualization::SceneFurniture furniture;
    item_->setSceneFurniture(&furniture);

    EXPECT_TRUE(item_->isGridVisible());
    QKeyEvent event(QEvent::KeyPress, Qt::Key_G, Qt::NoModifier);
    sendEvent(item_.get(), &event);
    EXPECT_FALSE(item_->isGridVisible());
}

// ===================================================================
// Mouse event forwarding (smoke tests — no crash)
// ===================================================================

class VsgQuickItemMouseTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        item_ = std::make_unique<ui::VsgQuickItem>();

        auto perspective = vsg::Perspective::create(60.0, 1.0, 0.1, 10000.0);
        auto lookAt      = vsg::LookAt::create(
            vsg::dvec3(0, -500, 500), vsg::dvec3(0, 0, 0), vsg::dvec3(0, 0, 1));
        auto viewport = vsg::ViewportState::create(0, 0, 800, 600);
        camera_   = vsg::Camera::create(perspective, lookAt, viewport);
        trackball_ = vsg::Trackball::create(camera_);
        ctrl_ = std::make_unique<visualization::CameraController>(camera_, trackball_);

        item_->setCameraController(ctrl_.get());
    }

    std::unique_ptr<ui::VsgQuickItem> item_;
    vsg::ref_ptr<vsg::Camera> camera_;
    vsg::ref_ptr<vsg::Trackball> trackball_;
    std::unique_ptr<visualization::CameraController> ctrl_;
};

TEST_F(VsgQuickItemMouseTest, MousePressRelease_NoCrash)
{
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(100, 100), QPointF(100, 100),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    sendEvent(item_.get(), &press);

    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(100, 100), QPointF(100, 100),
                        Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    sendEvent(item_.get(), &release);
}

TEST_F(VsgQuickItemMouseTest, MouseMove_NoCrash)
{
    QMouseEvent move(QEvent::MouseMove, QPointF(200, 200), QPointF(200, 200),
                     Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    sendEvent(item_.get(), &move);
}

TEST_F(VsgQuickItemMouseTest, Wheel_NoCrash)
{
    QWheelEvent wheel(QPointF(100, 100), QPointF(100, 100),
                      QPoint(0, 0), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier,
                      Qt::NoScrollPhase, false);
    sendEvent(item_.get(), &wheel);
}
