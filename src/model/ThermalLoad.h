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

private:
    double installTemp_  = 20.0;   ///< 安装温度，默认 20°C
    double operatingTemp_ = 20.0;  ///< 操作温度，默认 20°C
};

} // namespace model
