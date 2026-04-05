// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/DocumentObject.h"
#include "foundation/Types.h"

#include <string>
#include <vector>
#include <algorithm>

namespace model {

/// 载荷条目：一个基本工况中的单个载荷引用
struct LoadEntry {
    foundation::UUID loadId;   ///< 指向 Load 文档对象
    double factor = 1.0;       ///< 组合系数（通常 1.0）
};

/// 基本工况：一种物理运行状态下的载荷集合
///
/// 每个基本工况对应求解器的一次独立运算。
/// 示例：W（自重）、T1（热态）、P1（内压）等。
class LoadCase : public DocumentObject {
public:
    explicit LoadCase(const std::string& name = "") : DocumentObject(name) {}
    ~LoadCase() override = default;

    /// 返回工况名称（与 DocumentObject::name() 相同，提供语义化别名）
    const std::string& caseName() const { return name_; }

    /// 添加载荷条目。若已存在相同 loadId，则忽略（不重复添加）
    void addEntry(const LoadEntry& entry) {
        auto it = std::find_if(entries_.begin(), entries_.end(),
            [&](const LoadEntry& e) { return e.loadId == entry.loadId; });
        if (it == entries_.end()) {
            entries_.push_back(entry);
            changed.emit();
        }
    }

    /// 移除指定 loadId 的条目。成功返回 true，未找到返回 false
    bool removeEntry(const foundation::UUID& loadId) {
        auto it = std::find_if(entries_.begin(), entries_.end(),
            [&](const LoadEntry& e) { return e.loadId == loadId; });
        if (it != entries_.end()) {
            entries_.erase(it);
            changed.emit();
            return true;
        }
        return false;
    }

    const std::vector<LoadEntry>& entries() const { return entries_; }

private:
    std::vector<LoadEntry> entries_;
};

} // namespace model
