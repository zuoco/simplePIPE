#pragma once

#include "model/Accessory.h"

#include <gp_Dir.hxx>

namespace model {

/// 支架类型
enum class SupportType {
    Rod,    ///< 吊杆
    Spring, ///< 弹簧
    Rigid,  ///< 刚性支架
    Guide   ///< 导向支架
};

/// 支架：类型 + 承载方向
class Support : public Accessory {
public:
    explicit Support(const std::string& name = "",
                     SupportType type = SupportType::Rigid,
                     const gp_Pnt& position = gp_Pnt(0, 0, 0))
        : Accessory(name, position), supportType_(type), loadDirection_(0, 0, 1) {}

    SupportType supportType() const { return supportType_; }
    void setSupportType(SupportType type) {
        if (supportType_ != type) {
            supportType_ = type;
            changed.emit();
        }
    }

    const gp_Dir& loadDirection() const { return loadDirection_; }
    void setLoadDirection(const gp_Dir& dir) {
        if (!loadDirection_.IsEqual(dir, 1e-12)) {
            loadDirection_ = dir;
            changed.emit();
        }
    }

private:
    SupportType supportType_;
    gp_Dir loadDirection_;
};

} // namespace model
