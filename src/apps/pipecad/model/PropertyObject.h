// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/DocumentObject.h"
#include "foundation/Types.h"

#include <map>
#include <string>

namespace model {

/// 属性对象：无坐标的文档对象，带可扩展字段字典
class PropertyObject : public DocumentObject {
public:
    explicit PropertyObject(const std::string& name = "")
        : DocumentObject(name) {}

    bool hasField(const std::string& key) const {
        return fields_.count(key) > 0;
    }

    const foundation::Variant& field(const std::string& key) const {
        return fields_.at(key);
    }

    void setField(const std::string& key, const foundation::Variant& value) {
        fields_[key] = value;
        changed.emit();
    }

    void removeField(const std::string& key) {
        if (fields_.erase(key)) {
            changed.emit();
        }
    }

    const std::map<std::string, foundation::Variant>& fields() const {
        return fields_;
    }

    /// "name" 代理到 DocumentObject::setProperty；其余 key 存入 fields_
    bool setProperty(const std::string& key, const foundation::Variant& value) override {
        if (key == "name") return DocumentObject::setProperty(key, value);
        setField(key, value);
        return true;
    }

    /// "name" 代理到 DocumentObject::getProperty；其余 key 从 fields_ 读取
    foundation::Variant getProperty(const std::string& key) const override {
        if (hasField(key)) return field(key);
        return DocumentObject::getProperty(key);
    }

protected:
    std::map<std::string, foundation::Variant> fields_;
};

} // namespace model
