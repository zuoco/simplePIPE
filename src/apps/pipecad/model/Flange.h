// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/Accessory.h"

namespace model {

/// 法兰：等级 + 面型 + 螺栓孔数
class Flange : public Accessory {
public:
    explicit Flange(const std::string& name = "",
                    const gp_Pnt& position = gp_Pnt(0, 0, 0))
        : Accessory(name, position), boltHoleCount_(0) {}

    /// 压力等级 (e.g., "150", "300", "600")
    const std::string& rating() const { return rating_; }
    void setRating(const std::string& rating) {
        if (rating_ != rating) {
            rating_ = rating;
            changed.emit();
        }
    }

    /// 密封面类型 (e.g., "RF", "FF", "RTJ")
    const std::string& faceType() const { return faceType_; }
    void setFaceType(const std::string& type) {
        if (faceType_ != type) {
            faceType_ = type;
            changed.emit();
        }
    }

    /// 螺栓孔数量
    int boltHoleCount() const { return boltHoleCount_; }
    void setBoltHoleCount(int count) {
        if (boltHoleCount_ != count) {
            boltHoleCount_ = count;
            changed.emit();
        }
    }

private:
    std::string rating_;
    std::string faceType_;
    int boltHoleCount_;
};

} // namespace model
