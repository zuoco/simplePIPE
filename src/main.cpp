#include "app/CadWorkbench.h"
#include "app/DependencyGraph.h"
#include "app/Document.h"
#include "app/SelectionManager.h"
#include "app/TransactionManager.h"
#include "app/WorkbenchManager.h"
#include "engine/RecomputeEngine.h"
#include "ui/AppController.h"
#include "ui/VsgQuickItem.h"
#include "ui/WorkbenchController.h"
#include "visualization/CameraController.h"
#include "visualization/ComponentNode.h"
#include "visualization/OcctToVsg.h"
#include "visualization/SceneFurniture.h"
#include "visualization/SceneManager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/Trackball.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/state/ViewportState.h>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<ui::VsgQuickItem>("PipeCAD", 1, 0, "VsgViewport");

    app::Document document;
    document.setName("Untitled");

    app::DependencyGraph dependencyGraph;
    app::TransactionManager transactionManager(document, dependencyGraph);
    app::SelectionManager selectionManager;
    app::WorkbenchManager workbenchManager(document);
    workbenchManager.registerWorkbench(std::make_unique<app::CadWorkbench>());
    workbenchManager.switchWorkbench("CAD");

    engine::RecomputeEngine recomputeEngine(document, dependencyGraph);

    visualization::SceneManager sceneManager;
    visualization::SceneFurniture sceneFurniture;
    sceneManager.root()->addChild(sceneFurniture.axisNode());
    sceneManager.root()->addChild(sceneFurniture.gridSwitch());

    auto perspective = vsg::Perspective::create(60.0, 16.0 / 9.0, 0.1, 100000.0);
    auto lookAt = vsg::LookAt::create(
        vsg::dvec3(1000.0, -2000.0, 1500.0),
        vsg::dvec3(0.0, 0.0, 0.0),
        vsg::dvec3(0.0, 0.0, 1.0));
    auto viewport = vsg::ViewportState::create(0, 0, 1280, 720);
    auto camera = vsg::Camera::create(perspective, lookAt, viewport);
    auto trackball = vsg::Trackball::create(camera);
    visualization::CameraController cameraController(camera, trackball);

    recomputeEngine.setSceneUpdateCallback([&sceneManager](const std::string& uuid,
                                                           const TopoDS_Shape& shape) {
        auto geometry = visualization::toVsgGeometry(shape);
        auto node = visualization::createComponentNode(geometry);
        if (sceneManager.hasNode(uuid)) {
            sceneManager.updateNode(uuid, node);
        } else {
            sceneManager.addNode(uuid, node);
        }
    });

    transactionManager.setRecomputeCallback([&recomputeEngine](const std::vector<foundation::UUID>& dirtyIds) {
        recomputeEngine.recompute(dirtyIds);
    });

    ui::AppController appController(document, transactionManager, selectionManager);
    ui::WorkbenchController workbenchController(workbenchManager);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appController", &appController);
    engine.rootContext()->setContextProperty("workbenchController", &workbenchController);

#ifdef PIPECAD_QML_MAIN
    const QUrl mainQmlUrl = QUrl::fromLocalFile(QStringLiteral(PIPECAD_QML_MAIN));
#else
    const QUrl mainQmlUrl = QUrl::fromLocalFile(QStringLiteral("ui/main.qml"));
#endif

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [&](QObject* obj, const QUrl& objUrl) {
                         if (!obj || objUrl != mainQmlUrl) {
                             return;
                         }

                         auto* viewportItem = obj->findChild<ui::VsgQuickItem*>("viewport3d");
                         if (!viewportItem) {
                             return;
                         }

                         viewportItem->setSceneManager(&sceneManager);
                         viewportItem->setCameraController(&cameraController);
                         viewportItem->setSceneFurniture(&sceneFurniture);
                     },
                     Qt::QueuedConnection);

    engine.load(mainQmlUrl);
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
