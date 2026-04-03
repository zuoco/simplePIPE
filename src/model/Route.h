// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/ContainerObject.h"
#include "model/Segment.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace model {

/// 管路：段的树状集合
class Route : public ContainerObject {
public:
    explicit Route(const std::string& name = "")
        : ContainerObject(name) {}

    void addSegment(std::shared_ptr<Segment> seg) {
        segments_.push_back(seg);
        addChild(seg);
    }

    bool removeSegment(const foundation::UUID& segId) {
        auto it = std::find_if(segments_.begin(), segments_.end(),
            [&](const auto& s) { return s->id() == segId; });
        if (it != segments_.end()) {
            segments_.erase(it);
            removeChild(segId);
            return true;
        }
        return false;
    }

    Segment* segmentAt(std::size_t index) const {
        return index < segments_.size() ? segments_[index].get() : nullptr;
    }

    const std::vector<std::shared_ptr<Segment>>& segments() const {
        return segments_;
    }

    std::size_t segmentCount() const { return segments_.size(); }

private:
    std::vector<std::shared_ptr<Segment>> segments_;
};

} // namespace model
