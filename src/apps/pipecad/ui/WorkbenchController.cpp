// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "ui/WorkbenchController.h"

namespace ui {

WorkbenchController::WorkbenchController(app::WorkbenchManager& manager, visualization::ViewManager* viewManager, QObject* parent)
    : QObject(parent)
    , manager_(manager)
    , viewManager_(viewManager)
{
    manager_.setWorkbenchChangedCallback([this](const app::Workbench*) {
        emit activeWorkbenchChanged();
    });
}

QString WorkbenchController::activeWorkbench() const
{
    const app::Workbench* wb = manager_.activeWorkbench();
    return wb ? QString::fromStdString(wb->name()) : QString{};
}

QStringList WorkbenchController::activePanels() const
{
    QStringList panels;
    const app::Workbench* wb = manager_.activeWorkbench();
    if (!wb) {
        return panels;
    }

    for (const auto& panel : wb->panelIds()) {
        panels.push_back(QString::fromStdString(panel));
    }
    return panels;
}

QStringList WorkbenchController::toolbarActions() const
{
    QStringList actions;
    const app::Workbench* wb = manager_.activeWorkbench();
    if (!wb) {
        return actions;
    }

    for (const auto& action : wb->toolbarActions()) {
        actions.push_back(QString::fromStdString(action.label));
    }
    return actions;
}

QStringList WorkbenchController::workbenchNames() const
{
    QStringList names;
    for (const auto& name : manager_.workbenchNames()) {
        names.push_back(QString::fromStdString(name));
    }
    return names;
}

bool WorkbenchController::switchWorkbench(const QString& name)
{
    std::string oldName = manager_.activeWorkbench() ? manager_.activeWorkbench()->name() : "";

    if (viewManager_ && !oldName.empty()) {
        viewManager_->saveViewState(oldName);
    }

    const bool changed = manager_.switchWorkbench(name.toStdString());
    if (changed) {
        if (viewManager_) {
            const app::Workbench* wb = manager_.activeWorkbench();
            if (wb && wb->viewportType() == app::ViewportType::Vtk) {
                viewManager_->setActiveViewport(visualization::ViewManager::ActiveViewport::VTK);
            } else {
                viewManager_->setActiveViewport(visualization::ViewManager::ActiveViewport::VSG);
            }
            viewManager_->restoreViewState(name.toStdString());
        }

        emit activeWorkbenchChanged();
    }
    return changed;
}

void WorkbenchController::notifyViewportLoaded(QObject* viewportObj) {
    emit viewportLoaded(viewportObj);
}

} // namespace ui
