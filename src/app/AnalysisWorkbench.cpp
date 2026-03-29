#include "app/AnalysisWorkbench.h"

namespace app {

std::string AnalysisWorkbench::name() const {
    return "Analysis";
}

void AnalysisWorkbench::activate(Document&) {
    // AnalysisWorkbench 激活时默认使用 Solid 渲染模式
    // 未来可在此恢复上次退出时的渲染模式
}

void AnalysisWorkbench::deactivate(Document&) {
    // 切出时不改变 renderMode_，以备下次切回时恢复
}

std::vector<ToolbarAction> AnalysisWorkbench::toolbarActions() const {
    return {
        {"toggle-render-mode", "切换渲染模式"},
        {"add-load",           "添加载荷"},
        {"manage-loadcase",    "工况管理"},
        {"run-analysis",       "执行分析"},
        {"show-results",       "查看结果"}
    };
}

std::vector<std::string> AnalysisWorkbench::panelIds() const {
    return {
        "AnalysisTree",
        "VtkViewport",
        "LoadTable",
        "LoadCaseTable",
        "PropertyPanel"
    };
}

ViewportType AnalysisWorkbench::viewportType() const {
    return ViewportType::Vtk;
}

void AnalysisWorkbench::setRenderMode(RenderMode mode) {
    renderMode_ = mode;
}

RenderMode AnalysisWorkbench::renderMode() const {
    return renderMode_;
}

} // namespace app
