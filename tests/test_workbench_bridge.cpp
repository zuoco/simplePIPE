// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "app/CadWorkbench.h"
#include "app/DependencyGraph.h"
#include "app/Document.h"
#include "app/SelectionManager.h"
#include "app/WorkbenchManager.h"
#include "command/CommandContext.h"
#include "command/CommandStack.h"
#include "command/SetPropertyCommand.h"
#include "model/PipePoint.h"
#include "ui/AppController.h"
#include "ui/WorkbenchController.h"

#include <QCoreApplication>
#include <QSignalSpy>

#include <gp_Pnt.hxx>

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
    command::CommandStack commandStack;
    app::SelectionManager selection;

    ui::AppController controller(document, commandStack, selection);
    QSignalSpy spy(&controller, &ui::AppController::selectionChanged);

    const foundation::UUID id = foundation::UUID::generate();
    EXPECT_TRUE(selection.select(id));
    EXPECT_EQ(controller.selectedCount(), 1);
    EXPECT_EQ(controller.selectedUuids().size(), 1);
    EXPECT_GE(spy.count(), 1);

    controller.clearSelection();
    EXPECT_EQ(controller.selectedCount(), 0);
}

// ============================================================
// T7: AppController undo/redo 与 CommandStack 集成测试
// ============================================================

TEST(AppControllerTest, CanUndoRedoReflectsCommandStack)
{
    app::Document document;
    command::CommandStack commandStack;
    app::SelectionManager selection;

    ui::AppController controller(document, commandStack, selection);
    EXPECT_FALSE(controller.canUndo());
    EXPECT_FALSE(controller.canRedo());

    // 手动执行一个命令
    auto point = std::make_shared<model::PipePoint>("P1", model::PipePointType::Run, gp_Pnt(0, 0, 0));
    document.addObject(point);

    auto cmd = command::SetPropertyCommand::createWithOldValue(point->id(), "x", 0.0, 100.0);
    command::CommandContext ctx{&document, nullptr, nullptr};
    commandStack.execute(std::move(cmd), ctx);

    EXPECT_TRUE(controller.canUndo());
    EXPECT_FALSE(controller.canRedo());

    commandStack.undo(ctx);
    EXPECT_FALSE(controller.canUndo());
    EXPECT_TRUE(controller.canRedo());

    commandStack.redo(ctx);
    EXPECT_TRUE(controller.canUndo());
    EXPECT_FALSE(controller.canRedo());
}

TEST(AppControllerTest, StackChangedEmitsTransactionStateChanged)
{
    app::Document document;
    command::CommandStack commandStack;
    app::SelectionManager selection;

    ui::AppController controller(document, commandStack, selection);
    QSignalSpy spy(&controller, &ui::AppController::transactionStateChanged);

    auto point = std::make_shared<model::PipePoint>("P1", model::PipePointType::Run, gp_Pnt(0, 0, 0));
    document.addObject(point);

    auto cmd = command::SetPropertyCommand::createWithOldValue(point->id(), "x", 0.0, 50.0);
    command::CommandContext ctx{&document, nullptr, nullptr};
    commandStack.execute(std::move(cmd), ctx);

    // stackChanged 信号触发后，controller 应 emit transactionStateChanged
    EXPECT_GE(spy.count(), 1);
}

TEST(AppControllerTest, UndoRedoMethodsWork)
{
    // 此测试需要 Application 单例，因为 AppController::undo() 调用 Application::instance()
    // 所以我们测试通过 CommandStack 直接操作的等效行为
    app::Document document;
    command::CommandStack commandStack;
    app::SelectionManager selection;

    ui::AppController controller(document, commandStack, selection);

    auto point = std::make_shared<model::PipePoint>("P1", model::PipePointType::Run, gp_Pnt(10, 20, 30));
    document.addObject(point);

    auto cmd = command::SetPropertyCommand::createWithOldValue(point->id(), "x", 10.0, 999.0);
    command::CommandContext ctx{&document, nullptr, nullptr};
    commandStack.execute(std::move(cmd), ctx);

    EXPECT_DOUBLE_EQ(point->position().X(), 999.0);
    EXPECT_TRUE(controller.canUndo());

    // undo 通过 CommandStack（AppController::undo 需要 Application 单例）
    commandStack.undo(ctx);
    EXPECT_DOUBLE_EQ(point->position().X(), 10.0);
    EXPECT_TRUE(controller.canRedo());
    EXPECT_FALSE(controller.canUndo());
}
