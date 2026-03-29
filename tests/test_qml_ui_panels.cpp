#include <gtest/gtest.h>

#include "app/DesignWorkbench.h"
#include "app/DependencyGraph.h"
#include "app/Document.h"
#include "app/SelectionManager.h"
#include "app/TransactionManager.h"
#include "app/WorkbenchManager.h"
#include "ui/AppController.h"
#include "ui/VsgQuickItem.h"
#include "ui/WorkbenchController.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <filesystem>

static QGuiApplication* g_app = nullptr;

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    g_app = new QGuiApplication(argc, argv);

    const int result = RUN_ALL_TESTS();
    delete g_app;
    g_app = nullptr;
    return result;
}

namespace {

std::filesystem::path projectRootFromThisFile()
{
    // tests/test_qml_ui_panels.cpp -> project root
    const std::filesystem::path here = std::filesystem::path(__FILE__).parent_path();
    return here.parent_path();
}

} // namespace

TEST(QmlUiPanelsTest, MainWindowLoadsAndCorePanelsExist)
{
    qmlRegisterType<ui::VsgQuickItem>("PipeCAD", 1, 0, "VsgViewport");

    app::Document document;
    document.setName("UiPanelSmoke");

    app::DependencyGraph graph;
    app::TransactionManager transactionManager(document, graph);
    app::SelectionManager selectionManager;

    app::WorkbenchManager workbenchManager(document);
    workbenchManager.registerWorkbench(std::make_unique<app::DesignWorkbench>());
    ASSERT_TRUE(workbenchManager.switchWorkbench("Design"));

    ui::AppController appController(document, transactionManager, selectionManager);
    ui::WorkbenchController workbenchController(workbenchManager);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appController", &appController);
    engine.rootContext()->setContextProperty("workbenchController", &workbenchController);

    const std::filesystem::path qmlPath = projectRootFromThisFile() / "ui" / "main.qml";
    engine.load(QUrl::fromLocalFile(QString::fromStdString(qmlPath.string())));

    ASSERT_FALSE(engine.rootObjects().isEmpty());

    QObject* root = engine.rootObjects().constFirst();
    ASSERT_NE(root, nullptr);

    EXPECT_NE(root->findChild<QObject*>("viewport3d"), nullptr);
    EXPECT_NE(root->findChild<QObject*>("designTreePanel"), nullptr);
    EXPECT_NE(root->findChild<QObject*>("parameterPanel"), nullptr);
    EXPECT_NE(root->findChild<QObject*>("pipePointTablePanel"), nullptr);
    EXPECT_NE(root->findChild<QObject*>("propertyPanel"), nullptr);
    EXPECT_NE(root->findChild<QObject*>("statusBarPanel"), nullptr);
}
