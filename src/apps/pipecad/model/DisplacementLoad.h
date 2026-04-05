// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/Load.h"
#include "foundation/Math.h"

namespace model {

/// 强制位移载荷（锚点位移）
class DisplacementLoad : public Load {
public:
    explicit DisplacementLoad(const std::string& name = "DisplacementLoad")
        : Load(name) {}

    std::string loadType() const override { return "Displacement"; }

    /// 强制平移量（mm，XYZ 分量）
    const foundation::math::Vec3& translation() const { return translation_; }
    void setTranslation(const foundation::math::Vec3& t) {
        translation_ = t;
        changed.emit();
    }

    /// 强制转角（deg，绕 XYZ 轴的转角）
    const foundation::math::Vec3& rotation() const { return rotation_; }
    void setRotation(const foundation::math::Vec3& r) {
        rotation_ = r;
        changed.emit();
    }

    bool setProperty(const std::string& key, const foundation::Variant& value) override {
        if (key == "translation") { setTranslation(foundation::variantToVec3(value)); return true; }
        if (key == "rotation")    { setRotation(foundation::variantToVec3(value));    return true; }
        return DocumentObject::setProperty(key, value);
    }

    foundation::Variant getProperty(const std::string& key) const override {
        if (key == "translation") return translation_;
        if (key == "rotation")    return rotation_;
        return DocumentObject::getProperty(key);
    }

private:
    foundation::math::Vec3 translation_ = {0.0, 0.0, 0.0}; ///< 强制平移 (mm)
    foundation::math::Vec3 rotation_    = {0.0, 0.0, 0.0}; ///< 强制转角 (deg)
};

} // namespace model
