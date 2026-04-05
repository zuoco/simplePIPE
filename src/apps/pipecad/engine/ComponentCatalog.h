// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "engine/ComponentTemplate.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

/// 参数化构件模板注册表 (单例)
/// GeometryDeriver 通过 templateId 查表获取对应模板
class ComponentCatalog {
public:
    static ComponentCatalog& instance();

    /// 注册一个模板 (ID 不能重复)
    void registerTemplate(std::unique_ptr<ComponentTemplate> tpl);

    /// 按 ID 查询模板，未找到返回 nullptr
    ComponentTemplate* getTemplate(const std::string& templateId) const;

    /// 返回所有已注册的模板 ID
    std::vector<std::string> allTemplateIds() const;

    /// 已注册模板数量
    size_t size() const;

    /// 清空所有模板 (主要用于测试)
    void clear();

private:
    ComponentCatalog();
    void registerBuiltins();

    std::unordered_map<std::string, std::unique_ptr<ComponentTemplate>> templates_;
};

} // namespace engine
