#include "app/DesignWorkbench.h"
#include "app/Document.h"

#include <gtest/gtest.h>

using namespace app;

// 辅助：创建最小 Document
static Document makeDocument()
{
    return Document{};
}

TEST(DesignWorkbench, NameIsDesign)
{
    DesignWorkbench wb;
    EXPECT_EQ(wb.name(), "Design");
}

TEST(DesignWorkbench, ToolbarActionsCount)
{
    DesignWorkbench wb;
    auto actions = wb.toolbarActions();
    EXPECT_EQ(actions.size(), 4u);
}

TEST(DesignWorkbench, ToolbarActionIds)
{
    DesignWorkbench wb;
    auto actions = wb.toolbarActions();
    ASSERT_EQ(actions.size(), 4u);
    EXPECT_EQ(actions[0].id, "new-segment");
    EXPECT_EQ(actions[1].id, "add-point");
    EXPECT_EQ(actions[2].id, "measure");
    EXPECT_EQ(actions[3].id, "export-step");
}

TEST(DesignWorkbench, ToolbarActionLabels)
{
    DesignWorkbench wb;
    auto actions = wb.toolbarActions();
    for (const auto& a : actions) {
        EXPECT_FALSE(a.label.empty()) << "Action " << a.id << " has empty label";
    }
}

TEST(DesignWorkbench, PanelIdsCount)
{
    DesignWorkbench wb;
    auto panels = wb.panelIds();
    EXPECT_EQ(panels.size(), 4u);
}

TEST(DesignWorkbench, PanelIdsContent)
{
    DesignWorkbench wb;
    auto panels = wb.panelIds();
    ASSERT_EQ(panels.size(), 4u);
    EXPECT_EQ(panels[0], "DesignTree");
    EXPECT_EQ(panels[1], "Viewport3D");
    EXPECT_EQ(panels[2], "ComponentToolStrip");
    EXPECT_EQ(panels[3], "ParameterPanel");
}

TEST(DesignWorkbench, ViewportTypeIsVsg)
{
    DesignWorkbench wb;
    EXPECT_EQ(wb.viewportType(), ViewportType::Vsg);
}

TEST(DesignWorkbench, ActivateDeactivateLifecycle)
{
    DesignWorkbench wb;
    Document doc = makeDocument();

    // activate/deactivate 应无异常、无崩溃
    EXPECT_NO_THROW(wb.activate(doc));
    EXPECT_NO_THROW(wb.deactivate(doc));
}

TEST(DesignWorkbench, ActivateDeactivateToggle)
{
    DesignWorkbench wb;
    Document doc = makeDocument();

    // 多次切换也应稳定
    wb.activate(doc);
    wb.deactivate(doc);
    wb.activate(doc);
    wb.deactivate(doc);
    SUCCEED();
}
