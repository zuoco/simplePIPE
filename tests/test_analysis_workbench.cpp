// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "app/AnalysisWorkbench.h"
#include "app/Document.h"
#include "app/WorkbenchManager.h"
#include "app/DesignWorkbench.h"
#include "model/Load.h"
#include "model/LoadCase.h"
#include "model/LoadCombination.h"
#include "model/DeadWeightLoad.h"
#include "model/ThermalLoad.h"
#include "model/PressureLoad.h"

#include <gtest/gtest.h>
#include <memory>

using namespace app;

// 辅助：创建最小 Document
static Document makeDocument()
{
    return Document{};
}

// === 基本属性测试 ===

TEST(AnalysisWorkbench, NameIsAnalysis)
{
    AnalysisWorkbench wb;
    EXPECT_EQ(wb.name(), "Analysis");
}

TEST(AnalysisWorkbench, ViewportTypeIsVtk)
{
    AnalysisWorkbench wb;
    EXPECT_EQ(wb.viewportType(), ViewportType::Vtk);
}

// === 工具栏动作测试 ===

TEST(AnalysisWorkbench, ToolbarActionsCount)
{
    AnalysisWorkbench wb;
    auto actions = wb.toolbarActions();
    EXPECT_EQ(actions.size(), 5u);
}

TEST(AnalysisWorkbench, ToolbarActionIds)
{
    AnalysisWorkbench wb;
    auto actions = wb.toolbarActions();
    ASSERT_EQ(actions.size(), 5u);
    EXPECT_EQ(actions[0].id, "toggle-render-mode");
    EXPECT_EQ(actions[1].id, "add-load");
    EXPECT_EQ(actions[2].id, "manage-loadcase");
    EXPECT_EQ(actions[3].id, "run-analysis");
    EXPECT_EQ(actions[4].id, "show-results");
}

TEST(AnalysisWorkbench, ToolbarActionLabels)
{
    AnalysisWorkbench wb;
    auto actions = wb.toolbarActions();
    for (const auto& a : actions) {
        EXPECT_FALSE(a.label.empty()) << "Action " << a.id << " has empty label";
    }
}

TEST(AnalysisWorkbench, ToolbarContainsToggleRenderMode)
{
    AnalysisWorkbench wb;
    auto actions = wb.toolbarActions();
    bool found = false;
    for (const auto& a : actions) {
        if (a.id == "toggle-render-mode") { found = true; break; }
    }
    EXPECT_TRUE(found) << "toggle-render-mode not found in toolbar actions";
}

TEST(AnalysisWorkbench, ToolbarContainsAddLoad)
{
    AnalysisWorkbench wb;
    auto actions = wb.toolbarActions();
    bool found = false;
    for (const auto& a : actions) {
        if (a.id == "add-load") { found = true; break; }
    }
    EXPECT_TRUE(found) << "add-load not found in toolbar actions";
}

TEST(AnalysisWorkbench, ToolbarContainsManageLoadcase)
{
    AnalysisWorkbench wb;
    auto actions = wb.toolbarActions();
    bool found = false;
    for (const auto& a : actions) {
        if (a.id == "manage-loadcase") { found = true; break; }
    }
    EXPECT_TRUE(found) << "manage-loadcase not found in toolbar actions";
}

// === 面板 ID 测试 ===

TEST(AnalysisWorkbench, PanelIdsCount)
{
    AnalysisWorkbench wb;
    auto panels = wb.panelIds();
    EXPECT_EQ(panels.size(), 5u);
}

TEST(AnalysisWorkbench, PanelIdsContent)
{
    AnalysisWorkbench wb;
    auto panels = wb.panelIds();
    ASSERT_EQ(panels.size(), 5u);
    EXPECT_EQ(panels[0], "AnalysisTree");
    EXPECT_EQ(panels[1], "VtkViewport");
    EXPECT_EQ(panels[2], "LoadTable");
    EXPECT_EQ(panels[3], "LoadCaseTable");
    EXPECT_EQ(panels[4], "PropertyPanel");
}

