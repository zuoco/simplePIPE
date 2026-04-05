// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "app/Application.h"
#include "app/CadWorkbench.h"
#include "app/DesignWorkbench.h"
#include "app/SpecWorkbench.h"
#include "app/AnalysisWorkbench.h"
#include "app/DependencyGraph.h"
#include "app/Document.h"
#include "app/SelectionManager.h"
#include "app/WorkbenchManager.h"
#include "command/CommandStack.h"
#include "command/CommandRegistry.h"
#include "engine/RecomputeEngine.h"
#include "ui/AppController.h"
#include "ui/VsgQuickItem.h"
#include "ui/WorkbenchController.h"
#include "visualization/CameraController.h"
#include "visualization/ComponentNode.h"
#include "visualization/OcctToVsg.h"
#include "visualization/SceneFurniture.h"
#include "visualization/SceneManager.h"
#include "visualization/ViewManager.h"

#include "vtk-visualization/VtkViewport.h"
#include "vtk-visualization/VtkSceneManager.h"

// 异步管线基础设施（T71）
#include "lib/runtime/task/TaskQueue.h"
#include "lib/runtime/task/ResultChannel.h"
#include "lib/runtime/task/SceneUpdateAdapter.h"
#include "lib/runtime/app/DocumentSnapshot.h"

#include <algorithm>
#include <utility>
#include <vector>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/Trackball.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/state/ViewportState.h>

