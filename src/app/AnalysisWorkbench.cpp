#include "app/AnalysisWorkbench.h"

namespace app {

std::string AnalysisWorkbench::name() const {
    return "Analysis";
}

void AnalysisWorkbench::activate(Document&) {}

void AnalysisWorkbench::deactivate(Document&) {}

std::vector<ToolbarAction> AnalysisWorkbench::toolbarActions() const {
    return {};
}

std::vector<std::string> AnalysisWorkbench::panelIds() const {
    return {
        "AnalysisTree",
        "VtkViewport",
        "LoadTable",
        "PropertyPanel"
    };
}

ViewportType AnalysisWorkbench::viewportType() const {
    return ViewportType::Vtk;
}

} // namespace app
