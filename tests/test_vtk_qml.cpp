#include <gtest/gtest.h>

#include "vtk-visualization/VtkViewport.h"
#include "vtk-visualization/VtkSceneManager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickItem>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>

class VtkQmlTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        static int argc = 1;
        static char* argv[] = {(char*)"test_vtk_qml"};
        if (!QCoreApplication::instance()) {
            new QGuiApplication(argc, argv);
        }
        qmlRegisterType<vtk_vis::VtkViewport>("PipeCAD", 1, 0, "VtkViewportElement");
    }

    void SetUp() override {
        sceneManager_ = new vtk_vis::VtkSceneManager();
        engine_ = new QQmlApplicationEngine();
        
        QByteArray qml = R"(
            import QtQuick
            import PipeCAD 1.0

            Window {
                id: window
                width: 800
                height: 600
                visible: true

                VtkViewportElement {
                    id: viewport
                    objectName: "viewport"
                    anchors.fill: parent
                    focus: true
                }
            }
        )";

        engine_->loadData(qml);
        
        auto rootObjects = engine_->rootObjects();
        ASSERT_FALSE(rootObjects.isEmpty());
        
        window_ = qobject_cast<QQuickWindow*>(rootObjects.first());
        ASSERT_NE(window_, nullptr);
        
        viewport_ = window_->findChild<vtk_vis::VtkViewport*>("viewport");
        ASSERT_NE(viewport_, nullptr);
        
        viewport_->setSceneManager(sceneManager_);
    }

    void TearDown() override {
        delete engine_;
        delete sceneManager_;
    }

    vtk_vis::VtkSceneManager* sceneManager_ = nullptr;
    QQmlApplicationEngine* engine_ = nullptr;
    QQuickWindow* window_ = nullptr;
    vtk_vis::VtkViewport* viewport_ = nullptr;
};

TEST_F(VtkQmlTest, ElementCreatedAndAttached) {
    ASSERT_NE(viewport_->sceneManager(), nullptr);
    ASSERT_NE(viewport_->interactor(), nullptr);
    ASSERT_NE(viewport_->renderWindow(), nullptr);
}

// Just test standard event routing (no actual VTK render check required here, QQuickFramebufferObject handles rest)
TEST_F(VtkQmlTest, MouseEventRouting) {
    auto pos = QPointF(100.0, 100.0);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMouseEvent pressEvent(QEvent::MouseButtonPress, pos, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#else
    QMouseEvent pressEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#endif
    
    // Process event directly
    QCoreApplication::sendEvent(viewport_, &pressEvent);
    
    // Given VTK's interactor receives the event, we just assure no crash
    SUCCEED();
}

