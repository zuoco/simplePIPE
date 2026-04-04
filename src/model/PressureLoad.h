// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/Load.h"

namespace model {

/// 压力载荷：内压或外压
class PressureLoad : public Load {
public:
    explicit PressureLoad(const std::string& name = "PressureLoad")
        : Load(name) {}

    std::string loadType() const override { return "Pressure"; }

    /// 压力值（MPa）
    double pressure() const { return pressure_; }
    void setPressure(double p) {
        if (pressure_ != p) {
            pressure_ = p;
            changed.emit();
        }
    }

    /// true = 外压，false = 内压（默认）
    bool isExternal() const { return isExternal_; }
    void setIsExternal(bool external) {
        if (isExternal_ != external) {
            isExternal_ = external;
            changed.emit();
        }
    }

    bool setProperty(const std::string& key, const foundation::Variant& value) override {
        if (key == "pressure")   { setPressure(foundation::variantToDouble(value));  return true; }
        if (key == "isExternal") { setIsExternal(foundation::variantToBool(value)); return true; }
        return DocumentObject::setProperty(key, value);
    }

    foundation::Variant getProperty(const std::string& key) const override {
        if (key == "pressure")   return pressure_;
        if (key == "isExternal") return isExternal_;
        return DocumentObject::getProperty(key);
    }

private:
    double pressure_   = 0.0;    ///< 压力 (MPa)
    bool   isExternal_ = false;  ///< 是否为外压
};

} // namespace model
