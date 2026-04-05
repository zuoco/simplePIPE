// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/Workbench.h"

namespace app {

class CadWorkbench : public Workbench {
public:
    std::string name() const override;
    void activate(Document& document) override;
    void deactivate(Document& document) override;
    std::vector<ToolbarAction> toolbarActions() const override;
    std::vector<std::string> panelIds() const override;
    ViewportType viewportType() const override;
};

} // namespace app
