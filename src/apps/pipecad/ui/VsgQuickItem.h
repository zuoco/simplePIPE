// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <QQuickItem>

#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include "visualization/CameraController.h"
#include "visualization/SceneFurniture.h"
#include "visualization/SceneManager.h"

namespace ui {

/// @brief QML item that bridges VSG 3D rendering with Qt Quick
///
/// VsgQuickItem 是 VSG 渲染结果在 QML 中显示的桥接组件。
/// 它持有 SceneManager / CameraController / SceneFurniture 的非拥有指针，
/// 将 Qt 鼠标/键盘事件转换为 VSG 事件并转发给 Trackball，
/// 提供视图预设、FitAll、地面网格显隐等 QML 可调用接口。
///
/// 典型用法（在 main.cpp 中，T17 实现）：
/// @code
///   qmlRegisterType<ui::VsgQuickItem>("PipeCAD", 1, 0, "VsgViewport");
///   // 在 QML 中: VsgViewport { anchors.fill: parent }
/// @endcode
///
/// @note 基类使用 QQuickItem 而非 QQuickFramebufferObject，
///       因为 VSG 使用 Vulkan 渲染，QQuickFramebufferObject 仅支持 OpenGL 后端。
///       实际离屏渲染→纹理上传在应用层集成时完成（T17）。
class VsgQuickItem : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(bool gridVisible READ isGridVisible WRITE setGridVisible NOTIFY gridVisibleChanged)

public:
    explicit VsgQuickItem(QQuickItem* parent = nullptr);
    ~VsgQuickItem() override;

    /// @name 场景管理（非拥有指针，生命期由外部管理）
    /// @{
    void setSceneManager(visualization::SceneManager* mgr);
    visualization::SceneManager* sceneManager() const;

    void setCameraController(visualization::CameraController* ctrl);
    visualization::CameraController* cameraController() const;

    void setSceneFurniture(visualization::SceneFurniture* furniture);
    visualization::SceneFurniture* sceneFurniture() const;
    /// @}

    /// @name QML 属性
    /// @{
    bool isGridVisible() const;
    void setGridVisible(bool visible);
    /// @}

    /// @name QML 可调用方法
    /// @{
    Q_INVOKABLE void fitAll();
    Q_INVOKABLE void setViewPreset(int preset);
    Q_INVOKABLE void toggleGrid();
    Q_INVOKABLE void requestRender();
    /// @}

    /// @name 事件转换工具（公开以便单元测试）
    /// @{
    static vsg::ButtonMask qtButtonToVsgMask(Qt::MouseButton btn);
    static vsg::ButtonMask qtButtonsToVsgMask(Qt::MouseButtons btns);
    static vsg::KeyModifier qtModifiersToVsg(Qt::KeyboardModifiers mods);
    static vsg::KeySymbol qtKeyToVsg(int qtKey);
    /// @}

signals:
    void gridVisibleChanged();
    void deleteRequested();
    void renderRequested();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    visualization::SceneManager*     sceneMgr_   = nullptr;
    visualization::CameraController* cameraCtrl_ = nullptr;
    visualization::SceneFurniture*   furniture_   = nullptr;

    vsg::ButtonMask currentMask_ = vsg::BUTTON_MASK_OFF;
    bool dirty_ = true;
};

} // namespace ui
