#include "app/WorkbenchManager.h"

#include <algorithm>

namespace app {

WorkbenchManager::WorkbenchManager(Document& document)
    : document_(document)
{
}

void WorkbenchManager::registerWorkbench(std::unique_ptr<Workbench> workbench)
{
    if (!workbench) {
        return;
    }

    const std::string key = workbench->name();
    if (key.empty()) {
        return;
    }

    workbenches_[key] = std::move(workbench);
}

bool WorkbenchManager::switchWorkbench(const std::string& name)
{
    auto it = workbenches_.find(name);
    if (it == workbenches_.end()) {
        return false;
    }

    Workbench* next = it->second.get();
    if (next == active_) {
        return true;
    }

    if (active_) {
        active_->deactivate(document_);
    }

    active_ = next;
    active_->activate(document_);

    if (onWorkbenchChanged_) {
        onWorkbenchChanged_(active_);
    }

    return true;
}

Workbench* WorkbenchManager::activeWorkbench() const
{
    return active_;
}

std::vector<std::string> WorkbenchManager::workbenchNames() const
{
    std::vector<std::string> names;
    names.reserve(workbenches_.size());

    for (const auto& [name, wb] : workbenches_) {
        (void)wb;
        names.push_back(name);
    }

    std::sort(names.begin(), names.end());
    return names;
}

void WorkbenchManager::setWorkbenchChangedCallback(WorkbenchChangedCallback cb)
{
    onWorkbenchChanged_ = std::move(cb);
}

} // namespace app
