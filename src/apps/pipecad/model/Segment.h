// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/ContainerObject.h"
#include "model/PipePoint.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace model {

/// 管段：有序管点列表
class Segment : public ContainerObject {
public:
    explicit Segment(const std::string& name = "")
        : ContainerObject(name) {}

    /// 在末尾追加管点
    void addPoint(std::shared_ptr<PipePoint> point) {
        points_.push_back(point);
        addChild(point);
    }

    /// 在指定位置插入管点
    void insertPoint(std::size_t index, std::shared_ptr<PipePoint> point) {
        if (index > points_.size()) index = points_.size();
        points_.insert(points_.begin() + static_cast<std::ptrdiff_t>(index), point);
        addChild(point);
    }

    /// 按 UUID 移除管点
    bool removePoint(const foundation::UUID& pointId) {
        auto it = std::find_if(points_.begin(), points_.end(),
            [&](const auto& p) { return p->id() == pointId; });
        if (it != points_.end()) {
            points_.erase(it);
            removeChild(pointId);
            return true;
        }
        return false;
    }

    PipePoint* pointAt(std::size_t index) const {
        return index < points_.size() ? points_[index].get() : nullptr;
    }

    const std::vector<std::shared_ptr<PipePoint>>& points() const {
        return points_;
    }

    std::size_t pointCount() const { return points_.size(); }

private:
    std::vector<std::shared_ptr<PipePoint>> points_;
};

} // namespace model
