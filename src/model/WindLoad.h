#pragma once

#include "model/Load.h"
#include "foundation/Math.h"

namespace model {

/// 风载荷
class WindLoad : public Load {
public:
    explicit WindLoad(const std::string& name = "WindLoad")
        : Load(name) {}

    std::string loadType() const override { return "Wind"; }

    /// 风速（m/s）
    double speed() const { return speed_; }
    void setSpeed(double s) {
        if (speed_ != s) {
            speed_ = s;
            changed.emit();
        }
    }

    /// 风向向量（单位向量，世界坐标系）
    const foundation::math::Vec3& direction() const { return direction_; }
    void setDirection(const foundation::math::Vec3& d) {
        direction_ = d;
        changed.emit();
    }

private:
    double               speed_     = 0.0;           ///< 风速 (m/s)
    foundation::math::Vec3 direction_ = {1.0, 0.0, 0.0}; ///< 默认 +X 方向
};

} // namespace model
