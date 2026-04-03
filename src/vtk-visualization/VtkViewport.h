// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <QQuickFramebufferObject>
#include <vtkSmartPointer.h>

class vtkGenericRenderWindowInteractor;
class vtkGenericOpenGLRenderWindow;
class vtkRenderer;

namespace vtk_vis {

class VtkSceneManager;

class VtkViewport : public QQuickFramebufferObject {
    Q_OBJECT

public:
    explicit VtkViewport(QQuickItem* parent = nullptr);
    ~VtkViewport() override;

    Renderer* createRenderer() const override;

    void setSceneManager(VtkSceneManager* manager);
    VtkSceneManager* sceneManager() const;

    vtkGenericRenderWindowInteractor* interactor() const;
    vtkGenericOpenGLRenderWindow* renderWindow() const;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    VtkSceneManager* sceneManager_ = nullptr;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow_;
    vtkSmartPointer<vtkGenericRenderWindowInteractor> interactor_;
};

} // namespace vtk_vis
