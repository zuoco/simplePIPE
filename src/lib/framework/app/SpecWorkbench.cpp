// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "app/SpecWorkbench.h"

namespace app {

std::string SpecWorkbench::name() const
{
    return "Specification";
}

void SpecWorkbench::activate(Document& document)
{
    (void)document;
}

void SpecWorkbench::deactivate(Document& document)
{
    (void)document;
}

std::vector<ToolbarAction> SpecWorkbench::toolbarActions() const
{
    return {
        {"new-spec",        "新建规格"},
        {"import-code",     "导入代号"},
        {"add-material",    "添加材料"},
        {"add-component",   "添加元件"},
        {"validate",        "验证"}
    };
}

std::vector<std::string> SpecWorkbench::panelIds() const
{
    return {
        "SpecTree",
        "MaterialTable",
        "ComponentTable",
        "PropertyPanel"
    };
}

ViewportType SpecWorkbench::viewportType() const
{
    return ViewportType::Vsg;
}

} // namespace app
