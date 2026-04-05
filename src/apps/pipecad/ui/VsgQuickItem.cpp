// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "ui/VsgQuickItem.h"

#include <QSGSimpleRectNode>
#include <vsg/ui/ScrollWheelEvent.h>

namespace ui {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

VsgQuickItem::VsgQuickItem(QQuickItem* parent)
    : QQuickItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(true);
}

VsgQuickItem::~VsgQuickItem() = default;

// ---------------------------------------------------------------------------
// Scene management
// ---------------------------------------------------------------------------

void VsgQuickItem::setSceneManager(visualization::SceneManager* mgr)
{
    sceneMgr_ = mgr;
}

visualization::SceneManager* VsgQuickItem::sceneManager() const
{
    return sceneMgr_;
}

void VsgQuickItem::setCameraController(visualization::CameraController* ctrl)
{
    cameraCtrl_ = ctrl;
}

visualization::CameraController* VsgQuickItem::cameraController() const
{
    return cameraCtrl_;
}

void VsgQuickItem::setSceneFurniture(visualization::SceneFurniture* furniture)
{
    furniture_ = furniture;
}

visualization::SceneFurniture* VsgQuickItem::sceneFurniture() const
{
    return furniture_;
}

// ---------------------------------------------------------------------------
// Grid visibility (Q_PROPERTY)
// ---------------------------------------------------------------------------

bool VsgQuickItem::isGridVisible() const
{
    return furniture_ ? furniture_->isGridVisible() : true;
}

void VsgQuickItem::setGridVisible(bool visible)
{
    if (!furniture_) return;
    if (furniture_->isGridVisible() != visible) {
        furniture_->setGridVisible(visible);
        emit gridVisibleChanged();
        requestRender();
    }
}

// ---------------------------------------------------------------------------
// QML invokable
// ---------------------------------------------------------------------------

void VsgQuickItem::fitAll()
{
    if (cameraCtrl_ && sceneMgr_) {
        cameraCtrl_->fitAll(sceneMgr_->root());
        requestRender();
    }
}

void VsgQuickItem::setViewPreset(int preset)
{
    if (!cameraCtrl_) return;
    if (preset < 0 || preset > static_cast<int>(visualization::ViewPreset::Isometric))
        return;
    cameraCtrl_->setViewPreset(static_cast<visualization::ViewPreset>(preset));
    requestRender();
}

void VsgQuickItem::toggleGrid()
{
    setGridVisible(!isGridVisible());
}

void VsgQuickItem::requestRender()
{
    dirty_ = true;
    update();
    emit renderRequested();
}

// ---------------------------------------------------------------------------
// Event conversion utilities
// ---------------------------------------------------------------------------

vsg::ButtonMask VsgQuickItem::qtButtonToVsgMask(Qt::MouseButton btn)
{
    switch (btn) {
    case Qt::LeftButton:   return vsg::BUTTON_MASK_1;
    case Qt::MiddleButton: return vsg::BUTTON_MASK_2;
    case Qt::RightButton:  return vsg::BUTTON_MASK_3;
    default:               return vsg::BUTTON_MASK_OFF;
    }
}

vsg::ButtonMask VsgQuickItem::qtButtonsToVsgMask(Qt::MouseButtons btns)
{
    uint16_t mask = 0;
    if (btns & Qt::LeftButton)   mask |= vsg::BUTTON_MASK_1;
    if (btns & Qt::MiddleButton) mask |= vsg::BUTTON_MASK_2;
    if (btns & Qt::RightButton)  mask |= vsg::BUTTON_MASK_3;
    return static_cast<vsg::ButtonMask>(mask);
}

vsg::KeyModifier VsgQuickItem::qtModifiersToVsg(Qt::KeyboardModifiers mods)
{
    uint16_t mod = 0;
    if (mods & Qt::ShiftModifier)   mod |= vsg::MODKEY_Shift;
    if (mods & Qt::ControlModifier) mod |= vsg::MODKEY_Control;
    if (mods & Qt::AltModifier)     mod |= vsg::MODKEY_Alt;
    if (mods & Qt::MetaModifier)    mod |= vsg::MODKEY_Meta;
    return static_cast<vsg::KeyModifier>(mod);
}

