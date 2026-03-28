#pragma once

#include "foundation/Types.h"
#include "foundation/Signal.h"

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

protected:
    foundation::UUID id_;
    std::string      name_;
};

} // namespace model
