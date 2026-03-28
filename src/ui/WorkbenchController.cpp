#include "ui/WorkbenchController.h"

namespace ui {

WorkbenchController::WorkbenchController(app::WorkbenchManager& manager, QObject* parent)
    : QObject(parent)
    , manager_(manager)
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
    const bool changed = manager_.switchWorkbench(name.toStdString());
    if (changed) {
        emit activeWorkbenchChanged();
    }
    return changed;
}

} // namespace ui
