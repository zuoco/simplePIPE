// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "app/Document.h"

namespace app {

Document::Document() : name_("Untitled") {}

void Document::attachObjectObserver(const std::shared_ptr<model::DocumentObject>& obj) {
    if (!obj) {
        return;
    }

    const std::string key = obj->id().toString();
    detachObjectObserver(obj->id());
    objectChangeSlots_[key] = obj->changed.connect([this]() {
        bumpVersion();
    });
}

void Document::detachObjectObserver(const foundation::UUID& id) {
    const std::string key = id.toString();
    auto slotIt = objectChangeSlots_.find(key);
    auto objIt = objects_.find(key);

    if (slotIt != objectChangeSlots_.end()) {
        if (objIt != objects_.end() && objIt->second) {
            objIt->second->changed.disconnect(slotIt->second);
        }
        objectChangeSlots_.erase(slotIt);
    }
}

void Document::addObject(std::shared_ptr<model::DocumentObject> obj) {
    if (!obj) return;
    const std::string key = obj->id().toString();
    if (objects_.count(key) > 0) {
        return;
    }

    attachObjectObserver(obj);
    objects_.emplace(key, std::move(obj));
    bumpVersion();
}

bool Document::removeObject(const foundation::UUID& id) {
    const std::string key = id.toString();
    auto it = objects_.find(key);
    if (it == objects_.end()) {
        return false;
    }

    detachObjectObserver(id);
    objects_.erase(it);
    bumpVersion();
    return true;
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

void Document::setName(const std::string& name) {
    if (name_ == name) {
        return;
    }

    name_ = name;
    bumpVersion();
}

void Document::bumpVersion() {
    ++version_;
}

} // namespace app
