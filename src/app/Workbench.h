// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/Document.h"

#include <string>
#include <vector>

namespace app {

enum class ViewportType {
    Vsg,
    Vtk
};

struct ToolbarAction {
    std::string id;
    std::string label;
};

class Workbench {
public:
    virtual ~Workbench() = default;

    virtual std::string name() const = 0;
    virtual void activate(Document& document) = 0;
    virtual void deactivate(Document& document) = 0;
    virtual std::vector<ToolbarAction> toolbarActions() const = 0;
    virtual std::vector<std::string> panelIds() const = 0;
    virtual ViewportType viewportType() const = 0;
};

} // namespace app
