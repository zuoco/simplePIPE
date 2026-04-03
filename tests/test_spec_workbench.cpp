// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "app/SpecWorkbench.h"
#include "app/Document.h"

#include <gtest/gtest.h>

using namespace app;

// 辅助：创建最小 Document
static Document makeDocument()
{
    return Document{};
}

TEST(SpecWorkbench, NameIsSpecification)
{
    SpecWorkbench wb;
    EXPECT_EQ(wb.name(), "Specification");
}

TEST(SpecWorkbench, ToolbarActionsCount)
{
    SpecWorkbench wb;
    auto actions = wb.toolbarActions();
    EXPECT_EQ(actions.size(), 5u);
}

TEST(SpecWorkbench, ToolbarActionIds)
{
    SpecWorkbench wb;
    auto actions = wb.toolbarActions();
    ASSERT_EQ(actions.size(), 5u);
    EXPECT_EQ(actions[0].id, "new-spec");
    EXPECT_EQ(actions[1].id, "import-code");
    EXPECT_EQ(actions[2].id, "add-material");
    EXPECT_EQ(actions[3].id, "add-component");
    EXPECT_EQ(actions[4].id, "validate");
}

TEST(SpecWorkbench, ToolbarActionLabels)
{
    SpecWorkbench wb;
    auto actions = wb.toolbarActions();
    for (const auto& a : actions) {
        EXPECT_FALSE(a.label.empty()) << "Action " << a.id << " has empty label";
    }
}

TEST(SpecWorkbench, PanelIdsCount)
{
    SpecWorkbench wb;
    auto panels = wb.panelIds();
    EXPECT_EQ(panels.size(), 4u);
}

TEST(SpecWorkbench, PanelIdsContent)
{
    SpecWorkbench wb;
    auto panels = wb.panelIds();
    ASSERT_EQ(panels.size(), 4u);
    EXPECT_EQ(panels[0], "SpecTree");
    EXPECT_EQ(panels[1], "MaterialTable");
    EXPECT_EQ(panels[2], "ComponentTable");
    EXPECT_EQ(panels[3], "PropertyPanel");
}

TEST(SpecWorkbench, ViewportTypeIsVsg)
{
    SpecWorkbench wb;
    EXPECT_EQ(wb.viewportType(), ViewportType::Vsg);
}

TEST(SpecWorkbench, ActivateDeactivateLifecycle)
{
    SpecWorkbench wb;
    Document doc = makeDocument();

    // activate/deactivate 应无异常、无崩溃
    EXPECT_NO_THROW(wb.activate(doc));
    EXPECT_NO_THROW(wb.deactivate(doc));
}

TEST(SpecWorkbench, ActivateDeactivateToggle)
{
    SpecWorkbench wb;
    Document doc = makeDocument();

    // 多次切换也应稳定
    wb.activate(doc);
    wb.deactivate(doc);
    wb.activate(doc);
    wb.deactivate(doc);
    SUCCEED();
}
