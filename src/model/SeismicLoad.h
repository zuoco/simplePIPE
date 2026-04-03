// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/Load.h"
#include "foundation/Math.h"

namespace model {

/// 地震载荷
class SeismicLoad : public Load {
public:
    explicit SeismicLoad(const std::string& name = "SeismicLoad")
        : Load(name) {}

    std::string loadType() const override { return "Seismic"; }

    /// 地震加速度（g，重力加速度倍数）
    double acceleration() const { return acceleration_; }
    void setAcceleration(double a) {
        if (acceleration_ != a) {
            acceleration_ = a;
            changed.emit();
        }
    }

    /// 地震作用方向（单位向量，世界坐标系）
    const foundation::math::Vec3& direction() const { return direction_; }
    void setDirection(const foundation::math::Vec3& d) {
        direction_ = d;
        changed.emit();
    }

private:
    double                 acceleration_ = 0.0;           ///< 加速度 (g)
    foundation::math::Vec3 direction_    = {0.0, 0.0, 1.0}; ///< 默认 +Z 方向（竖向）
};

} // namespace model
