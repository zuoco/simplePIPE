#include <gtest/gtest.h>

#include "app/CadWorkbench.h"
#include "app/DependencyGraph.h"
#include "app/Document.h"
#include "app/SelectionManager.h"
#include "app/WorkbenchManager.h"
#include "ui/AppController.h"
#include "ui/WorkbenchController.h"

#include <QCoreApplication>
#include <QSignalSpy>

static QCoreApplication* app_ = nullptr;

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    app_ = new QCoreApplication(argc, argv);
    const int result = RUN_ALL_TESTS();
    delete app_;
    return result;
}

TEST(WorkbenchManagerTest, SwitchWorkbenchTriggersCallback)
{
    app::Document document;
    app::WorkbenchManager manager(document);

    int callbackCount = 0;
    manager.setWorkbenchChangedCallback([&callbackCount](const app::Workbench* wb) {
        if (wb) {
            ++callbackCount;
        }
    });

    manager.registerWorkbench(std::make_unique<app::CadWorkbench>());

    EXPECT_TRUE(manager.switchWorkbench("CAD"));
    EXPECT_EQ(callbackCount, 1);
    EXPECT_NE(manager.activeWorkbench(), nullptr);
    EXPECT_EQ(manager.activeWorkbench()->name(), "CAD");
}

TEST(SelectionManagerTest, SelectDeselectAndClear)
{
    app::SelectionManager selection;
    const foundation::UUID a = foundation::UUID::generate();
    const foundation::UUID b = foundation::UUID::generate();

    int callbackCount = 0;
    selection.setSelectionChangedCallback([&callbackCount](const std::vector<foundation::UUID>& ids) {
        (void)ids;
        ++callbackCount;
    });

    EXPECT_TRUE(selection.select(a));
    EXPECT_TRUE(selection.select(b));
    EXPECT_EQ(selection.size(), 2u);
    EXPECT_TRUE(selection.isSelected(a));

    EXPECT_TRUE(selection.deselect(a));
    EXPECT_FALSE(selection.isSelected(a));
    EXPECT_EQ(selection.size(), 1u);

    selection.clear();
    EXPECT_EQ(selection.size(), 0u);
    EXPECT_GE(callbackCount, 4);
}

TEST(WorkbenchControllerTest, ExposesActivePanelsAndSignals)
{
    app::Document document;
    app::WorkbenchManager manager(document);
    manager.registerWorkbench(std::make_unique<app::CadWorkbench>());

    ui::WorkbenchController controller(manager);
    QSignalSpy spy(&controller, &ui::WorkbenchController::activeWorkbenchChanged);

    EXPECT_TRUE(controller.switchWorkbench("CAD"));
    EXPECT_EQ(controller.activeWorkbench().toStdString(), "CAD");
    EXPECT_FALSE(controller.activePanels().isEmpty());
    EXPECT_FALSE(controller.toolbarActions().isEmpty());
    EXPECT_GE(spy.count(), 1);
}

TEST(AppControllerTest, MirrorsSelectionState)
{
    app::Document document;
    app::DependencyGraph graph;
    app::TransactionManager tm(document, graph);
    app::SelectionManager selection;

    ui::AppController controller(document, tm, selection);
    QSignalSpy spy(&controller, &ui::AppController::selectionChanged);

    const foundation::UUID id = foundation::UUID::generate();
    EXPECT_TRUE(selection.select(id));
    EXPECT_EQ(controller.selectedCount(), 1);
    EXPECT_EQ(controller.selectedUuids().size(), 1);
    EXPECT_GE(spy.count(), 1);

    controller.clearSelection();
    EXPECT_EQ(controller.selectedCount(), 0);
}
