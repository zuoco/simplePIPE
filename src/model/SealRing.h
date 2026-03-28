#pragma once

#include "model/Accessory.h"

namespace model {

/// 密封圈：材料 + 截面尺寸
class SealRing : public Accessory {
public:
    explicit SealRing(const std::string& name = "",
                      const gp_Pnt& position = gp_Pnt(0, 0, 0))
        : Accessory(name, position), crossSectionDiameter_(0.0) {}

    const std::string& sealMaterial() const { return sealMaterial_; }
    void setSealMaterial(const std::string& material) {
        if (sealMaterial_ != material) {
            sealMaterial_ = material;
            changed.emit();
        }
    }

    double crossSectionDiameter() const { return crossSectionDiameter_; }
    void setCrossSectionDiameter(double d) {
        if (crossSectionDiameter_ != d) {
            crossSectionDiameter_ = d;
            changed.emit();
        }
    }

private:
    std::string sealMaterial_;
    double crossSectionDiameter_;
};

} // namespace model
