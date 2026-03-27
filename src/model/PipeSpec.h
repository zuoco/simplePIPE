#pragma once

#include "model/PropertyObject.h"

namespace model {

/// 管道规格：可扩展字段字典，核心字段包括 OD/wallThickness/material 等
class PipeSpec : public PropertyObject {
public:
    explicit PipeSpec(const std::string& name = "")
        : PropertyObject(name) {}

    // 便捷访问核心字段
    double od() const {
        return hasField("OD") ? foundation::variantToDouble(field("OD")) : 0.0;
    }
    void setOd(double value) { setField("OD", value); }

    double wallThickness() const {
        return hasField("wallThickness")
                   ? foundation::variantToDouble(field("wallThickness"))
                   : 0.0;
    }
    void setWallThickness(double value) { setField("wallThickness", value); }

    std::string material() const {
        return hasField("material")
                   ? foundation::variantToString(field("material"))
                   : "";
    }
    void setMaterial(const std::string& value) { setField("material", value); }
};

} // namespace model
