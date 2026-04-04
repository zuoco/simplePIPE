// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/Load.h"

namespace model {

/// 温度载荷：描述安装温度与操作温度，用于计算热膨胀变形
class ThermalLoad : public Load {
public:
    explicit ThermalLoad(const std::string& name = "ThermalLoad")
        : Load(name) {}

    std::string loadType() const override { return "Thermal"; }

    /// 安装温度（°C）
    double installTemp() const { return installTemp_; }
    void setInstallTemp(double t) {
        if (installTemp_ != t) {
            installTemp_ = t;
            changed.emit();
        }
    }

    /// 操作温度（°C）
    double operatingTemp() const { return operatingTemp_; }
    void setOperatingTemp(double t) {
        if (operatingTemp_ != t) {
            operatingTemp_ = t;
            changed.emit();
        }
    }

    bool setProperty(const std::string& key, const foundation::Variant& value) override {
        if (key == "installTemp")   { setInstallTemp(foundation::variantToDouble(value));   return true; }
        if (key == "operatingTemp") { setOperatingTemp(foundation::variantToDouble(value)); return true; }
        return DocumentObject::setProperty(key, value);
    }

    foundation::Variant getProperty(const std::string& key) const override {
        if (key == "installTemp")   return installTemp_;
        if (key == "operatingTemp") return operatingTemp_;
        return DocumentObject::getProperty(key);
    }

private:
    double installTemp_  = 20.0;   ///< 安装温度，默认 20°C
    double operatingTemp_ = 20.0;  ///< 操作温度，默认 20°C
};

} // namespace model
