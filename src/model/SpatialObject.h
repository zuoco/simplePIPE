#pragma once

#include "model/DocumentObject.h"

#include <gp_Pnt.hxx>

namespace model {

/// 空间对象：带 3D 位置的文档对象
class SpatialObject : public DocumentObject {
public:
    explicit SpatialObject(const std::string& name = "",
                           const gp_Pnt& position = gp_Pnt(0, 0, 0))
        : DocumentObject(name), position_(position) {}

    const gp_Pnt& position() const { return position_; }
    void setPosition(const gp_Pnt& pos) {
        if (!position_.IsEqual(pos, 1e-12)) {
            position_ = pos;
            changed.emit();
        }
    }

protected:
    gp_Pnt position_;
};

} // namespace model
