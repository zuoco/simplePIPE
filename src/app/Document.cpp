// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "app/Document.h"

namespace app {

Document::Document() : name_("Untitled") {}

void Document::addObject(std::shared_ptr<model::DocumentObject> obj) {
    if (!obj) return;
    const std::string key = obj->id().toString();
    objects_.emplace(key, std::move(obj));
}

bool Document::removeObject(const foundation::UUID& id) {
    return objects_.erase(id.toString()) > 0;
}

model::DocumentObject* Document::findObject(const foundation::UUID& id) const {
    auto it = objects_.find(id.toString());
    return (it != objects_.end()) ? it->second.get() : nullptr;
}

std::shared_ptr<model::DocumentObject> Document::findObjectShared(const foundation::UUID& id) const {
    auto it = objects_.find(id.toString());
    return (it != objects_.end()) ? it->second : nullptr;
}

std::shared_ptr<model::DocumentObject> Document::findByName(const std::string& name) const {
    for (auto& [id, obj] : objects_) {
        if (obj->name() == name)
            return obj;
    }
    return nullptr;
}

std::vector<model::PipePoint*> Document::allPipePoints() const {
    return findByType<model::PipePoint>();
}

std::vector<model::Segment*> Document::allSegments() const {
    return findByType<model::Segment>();
}

std::vector<model::PipeSpec*> Document::allPipeSpecs() const {
    return findByType<model::PipeSpec>();
}

void Document::forEach(const std::function<void(model::DocumentObject&)>& fn) const {
    for (auto& [id, obj] : objects_) {
        fn(*obj);
    }
}

} // namespace app
