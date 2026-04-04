// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/DocumentObject.h"
#include "foundation/Types.h"

#include <stdexcept>
#include <string>

namespace command {

/// 属性变更薄转发层：替代 TransactionManager 中集中式 dynamic_cast 分派链
///
/// 通过 DocumentObject 虚方法 setProperty/getProperty 多态分派到各模型子类。
/// 新增模型类型只需覆写自身的 setProperty/getProperty，无需修改此类。
class PropertyApplier {
public:
    /// 将 key=value 应用到对象上（转发到 obj->setProperty()）
    /// @throws std::invalid_argument 若对象为 nullptr 或 key 不被该对象类型识别
    static void apply(model::DocumentObject* obj,
                      const std::string& key,
                      const foundation::Variant& value) {
        if (!obj) {
            throw std::invalid_argument("PropertyApplier::apply: null object");
        }
        if (!obj->setProperty(key, value)) {
            throw std::invalid_argument(
                "PropertyApplier::apply: unknown key '" + key +
                "' on object '" + obj->name() + "'");
        }
    }

    /// 读取对象的指定属性值（转发到 obj->getProperty()）
    /// @throws std::invalid_argument  若对象为 nullptr
    /// @throws std::out_of_range     若 key 不被该对象类型识别（由 getProperty 抛出）
    static foundation::Variant read(const model::DocumentObject* obj,
                                    const std::string& key) {
        if (!obj) {
            throw std::invalid_argument("PropertyApplier::read: null object");
        }
        return obj->getProperty(key);
    }
};

} // namespace command