TEST(AnalysisWorkbench, PanelContainsVtkViewport)
{
    AnalysisWorkbench wb;
    auto panels = wb.panelIds();
    bool found = false;
    for (const auto& p : panels) {
        if (p == "VtkViewport") { found = true; break; }
    }
    EXPECT_TRUE(found) << "VtkViewport not in panel list";
}

// === 渲染模式测试 ===

TEST(AnalysisWorkbench, DefaultRenderModeIsSolid)
{
    AnalysisWorkbench wb;
    EXPECT_EQ(wb.renderMode(), RenderMode::Solid);
}

TEST(AnalysisWorkbench, SetRenderModeToBeam)
{
    AnalysisWorkbench wb;
    wb.setRenderMode(RenderMode::Beam);
    EXPECT_EQ(wb.renderMode(), RenderMode::Beam);
}

TEST(AnalysisWorkbench, SetRenderModeBackToSolid)
{
    AnalysisWorkbench wb;
    wb.setRenderMode(RenderMode::Beam);
    wb.setRenderMode(RenderMode::Solid);
    EXPECT_EQ(wb.renderMode(), RenderMode::Solid);
}

TEST(AnalysisWorkbench, RenderModeToggle)
{
    AnalysisWorkbench wb;
    EXPECT_EQ(wb.renderMode(), RenderMode::Solid);
    wb.setRenderMode(RenderMode::Beam);
    EXPECT_EQ(wb.renderMode(), RenderMode::Beam);
    wb.setRenderMode(RenderMode::Solid);
    EXPECT_EQ(wb.renderMode(), RenderMode::Solid);
}

// === 生命周期测试 ===

TEST(AnalysisWorkbench, ActivateDeactivateLifecycle)
{
    AnalysisWorkbench wb;
    Document doc = makeDocument();

    EXPECT_NO_THROW(wb.activate(doc));
    EXPECT_NO_THROW(wb.deactivate(doc));
}

TEST(AnalysisWorkbench, ActivateDeactivateToggle)
{
    AnalysisWorkbench wb;
    Document doc = makeDocument();

    wb.activate(doc);
    wb.deactivate(doc);
    wb.activate(doc);
    wb.deactivate(doc);
    SUCCEED();
}

TEST(AnalysisWorkbench, RenderModePreservedAcrossDeactivate)
{
    AnalysisWorkbench wb;
    Document doc = makeDocument();

    wb.activate(doc);
    wb.setRenderMode(RenderMode::Beam);
    wb.deactivate(doc);

    // 重新激活后 renderMode 应保留
    wb.activate(doc);
    EXPECT_EQ(wb.renderMode(), RenderMode::Beam);
}

// === 工作台管理器集成测试 ===

TEST(AnalysisWorkbench, RegisterAndActivateViaManager)
{
    Document doc = makeDocument();
    WorkbenchManager manager(doc);

    manager.registerWorkbench(std::make_unique<AnalysisWorkbench>());
    bool switched = manager.switchWorkbench("Analysis");

    EXPECT_TRUE(switched);
    ASSERT_NE(manager.activeWorkbench(), nullptr);
    EXPECT_EQ(manager.activeWorkbench()->name(), "Analysis");
}

TEST(AnalysisWorkbench, SwitchDesignToAnalysisNoCrash)
{
    Document doc = makeDocument();
    WorkbenchManager manager(doc);

    manager.registerWorkbench(std::make_unique<DesignWorkbench>());
    manager.registerWorkbench(std::make_unique<AnalysisWorkbench>());

    EXPECT_TRUE(manager.switchWorkbench("Design"));
    EXPECT_EQ(manager.activeWorkbench()->name(), "Design");

    EXPECT_TRUE(manager.switchWorkbench("Analysis"));
    EXPECT_EQ(manager.activeWorkbench()->name(), "Analysis");
    EXPECT_EQ(manager.activeWorkbench()->viewportType(), ViewportType::Vtk);
}

