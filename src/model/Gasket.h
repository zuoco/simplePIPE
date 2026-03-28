#pragma once

#include "model/Accessory.h"

namespace model {

/// 垫片：材料 + 厚度
class Gasket : public Accessory {
public:
    explicit Gasket(const std::string& name = "",
                    const gp_Pnt& position = gp_Pnt(0, 0, 0))
        : Accessory(name, position), thickness_(0.0) {}

    const std::string& gasketMaterial() const { return gasketMaterial_; }
    void setGasketMaterial(const std::string& material) {
        if (gasketMaterial_ != material) {
            gasketMaterial_ = material;
            changed.emit();
        }
    }

    double thickness() const { return thickness_; }
    void setThickness(double t) {
        if (thickness_ != t) {
            thickness_ = t;
            changed.emit();
        }
    }

private:
    std::string gasketMaterial_;
    double thickness_;
};

} // namespace model
