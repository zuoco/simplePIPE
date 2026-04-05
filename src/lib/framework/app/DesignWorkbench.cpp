// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "app/DesignWorkbench.h"

namespace app {

std::string DesignWorkbench::name() const
{
    return "Design";
}

void DesignWorkbench::activate(Document& document)
{
    (void)document;
}

void DesignWorkbench::deactivate(Document& document)
{
    (void)document;
}

std::vector<ToolbarAction> DesignWorkbench::toolbarActions() const
{
    return {
        {"new-segment",  "新建段"},
        {"add-point",    "添加管点"},
        {"measure",      "测量"},
        {"export-step",  "STEP导出"}
    };
}

std::vector<std::string> DesignWorkbench::panelIds() const
{
    return {
        "DesignTree",
        "Viewport3D",
        "ComponentToolStrip",
        "ParameterPanel"
    };
}

ViewportType DesignWorkbench::viewportType() const
{
    return ViewportType::Vsg;
}

} // namespace app
