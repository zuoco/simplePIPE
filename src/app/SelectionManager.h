#pragma once

#include "foundation/Types.h"

#include <functional>
#include <vector>

namespace app {

class SelectionManager {
public:
    using SelectionChangedCallback = std::function<void(const std::vector<foundation::UUID>&)>;

    bool select(const foundation::UUID& id);
    bool deselect(const foundation::UUID& id);
    void clear();

    bool isSelected(const foundation::UUID& id) const;
    const std::vector<foundation::UUID>& selected() const;
    std::size_t size() const;

    void setSelectionChangedCallback(SelectionChangedCallback cb);

private:
    std::vector<foundation::UUID> selected_;
    SelectionChangedCallback onSelectionChanged_;

    void notifySelectionChanged();
};

} // namespace app
