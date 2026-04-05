// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "foundation/Types.h"
#include "foundation/Signal.h"

#include <stdexcept>
#include <string>

namespace model {

/// 文档对象基类：每个对象有唯一 ID 和名称
class DocumentObject {
public:
    explicit DocumentObject(const std::string& name = "")
        : id_(foundation::UUID::generate()), name_(name) {}

    virtual ~DocumentObject() = default;

    // 禁止拷贝，允许移动
    DocumentObject(const DocumentObject&) = delete;
    DocumentObject& operator=(const DocumentObject&) = delete;
    DocumentObject(DocumentObject&&) = default;
    DocumentObject& operator=(DocumentObject&&) = default;

    const foundation::UUID& id() const { return id_; }

    /// 仅用于反序列化加载历史工程时恢复对象 UUID。
    void setIdForDeserialization(const foundation::UUID& id) { id_ = id; }

    const std::string& name() const { return name_; }
    void setName(const std::string& name) {
        if (name_ != name) {
            name_ = name;
            changed.emit();
        }
    }

    /// 属性变更信号
    foundation::ChangeSignal changed;

    /// 通过 key 设置属性值。返回 true 表示成功，返回 false 表示 key 不被该对象类型识别。
    /// 基类只处理 "name"。子类可覆写以支持更多属性，未识别的 key 应调用基类并返回其结果。
    virtual bool setProperty(const std::string& key, const foundation::Variant& value) {
        if (key == "name") {
            if (const std::string* s = std::get_if<std::string>(&value)) {
                setName(*s);
                return true;
            }
            return false;
        }
        return false;
    }

    /// 通过 key 读取属性值。
    /// @throws std::out_of_range 若 key 不被识别
    virtual foundation::Variant getProperty(const std::string& key) const {
        if (key == "name") return name_;
        throw std::out_of_range("DocumentObject::getProperty: unknown key '" + key + "'");
    }

protected:
    foundation::UUID id_;
    std::string      name_;
};

} // namespace model
