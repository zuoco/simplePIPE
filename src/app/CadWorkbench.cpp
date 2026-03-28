#include "app/CadWorkbench.h"

namespace app {

std::string CadWorkbench::name() const
{
    return "CAD";
}

void CadWorkbench::activate(Document& document)
{
    (void)document;
}

void CadWorkbench::deactivate(Document& document)
{
    (void)document;
}

std::vector<ToolbarAction> CadWorkbench::toolbarActions() const
{
    return {
        {"new-segment", "新建段"},
        {"add-point", "添加管点"},
        {"add-accessory", "添加附属"},
        {"measure", "测量"},
        {"export-step", "STEP导出"}
    };
}

std::vector<std::string> CadWorkbench::panelIds() const
{
    return {
        "StructureTree",
        "Viewport3D",
        "PipePointTable",
        "PropertyPanel",
        "PipeSpecEditor"
    };
}

ViewportType CadWorkbench::viewportType() const
{
    return ViewportType::Vsg;
}

} // namespace app