int main(int argc, char* argv[])
{
    QGuiApplication qtApp(argc, argv);

    qmlRegisterType<ui::VsgQuickItem>("PipeCAD", 1, 0, "VsgViewport");
    qmlRegisterType<vtk_vis::VtkViewport>("PipeCAD", 1, 0, "VtkViewportElement");

    // 初始化中央单例（线程安全）
    app::Application::init();
    auto& appSingleton = app::Application::instance();

    auto& document           = appSingleton.document();
    auto& dependencyGraph    = appSingleton.dependencyGraph();
    auto& selectionManager   = appSingleton.selectionManager();
    auto& workbenchManager   = appSingleton.workbenchManager();
    auto& commandStack       = appSingleton.commandStack();
    auto& commandRegistry    = appSingleton.commandRegistry();

    document.setName("Untitled");
    workbenchManager.registerWorkbench(std::make_unique<app::CadWorkbench>());
    workbenchManager.registerWorkbench(std::make_unique<app::DesignWorkbench>());
    workbenchManager.registerWorkbench(std::make_unique<app::SpecWorkbench>());
    workbenchManager.registerWorkbench(std::make_unique<app::AnalysisWorkbench>());
    workbenchManager.switchWorkbench("Design");

    // 注册内置命令工厂（SetProperty, BatchSetProperty, Macro）
    commandRegistry.registerBuiltins();

    // ---- 异步重算基础设施（T71）----
    // WorkerGroup：后台几何推导线程池（2 线程）
    task::WorkerGroup workers(2);

    // ResultChannel：后台→主线程的结果回投队列（内置互斥锁，线程安全）
    task::ResultChannel resultChannel;

    // SceneUpdateAdapter：主线程消费 ResultChannel，按版本校验后执行 applyFn
    task::SceneUpdateAdapter sceneAdapter(resultChannel,
        [&document]() { return document.currentVersion(); });

    engine::RecomputeEngine recomputeEngine(document, dependencyGraph);

    visualization::SceneManager sceneManager;
    visualization::SceneFurniture sceneFurniture;
    
    // ViewManager 注入
    visualization::ViewManager viewManager;
    vtk_vis::VtkSceneManager vtkScene;
    viewManager.setVtkComponents(&vtkScene);
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
    viewManager.setVsgComponents(&sceneManager, &cameraController, &sceneFurniture);

    // 定义场景更新回调（同步/异步路径共用，applyFn 始终在主线程执行）
    engine::RecomputeEngine::SceneUpdateCallback sceneUpdateFn =
        [&sceneManager](const std::string& uuid, const TopoDS_Shape& shape) {
            auto geometry = visualization::toVsgGeometry(shape);
            auto node = visualization::createComponentNode(geometry);
            if (sceneManager.hasNode(uuid)) {
                sceneManager.updateNode(uuid, node);
            } else {
                sceneManager.addNode(uuid, node);
            }
        };

    // 场景更新回调：同步 recompute() 路径直接调用
    recomputeEngine.setSceneUpdateCallback(sceneUpdateFn);

    // 注入异步管线二元组（T72）
    // asyncFn：构建快照 → 清除脏集 → 每个脏对象独立提交后台任务（并行批量重算）
    // 每个后台任务完成：BRep 推导 + ShapeMesher(网格化) + VSG 节点构建
    // applyFn 在主线程仅执行 addNode/updateNode（零几何运算，不阻塞 UI）
    recomputeEngine.enableAsyncMode(
        [&document, &dependencyGraph, &workers, &resultChannel,
         &sceneManager]() {
            // ① 构建只读快照（主线程，T70 同步协议第1步）
            //    用 shared_ptr 共享快照，多个后台任务无需拷贝
            auto snap = std::make_shared<app::DocumentSnapshot>(
                app::makeDocumentSnapshot(document, dependencyGraph));
            // ② 清除脏标记（主线程，T70 同步协议第2步）
            dependencyGraph.clearDirty();
            if (snap->dependencyGraph.dirtyIds.empty()) return;
            const auto version = snap->version;
            // ③ 每个脏对象独立提交后台任务（T72：并行批量重算）
            //    WorkerGroup 多线程同时推进，互不阻塞
            for (const auto& dirtyId : snap->dependencyGraph.dirtyIds) {
                workers.submit(
                    [snap, dirtyId, version, &resultChannel, &sceneManager]
                    (const task::CancellationToken& token) {
                        if (token.isCancellationRequested()) return;
                        const auto* ppSnap = snap->findPipePoint(dirtyId);
                        if (!ppSnap) return;
                        // 在冻结 segments 中查找前后邻居坐标
                        gp_Pnt prevPt = ppSnap->position;
                        gp_Pnt nextPt = ppSnap->position;
                        for (const auto& seg : snap->segments) {
                            auto it = std::find(seg.pointIds.begin(),
                                                seg.pointIds.end(), dirtyId);
                            if (it == seg.pointIds.end()) continue;
                            if (it != seg.pointIds.begin()) {
                                if (const auto* prev = snap->findPipePoint(*std::prev(it)))
                                    prevPt = prev->position;
                            }
                            if (auto nit = std::next(it); nit != seg.pointIds.end()) {
                                if (const auto* next = snap->findPipePoint(*nit))
                                    nextPt = next->position;
                            }
                            break;
                        }
                        // 查找 PipeSpec 快照
                        const app::PipeSpecSnapshot* specSnap = nullptr;
                        if (ppSnap->pipeSpecId.has_value())
                            specSnap = snap->findPipeSpec(*ppSnap->pipeSpecId);
                        // 后台 BRep 几何推导（只读快照，线程安全）
                        TopoDS_Shape shape = engine::GeometryDeriver::deriveFromSnapshot(
                            prevPt, *ppSnap, specSnap, nextPt);
                        if (shape.IsNull()) return;
                        // 后台网格化（T72 核心：ShapeMesher 不再阻塞主线程）
                        auto geometry = visualization::toVsgGeometry(shape);
                        auto node     = visualization::createComponentNode(geometry);
                        // ④ 将预构建的 VSG 节点回投到主线程（零几何运算）
                        const std::string uuid = ppSnap->id.toString();
                        resultChannel.post(version,
                            [uuid, node, &sceneManager]() {
                                if (sceneManager.hasNode(uuid))
                                    sceneManager.updateNode(uuid, node);
                                else
                                    sceneManager.addNode(uuid, node);
                            });
                    });
            }
        },
        // drainFn：主线程事件循环消费新鲜结果
        [&sceneAdapter]() { return sceneAdapter.drain(); }
    );

    // ---- CommandStack 信号连线：脏传播 + 异步场景重算（T72）----
    auto recomputeHandler = [&](const std::vector<foundation::UUID>& affectedIds) {
        for (const auto& id : affectedIds) {
            dependencyGraph.markDirty(id);
        }
        // asyncRecompute() 内部遵循 T70 快照窗口协议：
        //   collectDirty → makeDocumentSnapshot → clearDirty → submitTask
        recomputeEngine.asyncRecompute();
    };

    commandStack.commandCompleted.connect(recomputeHandler);
    commandStack.commandUndone.connect(recomputeHandler);
    commandStack.commandRedone.connect(recomputeHandler);

    // ---- sceneRemoveRequested：命令删除对象后通知场景移除节点 ----
    commandStack.sceneRemoveRequested.connect([&sceneManager](const std::string& uuid) {
        sceneManager.removeNode(uuid);
    });

    // ---- 定时消费后台几何结果（每 16ms ≈ 60fps，主线程事件循环驱动）----
    QTimer drainTimer;
    QObject::connect(&drainTimer, &QTimer::timeout, &qtApp, [&]() {
        recomputeEngine.drainResults();
    });
    drainTimer.start(16);

    ui::AppController appController(document, commandStack, selectionManager);
    ui::WorkbenchController workbenchController(workbenchManager, &viewManager);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appController", &appController);
    engine.rootContext()->setContextProperty("workbenchController", &workbenchController);

#ifdef PIPECAD_QML_MAIN
    const QUrl mainQmlUrl = QUrl::fromLocalFile(QStringLiteral(PIPECAD_QML_MAIN));
#else
    const QUrl mainQmlUrl = QUrl::fromLocalFile(QStringLiteral("ui/main.qml"));
#endif

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &qtApp,
                     [&](QObject* obj, const QUrl& objUrl) {
                         if (!obj || objUrl != mainQmlUrl) {
                             return;
                         }
                     },
                     Qt::QueuedConnection);

    QObject::connect(&workbenchController, &ui::WorkbenchController::viewportLoaded, &qtApp,
                     [&](QObject* obj) {
                         auto* vsgItem = qobject_cast<ui::VsgQuickItem*>(obj);
                         if (vsgItem) {
                             vsgItem->setSceneManager(&sceneManager);
                             vsgItem->setCameraController(&cameraController);
                             vsgItem->setSceneFurniture(&sceneFurniture);
                         } else {
                             auto* vtkItem = qobject_cast<vtk_vis::VtkViewport*>(obj);
                             if (vtkItem) {
                                 vtkItem->setSceneManager(&vtkScene);
                             }
                         }
                     });

    engine.load(mainQmlUrl);
    if (engine.rootObjects().isEmpty()) {
        // 关闭 WorkerGroup，避免后台线程在静态析构阶段访问已销毁对象
        workers.shutdown();
        resultChannel.discard();
        return -1;
    }

    int ret = qtApp.exec();

    // 优雅关闭：先丢弃挂起结果，再关闭线程池
    drainTimer.stop();
    resultChannel.discard();
    workers.shutdown();

    return ret;
}
