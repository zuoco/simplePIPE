// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/DocumentObject.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace model {

/// 容器对象：管理子对象集合
class ContainerObject : public DocumentObject {
public:
    explicit ContainerObject(const std::string& name = "")
        : DocumentObject(name) {}

    void addChild(std::shared_ptr<DocumentObject> child) {
        children_.push_back(std::move(child));
        changed.emit();
    }

    bool removeChild(const foundation::UUID& childId) {
        auto it = std::find_if(children_.begin(), children_.end(),
            [&](const auto& c) { return c->id() == childId; });
        if (it != children_.end()) {
            children_.erase(it);
            changed.emit();
            return true;
        }
        return false;
    }

    DocumentObject* findChild(const foundation::UUID& childId) const {
        for (const auto& c : children_) {
            if (c->id() == childId) return c.get();
        }
        return nullptr;
    }

    const std::vector<std::shared_ptr<DocumentObject>>& children() const {
        return children_;
    }

    std::size_t childCount() const { return children_.size(); }

protected:
    std::vector<std::shared_ptr<DocumentObject>> children_;
};

} // namespace model
