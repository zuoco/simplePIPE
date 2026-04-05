// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "app/DependencyGraph.h"

#include "app/Document.h"
#include "model/Load.h"
#include "model/LoadCase.h"
#include "model/LoadCombination.h"

#include <algorithm>

namespace app {

void DependencyGraph::cacheUUID(const foundation::UUID& id) {
    uuidCache_.emplace(id.toString(), id);
}

void DependencyGraph::addDependency(const foundation::UUID& dependent,
                                    const foundation::UUID& dependsOn) {
    cacheUUID(dependent);
    cacheUUID(dependsOn);

    const std::string dep = dependent.toString();
    const std::string src = dependsOn.toString();

    // src 变化后通知 dep
    dependents_[src].insert(dep);
    // dep 依赖 src
    dependencies_[dep].insert(src);
}

void DependencyGraph::removeDependency(const foundation::UUID& dependent,
                                       const foundation::UUID& dependsOn) {
    const std::string dep = dependent.toString();
    const std::string src = dependsOn.toString();

    if (auto it = dependents_.find(src); it != dependents_.end()) {
        it->second.erase(dep);
        if (it->second.empty()) dependents_.erase(it);
    }
    if (auto it = dependencies_.find(dep); it != dependencies_.end()) {
        it->second.erase(src);
        if (it->second.empty()) dependencies_.erase(it);
    }
}

void DependencyGraph::removeObject(const foundation::UUID& id) {
    const std::string key = id.toString();

    // 移除以该对象为来源的所有出边
    if (auto it = dependents_.find(key); it != dependents_.end()) {
        for (const auto& dep : it->second) {
            if (auto dit = dependencies_.find(dep); dit != dependencies_.end()) {
                dit->second.erase(key);
                if (dit->second.empty()) dependencies_.erase(dit);
            }
        }
        dependents_.erase(it);
    }

    // 移除以该对象为目标的所有入边
    if (auto it = dependencies_.find(key); it != dependencies_.end()) {
        for (const auto& src : it->second) {
            if (auto sit = dependents_.find(src); sit != dependents_.end()) {
                sit->second.erase(key);
                if (sit->second.empty()) dependents_.erase(sit);
            }
        }
        dependencies_.erase(it);
    }

    dirty_.erase(key);
    uuidCache_.erase(key);
}

void DependencyGraph::markDirty(const foundation::UUID& id) {
    cacheUUID(id);

    std::vector<std::string> stack;
    stack.push_back(id.toString());

    while (!stack.empty()) {
        std::string cur = std::move(stack.back());
        stack.pop_back();

        if (dirty_.count(cur)) continue;
        dirty_.insert(cur);

        // 沿依赖边传播
        if (auto it = dependents_.find(cur); it != dependents_.end()) {
            for (const auto& dep : it->second) {
                if (!dirty_.count(dep)) {
                    stack.push_back(dep);
                }
            }
        }
    }
}

void DependencyGraph::clearDirty() {
    dirty_.clear();
}

bool DependencyGraph::isDirty(const foundation::UUID& id) const {
    return dirty_.count(id.toString()) > 0;
}

void DependencyGraph::rebuildLoadDependencyChain(const Document& document) {
    auto loads = document.findByType<model::Load>();
    auto loadCases = document.findByType<model::LoadCase>();
    auto loadCombinations = document.findByType<model::LoadCombination>();

    std::unordered_set<std::string> loadIds;
    loadIds.reserve(loads.size());
    for (const auto* load : loads) {
        cacheUUID(load->id());
        loadIds.insert(load->id().toString());
    }

    std::unordered_set<std::string> loadCaseIds;
    loadCaseIds.reserve(loadCases.size());
    for (const auto* loadCase : loadCases) {
        cacheUUID(loadCase->id());
        loadCaseIds.insert(loadCase->id().toString());
    }

    for (const auto* combination : loadCombinations) {
        cacheUUID(combination->id());
    }

    for (const auto* loadCase : loadCases) {
        for (const auto& entry : loadCase->entries()) {
            if (loadIds.count(entry.loadId.toString()) > 0) {
                addDependency(loadCase->id(), entry.loadId);
            }
        }
    }

    for (const auto* combination : loadCombinations) {
        for (const auto& caseEntry : combination->caseEntries()) {
            if (loadCaseIds.count(caseEntry.caseId.toString()) > 0) {
                addDependency(combination->id(), caseEntry.caseId);
            }
        }
    }
}

void DependencyGraph::topoSort(const std::string& node,
                                std::unordered_set<std::string>& visited,
                                std::vector<std::string>& order) const {
    if (visited.count(node)) return;
    visited.insert(node);

    // 先处理该节点依赖的节点（dependencies_[node]），只处理同样是脏的
    if (auto it = dependencies_.find(node); it != dependencies_.end()) {
        for (const auto& dep : it->second) {
            if (dirty_.count(dep)) {
                topoSort(dep, visited, order);
            }
        }
    }
    order.push_back(node);
}

std::vector<foundation::UUID> DependencyGraph::collectDirty() const {
    std::unordered_set<std::string> visited;
    std::vector<std::string> order;

    for (const auto& key : dirty_) {
        topoSort(key, visited, order);
    }

    std::vector<foundation::UUID> result;
    result.reserve(order.size());
    for (const auto& key : order) {
        if (dirty_.count(key)) {
            if (auto it = uuidCache_.find(key); it != uuidCache_.end()) {
                result.push_back(it->second);
            }
        }
    }
    return result;
}

std::vector<foundation::UUID> DependencyGraph::directDependencies(const foundation::UUID& id) const {
    std::vector<foundation::UUID> result;
    auto it = dependencies_.find(id.toString());
    if (it == dependencies_.end()) {
        return result;
    }

    result.reserve(it->second.size());
    for (const auto& key : it->second) {
        auto uuidIt = uuidCache_.find(key);
        if (uuidIt != uuidCache_.end()) {
            result.push_back(uuidIt->second);
        }
    }

    std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.toString() < rhs.toString();
    });
    return result;
}

std::vector<foundation::UUID> DependencyGraph::directDependents(const foundation::UUID& id) const {
    std::vector<foundation::UUID> result;
    auto it = dependents_.find(id.toString());
    if (it == dependents_.end()) {
        return result;
    }

    result.reserve(it->second.size());
    for (const auto& key : it->second) {
        auto uuidIt = uuidCache_.find(key);
        if (uuidIt != uuidCache_.end()) {
            result.push_back(uuidIt->second);
        }
    }

    std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.toString() < rhs.toString();
    });
    return result;
}

std::vector<foundation::UUID> DependencyGraph::allNodes() const {
    std::vector<foundation::UUID> result;
    result.reserve(uuidCache_.size());
    for (const auto& [key, uuid] : uuidCache_) {
        result.push_back(uuid);
    }

    std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.toString() < rhs.toString();
    });
    return result;
}

} // namespace app