vsg::KeySymbol VsgQuickItem::qtKeyToVsg(int qtKey)
{
    // Letters (Qt uses uppercase enum values, VSG lowercase)
    if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
        return static_cast<vsg::KeySymbol>(vsg::KEY_a + (qtKey - Qt::Key_A));

    // Digits
    if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)
        return static_cast<vsg::KeySymbol>(vsg::KEY_0 + (qtKey - Qt::Key_0));

    // Function keys
    if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F12)
        return static_cast<vsg::KeySymbol>(vsg::KEY_F1 + (qtKey - Qt::Key_F1));

    switch (qtKey) {
    case Qt::Key_Space:     return vsg::KEY_Space;
    case Qt::Key_Return:
    case Qt::Key_Enter:     return vsg::KEY_Return;
    case Qt::Key_Escape:    return vsg::KEY_Escape;
    case Qt::Key_Tab:       return vsg::KEY_Tab;
    case Qt::Key_Backspace: return vsg::KEY_BackSpace;
    case Qt::Key_Delete:    return vsg::KEY_Delete;
    case Qt::Key_Left:      return vsg::KEY_Left;
    case Qt::Key_Right:     return vsg::KEY_Right;
    case Qt::Key_Up:        return vsg::KEY_Up;
    case Qt::Key_Down:      return vsg::KEY_Down;
    case Qt::Key_Home:      return vsg::KEY_Home;
    case Qt::Key_End:       return vsg::KEY_End;
    case Qt::Key_PageUp:    return vsg::KEY_Page_Up;
    case Qt::Key_PageDown:  return vsg::KEY_Page_Down;
    case Qt::Key_Insert:    return vsg::KEY_Insert;
    case Qt::Key_Shift:     return vsg::KEY_Shift_L;
    case Qt::Key_Control:   return vsg::KEY_Control_L;
    case Qt::Key_Alt:       return vsg::KEY_Alt_L;
    case Qt::Key_Meta:      return vsg::KEY_Meta_L;
    default:                return vsg::KEY_Undefined;
    }
}

// ---------------------------------------------------------------------------
// Mouse event forwarding
// ---------------------------------------------------------------------------

void VsgQuickItem::mousePressEvent(QMouseEvent* event)
{
    auto btnMask = qtButtonToVsgMask(event->button());
    currentMask_ = static_cast<vsg::ButtonMask>(
        static_cast<uint16_t>(currentMask_) | static_cast<uint16_t>(btnMask));

    if (cameraCtrl_) {
        auto t = vsg::clock::now();
        auto vsgEvent = vsg::ButtonPressEvent::create(
            nullptr, t,
            static_cast<int32_t>(event->position().x()),
            static_cast<int32_t>(event->position().y()),
            currentMask_,
            static_cast<uint32_t>(btnMask));
        vsgEvent->accept(*(cameraCtrl_->trackball()));
        requestRender();
    }
    event->accept();
}

void VsgQuickItem::mouseReleaseEvent(QMouseEvent* event)
{
    auto btnMask = qtButtonToVsgMask(event->button());
    currentMask_ = static_cast<vsg::ButtonMask>(
        static_cast<uint16_t>(currentMask_) & ~static_cast<uint16_t>(btnMask));

    if (cameraCtrl_) {
        auto t = vsg::clock::now();
        auto vsgEvent = vsg::ButtonReleaseEvent::create(
            nullptr, t,
            static_cast<int32_t>(event->position().x()),
            static_cast<int32_t>(event->position().y()),
            currentMask_,
            static_cast<uint32_t>(btnMask));
        vsgEvent->accept(*(cameraCtrl_->trackball()));
        requestRender();
    }
    event->accept();
}

void VsgQuickItem::mouseMoveEvent(QMouseEvent* event)
{
    if (cameraCtrl_) {
        auto t = vsg::clock::now();
        auto vsgEvent = vsg::MoveEvent::create(
            nullptr, t,
            static_cast<int32_t>(event->position().x()),
            static_cast<int32_t>(event->position().y()),
            currentMask_);
        vsgEvent->accept(*(cameraCtrl_->trackball()));
        requestRender();
    }
    event->accept();
}

