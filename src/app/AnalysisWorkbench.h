#pragma once

#include "app/Workbench.h"

namespace app {

/// 渲染模式（仅 AnalysisWorkbench 使用）
enum class RenderMode {
    Solid, ///< 实体模式：3D 管件外观
    Beam   ///< 线条模式：管系中心线 + 节点
};

class AnalysisWorkbench : public Workbench {
public:
    std::string name() const override;
    void activate(Document& document) override;
    void deactivate(Document& document) override;
    std::vector<ToolbarAction> toolbarActions() const override;
    std::vector<std::string> panelIds() const override;
    ViewportType viewportType() const override;

    void setRenderMode(RenderMode mode);
    RenderMode renderMode() const;

private:
    RenderMode renderMode_ = RenderMode::Solid;
};

} // namespace app
