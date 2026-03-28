#include "app/SelectionManager.h"

#include <algorithm>

namespace app {

bool SelectionManager::select(const foundation::UUID& id)
{
    if (id.isNull() || isSelected(id)) {
        return false;
    }

    selected_.push_back(id);
    notifySelectionChanged();
    return true;
}

bool SelectionManager::deselect(const foundation::UUID& id)
{
    auto it = std::find(selected_.begin(), selected_.end(), id);
    if (it == selected_.end()) {
        return false;
    }

    selected_.erase(it);
    notifySelectionChanged();
    return true;
}

void SelectionManager::clear()
{
    if (selected_.empty()) {
        return;
    }

    selected_.clear();
    notifySelectionChanged();
}

bool SelectionManager::isSelected(const foundation::UUID& id) const
{
    return std::find(selected_.begin(), selected_.end(), id) != selected_.end();
}

const std::vector<foundation::UUID>& SelectionManager::selected() const
{
    return selected_;
}

std::size_t SelectionManager::size() const
{
    return selected_.size();
}

std::size_t SelectionManager::addSelectionChangedCallback(SelectionChangedCallback cb)
{
    selectionChangedCallbacks_.push_back(std::move(cb));
    return selectionChangedCallbacks_.size();
}

void SelectionManager::setSelectionChangedCallback(SelectionChangedCallback cb)
{
    selectionChangedCallbacks_.clear();
    selectionChangedCallbacks_.push_back(std::move(cb));
}

void SelectionManager::notifySelectionChanged()
{
    for (auto& callback : selectionChangedCallbacks_) {
        if (callback) {
            callback(selected_);
        }
    }
}

} // namespace app
