// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/PropertyObject.h"
#include "foundation/Types.h"

namespace model {

/// 工程配置：工程名/作者/标准/单位制
class ProjectConfig : public PropertyObject {
public:
    explicit ProjectConfig(const std::string& name = "Untitled Project")
        : PropertyObject(name) {}

    std::string projectName() const {
        return hasField("projectName")
                   ? foundation::variantToString(field("projectName"))
                   : name();
    }
    void setProjectName(const std::string& value) {
        setField("projectName", value);
    }

    std::string author() const {
        return hasField("author")
                   ? foundation::variantToString(field("author"))
                   : "";
    }
    void setAuthor(const std::string& value) { setField("author", value); }

    std::string standard() const {
        return hasField("standard")
                   ? foundation::variantToString(field("standard"))
                   : "";
    }
    void setStandard(const std::string& value) { setField("standard", value); }

    foundation::UnitSystem unitSystem() const { return unitSystem_; }
    void setUnitSystem(foundation::UnitSystem us) {
        if (unitSystem_ != us) {
            unitSystem_ = us;
            changed.emit();
        }
    }

private:
    foundation::UnitSystem unitSystem_ = foundation::UnitSystem::SI;
};

} // namespace model