TEST(AnalysisWorkbench, SwitchAnalysisToDesignNoCrash)
{
    Document doc = makeDocument();
    WorkbenchManager manager(doc);

    manager.registerWorkbench(std::make_unique<DesignWorkbench>());
    manager.registerWorkbench(std::make_unique<AnalysisWorkbench>());

    manager.switchWorkbench("Analysis");
    EXPECT_TRUE(manager.switchWorkbench("Design"));
    EXPECT_EQ(manager.activeWorkbench()->name(), "Design");
    EXPECT_EQ(manager.activeWorkbench()->viewportType(), ViewportType::Vsg);
}

TEST(AnalysisWorkbench, RepeatedSwitchingStability)
{
    Document doc = makeDocument();
    WorkbenchManager manager(doc);

    manager.registerWorkbench(std::make_unique<DesignWorkbench>());
    manager.registerWorkbench(std::make_unique<AnalysisWorkbench>());

    for (int i = 0; i < 10; ++i) {
        manager.switchWorkbench("Design");
        manager.switchWorkbench("Analysis");
    }

    EXPECT_EQ(manager.activeWorkbench()->name(), "Analysis");
    SUCCEED();
}

// === 载荷数据模型集成测试 ===

TEST(AnalysisWorkbench, LoadDataModelIntegration)
{
    // 验证 AnalysisWorkbench 涉及的数据模型可正常创建和使用
    auto deadWeight = std::make_shared<model::DeadWeightLoad>("自重");
    auto thermal = std::make_shared<model::ThermalLoad>("热载荷-操作");
    thermal->setInstallTemp(20.0);
    thermal->setOperatingTemp(150.0);
    auto pressure = std::make_shared<model::PressureLoad>("内压");
    pressure->setPressure(2.5);

    EXPECT_EQ(deadWeight->loadType(), "DeadWeight");
    EXPECT_EQ(thermal->loadType(), "Thermal");
    EXPECT_EQ(pressure->loadType(), "Pressure");
    EXPECT_DOUBLE_EQ(thermal->operatingTemp(), 150.0);
    EXPECT_DOUBLE_EQ(pressure->pressure(), 2.5);
}

TEST(AnalysisWorkbench, LoadCaseCreation)
{
    auto deadWeight = std::make_shared<model::DeadWeightLoad>("自重");
    auto pressure = std::make_shared<model::PressureLoad>("内压");
    pressure->setPressure(2.5);

    model::LoadCase sus("SUS");
    sus.addEntry({deadWeight->id(), 1.0});
    sus.addEntry({pressure->id(), 1.0});

    EXPECT_EQ(sus.caseName(), "SUS");
    EXPECT_EQ(sus.entries().size(), 2u);
}

TEST(AnalysisWorkbench, LoadCombinationCreation)
{
    auto deadWeight = std::make_shared<model::DeadWeightLoad>("自重");
    auto pressure = std::make_shared<model::PressureLoad>("内压");

    auto caseSus = std::make_shared<model::LoadCase>("W");
    caseSus->addEntry({deadWeight->id(), 1.0});

    auto caseP1 = std::make_shared<model::LoadCase>("P1");
    caseP1->addEntry({pressure->id(), 1.0});

    model::LoadCombination combo("SUS", model::StressCategory::Sustained, model::CombineMethod::Algebraic);
    combo.addCaseEntry({caseSus->id(), 1.0});
    combo.addCaseEntry({caseP1->id(), 1.0});

    EXPECT_EQ(combo.name(), "SUS");
    EXPECT_EQ(combo.category(), model::StressCategory::Sustained);
    EXPECT_EQ(combo.method(), model::CombineMethod::Algebraic);
    EXPECT_EQ(combo.caseEntries().size(), 2u);
}
