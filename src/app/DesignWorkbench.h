#pragma once

#include "app/Workbench.h"

namespace app {

class DesignWorkbench : public Workbench {
public:
    std::string name() const override;
    void activate(Document& document) override;
    void deactivate(Document& document) override;
    std::vector<ToolbarAction> toolbarActions() const override;
    std::vector<std::string> panelIds() const override;
    ViewportType viewportType() const override;
};

} // namespace app
