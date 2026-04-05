// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/SpatialObject.h"

#include <gp_Vec.hxx>
#include <memory>

namespace model {

class PipePoint; // forward declaration — weak_ptr works with incomplete type

/// 附属构件基类：关联管点引用 + 偏移量
class Accessory : public SpatialObject {
public:
    explicit Accessory(const std::string& name = "",
                       const gp_Pnt& position = gp_Pnt(0, 0, 0))
        : SpatialObject(name, position), offset_(0.0, 0.0, 0.0) {}

    /// 关联的管点
    std::shared_ptr<PipePoint> pipePoint() const { return pipePoint_.lock(); }
    void attachTo(std::shared_ptr<PipePoint> pt) {
        pipePoint_ = pt;
        changed.emit();
    }
    void detach() {
        pipePoint_.reset();
        changed.emit();
    }

    /// 相对管点的偏移
    const gp_Vec& offset() const { return offset_; }
    void setOffset(const gp_Vec& offset) {
        offset_ = offset;
        changed.emit();
    }

protected:
    std::weak_ptr<PipePoint> pipePoint_;
    gp_Vec offset_;
};

} // namespace model
