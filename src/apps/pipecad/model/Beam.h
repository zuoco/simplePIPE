// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/SpatialObject.h"
#include "model/PipePoint.h"

#include <memory>

namespace model {

/// 梁截面类型
enum class BeamSectionType {
    Rectangular, ///< 矩形截面
    HSection     ///< H型钢截面
};

/// 梁：双端管点引用 + 截面参数
class Beam : public SpatialObject {
public:
    explicit Beam(const std::string& name = "",
                  const gp_Pnt& position = gp_Pnt(0, 0, 0))
        : SpatialObject(name, position),
          sectionType_(BeamSectionType::Rectangular),
          width_(100.0), height_(200.0) {}

    /// 起始管点
    std::shared_ptr<PipePoint> startPoint() const { return startPoint_.lock(); }
    void setStartPoint(std::shared_ptr<PipePoint> pt) {
        startPoint_ = pt;
        changed.emit();
    }

    /// 终止管点
    std::shared_ptr<PipePoint> endPoint() const { return endPoint_.lock(); }
    void setEndPoint(std::shared_ptr<PipePoint> pt) {
        endPoint_ = pt;
        changed.emit();
    }

    /// 截面类型
    BeamSectionType sectionType() const { return sectionType_; }
    void setSectionType(BeamSectionType type) {
        if (sectionType_ != type) {
            sectionType_ = type;
            changed.emit();
        }
    }

    /// 截面宽度 (mm)
    double width() const { return width_; }
    void setWidth(double w) {
        if (width_ != w) {
            width_ = w;
            changed.emit();
        }
    }

    /// 截面高度 (mm)
    double height() const { return height_; }
    void setHeight(double h) {
        if (height_ != h) {
            height_ = h;
            changed.emit();
        }
    }

    /// 计算梁长度 = 两端管点距离
    double length() const {
        auto sp = startPoint_.lock();
        auto ep = endPoint_.lock();
        if (sp && ep) {
            return sp->position().Distance(ep->position());
        }
        return 0.0;
    }

private:
    std::weak_ptr<PipePoint> startPoint_;
    std::weak_ptr<PipePoint> endPoint_;
    BeamSectionType sectionType_;
    double width_;
    double height_;
};

} // namespace model
