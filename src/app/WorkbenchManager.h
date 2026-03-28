#pragma once

#include "app/Workbench.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace app {

class WorkbenchManager {
public:
    using WorkbenchChangedCallback = std::function<void(const Workbench*)>;

    explicit WorkbenchManager(Document& document);

    void registerWorkbench(std::unique_ptr<Workbench> workbench);
    bool switchWorkbench(const std::string& name);

    Workbench* activeWorkbench() const;
    std::vector<std::string> workbenchNames() const;

    void setWorkbenchChangedCallback(WorkbenchChangedCallback cb);

private:
    Document& document_;
    std::unordered_map<std::string, std::unique_ptr<Workbench>> workbenches_;
    Workbench* active_ = nullptr;
    WorkbenchChangedCallback onWorkbenchChanged_;
};

} // namespace app
