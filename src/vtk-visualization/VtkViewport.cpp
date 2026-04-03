// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "vtk-visualization/VtkViewport.h"
#include "vtk-visualization/VtkSceneManager.h"

#include <QOpenGLFramebufferObject>
#include <QQuickWindow>
#include <QMouseEvent>

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>

namespace vtk_vis {

class VtkFboRenderer : public QQuickFramebufferObject::Renderer {
public:
    VtkFboRenderer(VtkViewport* viewport) 
        : viewport_(viewport) 
        , renderWindow_(viewport->renderWindow())
    {
        // Must tell generic window not to use offscreen buffers
        // as QQuickFbo uses its own FBO
        renderWindow_->SetReadyForRendering(true);
        
    }

    QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::Depth);
        format.setSamples(4);
        return new QOpenGLFramebufferObject(size, format);
    }

    void synchronize(QQuickFramebufferObject* item) override {
        // Handle window size changes
        auto size = item->size() * item->window()->devicePixelRatio();
        if (size != size_) {
            size_ = size;
            renderWindow_->SetSize(size_.width(), size_.height());
            if (auto interactor = viewport_->interactor()) {
                interactor->UpdateSize(size_.width(), size_.height());
                // Make sure interactor size logic sets up things correctly
            }
        }
    }

    void render() override {
        if (!renderWindow_) return;

        renderWindow_->Render();
    }

private:
    VtkViewport* viewport_;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow_;
    QSizeF size_;
};

VtkViewport::VtkViewport(QQuickItem* parent)
    : QQuickFramebufferObject(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setFlag(ItemAcceptsInputMethod, true);

    renderWindow_ = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    interactor_ = vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();
    
    renderWindow_->SetInteractor(interactor_);

    auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    interactor_->SetInteractorStyle(style);
}

VtkViewport::~VtkViewport() = default;

QQuickFramebufferObject::Renderer* VtkViewport::createRenderer() const {
    return new VtkFboRenderer(const_cast<VtkViewport*>(this));
}

void VtkViewport::setSceneManager(VtkSceneManager* manager) {
    if (sceneManager_ == manager) return;
    
    // Remove old renderer
    if (sceneManager_ && sceneManager_->renderer()) {
        renderWindow_->RemoveRenderer(sceneManager_->renderer());
    }
    
    sceneManager_ = manager;
    
    // Add new renderer
    if (sceneManager_ && sceneManager_->renderer()) {
        renderWindow_->AddRenderer(sceneManager_->renderer());
    }
    
    update(); // Request QQuickFbo update
}

VtkSceneManager* VtkViewport::sceneManager() const {
    return sceneManager_;
}

vtkGenericRenderWindowInteractor* VtkViewport::interactor() const {
    return interactor_;
}

vtkGenericOpenGLRenderWindow* VtkViewport::renderWindow() const {
    return renderWindow_;
}

// Map Qt modifiers bitmask to VTK (ctrl, shift)
// Return pair<int, int> -> <ctrl, shift>
static std::pair<int, int> getModifiers(Qt::KeyboardModifiers modifiers) {
    return {
        (modifiers & Qt::ControlModifier) ? 1 : 0,
        (modifiers & Qt::ShiftModifier) ? 1 : 0
    };
}

void VtkViewport::mousePressEvent(QMouseEvent* event) {
    auto [ctrl, shift] = getModifiers(event->modifiers());
    
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    int y = height() - event->position().y();
    interactor_->SetEventInformationFlipY(event->position().x(), event->position().y(), ctrl, shift);
#else
    int y = height() - event->y();
    interactor_->SetEventInformationFlipY(event->x(), event->y(), ctrl, shift);
#endif

    interactor_->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);

    if (event->button() == Qt::LeftButton) {
        interactor_->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
    } else if (event->button() == Qt::RightButton) {
        interactor_->InvokeEvent(vtkCommand::RightButtonPressEvent, nullptr);
    } else if (event->button() == Qt::MiddleButton) {
        interactor_->InvokeEvent(vtkCommand::MiddleButtonPressEvent, nullptr);
    }
    update();
}

void VtkViewport::mouseReleaseEvent(QMouseEvent* event) {
    auto [ctrl, shift] = getModifiers(event->modifiers());
    
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    interactor_->SetEventInformationFlipY(event->position().x(), event->position().y(), ctrl, shift);
#else
    interactor_->SetEventInformationFlipY(event->x(), event->y(), ctrl, shift);
#endif

    if (event->button() == Qt::LeftButton) {
        interactor_->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
    } else if (event->button() == Qt::RightButton) {
        interactor_->InvokeEvent(vtkCommand::RightButtonReleaseEvent, nullptr);
    } else if (event->button() == Qt::MiddleButton) {
        interactor_->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, nullptr);
    }
    update();
}

void VtkViewport::mouseMoveEvent(QMouseEvent* event) {
    auto [ctrl, shift] = getModifiers(event->modifiers());
    
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    interactor_->SetEventInformationFlipY(event->position().x(), event->position().y(), ctrl, shift);
#else
    interactor_->SetEventInformationFlipY(event->x(), event->y(), ctrl, shift);
#endif

    interactor_->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);
    update();
}

void VtkViewport::wheelEvent(QWheelEvent* event) {
    auto [ctrl, shift] = getModifiers(event->modifiers());

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    interactor_->SetEventInformationFlipY(event->position().x(), event->position().y(), ctrl, shift);
#else
    interactor_->SetEventInformationFlipY(event->x(), event->y(), ctrl, shift);
#endif

    if (event->angleDelta().y() > 0) {
        interactor_->InvokeEvent(vtkCommand::MouseWheelForwardEvent, nullptr);
    } else {
        interactor_->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, nullptr);
    }
    update();
}

} // namespace vtk_vis
