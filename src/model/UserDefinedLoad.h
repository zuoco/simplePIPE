// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/Load.h"
#include "foundation/Math.h"

namespace model {

/// 用户自定义集中力 / 集中力矩载荷
class UserDefinedLoad : public Load {
public:
    explicit UserDefinedLoad(const std::string& name = "UserDefinedLoad")
        : Load(name) {}

    std::string loadType() const override { return "UserDefined"; }

    /// 集中力（N，XYZ 分量）
    const foundation::math::Vec3& force() const { return force_; }
    void setForce(const foundation::math::Vec3& f) {
        force_ = f;
        changed.emit();
    }

    /// 集中力矩（N·mm，绕 XYZ 轴）
    const foundation::math::Vec3& moment() const { return moment_; }
    void setMoment(const foundation::math::Vec3& m) {
        moment_ = m;
        changed.emit();
    }

private:
    foundation::math::Vec3 force_  = {0.0, 0.0, 0.0}; ///< 集中力 (N)
    foundation::math::Vec3 moment_ = {0.0, 0.0, 0.0}; ///< 集中力矩 (N·mm)
};

} // namespace model
