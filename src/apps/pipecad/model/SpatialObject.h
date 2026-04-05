// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

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

    bool setProperty(const std::string& key, const foundation::Variant& value) override {
        if (key == "x" || key == "y" || key == "z") {
            double v = foundation::variantToDouble(value);
            gp_Pnt p = position_;
            if      (key == "x") p.SetX(v);
            else if (key == "y") p.SetY(v);
            else                 p.SetZ(v);
            setPosition(p);
            return true;
        }
        return DocumentObject::setProperty(key, value);
    }

    foundation::Variant getProperty(const std::string& key) const override {
        if (key == "x") return position_.X();
        if (key == "y") return position_.Y();
        if (key == "z") return position_.Z();
        return DocumentObject::getProperty(key);
    }

protected:
    gp_Pnt position_;
};

} // namespace model