void VsgQuickItem::wheelEvent(QWheelEvent* event)
{
    if (cameraCtrl_) {
        auto t = vsg::clock::now();
        // Qt angleDelta is in 1/8 degree units; normalize to ±1 scroll steps
        float dy = event->angleDelta().y() > 0
                       ? 1.0f
                       : (event->angleDelta().y() < 0 ? -1.0f : 0.0f);
        auto vsgEvent = vsg::ScrollWheelEvent::create(
            nullptr, t, vsg::vec3(0.0f, dy, 0.0f));
        vsgEvent->accept(*(cameraCtrl_->trackball()));
        requestRender();
    }
    event->accept();
}

// ---------------------------------------------------------------------------
// Keyboard event handling
// ---------------------------------------------------------------------------

void VsgQuickItem::keyPressEvent(QKeyEvent* event)
{
    bool handled = false;

    // Application-level shortcuts
    switch (event->key()) {
    case Qt::Key_F:
        fitAll();
        handled = true;
        break;
    case Qt::Key_Delete:
        emit deleteRequested();
        handled = true;
        break;
    case Qt::Key_G:
        toggleGrid();
        handled = true;
        break;
    default:
        break;
    }

    // Numpad view presets
    if (!handled && (event->modifiers() & Qt::KeypadModifier)) {
        switch (event->key()) {
        case Qt::Key_1:
            setViewPreset(static_cast<int>(visualization::ViewPreset::Front));
            handled = true;
            break;
        case Qt::Key_3:
            setViewPreset(static_cast<int>(visualization::ViewPreset::Right));
            handled = true;
            break;
        case Qt::Key_7:
            setViewPreset(static_cast<int>(visualization::ViewPreset::Top));
            handled = true;
            break;
        case Qt::Key_0:
            setViewPreset(static_cast<int>(visualization::ViewPreset::Isometric));
            handled = true;
            break;
        default:
            break;
        }
    }

    // Forward unhandled keys to trackball
    if (!handled && cameraCtrl_) {
        auto t = vsg::clock::now();
        auto vsgKey = qtKeyToVsg(event->key());
        auto vsgMod = qtModifiersToVsg(event->modifiers());
        auto vsgEvent = vsg::KeyPressEvent::create(
            nullptr, t, vsgKey, vsgKey, vsgMod);
        vsgEvent->accept(*(cameraCtrl_->trackball()));
        requestRender();
    }

    if (handled)
        event->accept();
    else
        QQuickItem::keyPressEvent(event);
}

void VsgQuickItem::keyReleaseEvent(QKeyEvent* event)
{
    if (cameraCtrl_) {
        auto t = vsg::clock::now();
        auto vsgKey = qtKeyToVsg(event->key());
        auto vsgMod = qtModifiersToVsg(event->modifiers());
        auto vsgEvent = vsg::KeyReleaseEvent::create(
            nullptr, t, vsgKey, vsgKey, vsgMod);
        vsgEvent->accept(*(cameraCtrl_->trackball()));
    }
    QQuickItem::keyReleaseEvent(event);
}

// ---------------------------------------------------------------------------
// Geometry change — keep camera viewport in sync with item size
// ---------------------------------------------------------------------------

void VsgQuickItem::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    if (newGeometry.size() != oldGeometry.size() && cameraCtrl_) {
        auto cam = cameraCtrl_->camera();
        if (cam && cam->viewportState) {
            auto w = static_cast<uint32_t>(newGeometry.width());
            auto h = static_cast<uint32_t>(newGeometry.height());
            cam->viewportState->set(0, 0, w, h);
        }
        dirty_ = true;
        update();
    }
}

// ---------------------------------------------------------------------------
// Scene graph node — placeholder rect with background colour
// ---------------------------------------------------------------------------

QSGNode* VsgQuickItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    // Placeholder: solid background colour from SceneFurniture constants.
    // Actual VSG offscreen → texture rendering is integrated in the application
    // entry point (T17) once a Vulkan context is available.
    auto* rectNode = static_cast<QSGSimpleRectNode*>(oldNode);
    if (!rectNode)
        rectNode = new QSGSimpleRectNode();

    rectNode->setColor(QColor::fromRgbF(
        visualization::SceneFurniture::backgroundBotR(),
        visualization::SceneFurniture::backgroundBotG(),
        visualization::SceneFurniture::backgroundBotB()));
    rectNode->setRect(boundingRect());

    dirty_ = false;
    return rectNode;
}

} // namespace ui
