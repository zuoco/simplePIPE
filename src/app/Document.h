// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/DocumentObject.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Segment.h"
#include "model/Route.h"
#include "foundation/Types.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <map>

namespace app {

/// 文档容器：持有所有文档对象，提供增删查改接口
class Document {
public:
    Document();
    ~Document() = default;

    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    // ---- 对象管理 ----

    /// 添加对象（夺取所有权）
    void addObject(std::shared_ptr<model::DocumentObject> obj);

    /// 按 UUID 删除对象，成功返回 true
    bool removeObject(const foundation::UUID& id);

    /// 按 UUID 查找对象，不存在返回 nullptr
    model::DocumentObject* findObject(const foundation::UUID& id) const;

    /// 按 UUID 查找对象，返回 shared_ptr
    std::shared_ptr<model::DocumentObject> findObjectShared(const foundation::UUID& id) const;

    /// 按名称查找对象，返回 shared_ptr（同名返回第一个匹配）
    std::shared_ptr<model::DocumentObject> findByName(const std::string& name) const;

    /// 按类型查找所有对象（裸指针数组）
    template<typename T>
    std::vector<T*> findByType() const {
        std::vector<T*> result;
        for (auto& [id, obj] : objects_) {
            if (auto* ptr = dynamic_cast<T*>(obj.get()))
                result.push_back(ptr);
        }
        return result;
    }

    /// 按类型查找，返回 map<UUID字符串, shared_ptr>
    template<typename T>
    std::map<std::string, std::shared_ptr<T>> findByTypeIdMap() const {
        std::map<std::string, std::shared_ptr<T>> result;
        for (auto& [id, obj] : objects_) {
            if (auto ptr = std::dynamic_pointer_cast<T>(obj))
                result.emplace(id, ptr);
        }
        return result;
    }

    /// 按类型查找，返回 map<名称, shared_ptr>
    template<typename T>
    std::map<std::string, std::shared_ptr<T>> findByTypeNameMap() const {
        std::map<std::string, std::shared_ptr<T>> result;
        for (auto& [id, obj] : objects_) {
            if (auto ptr = std::dynamic_pointer_cast<T>(obj))
                result.emplace(obj->name(), ptr);
        }
        return result;
    }

    /// 全部 PipePoint
    std::vector<model::PipePoint*> allPipePoints() const;

    /// 全部 Segment
    std::vector<model::Segment*> allSegments() const;

    /// 全部 PipeSpec
    std::vector<model::PipeSpec*> allPipeSpecs() const;

    /// 对象总数
    std::size_t objectCount() const { return objects_.size(); }

    /// 遍历所有对象
    void forEach(const std::function<void(model::DocumentObject&)>& fn) const;

    // ---- 文档状态 ----

    /// 文档名称
    const std::string& name() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

private:
    std::string name_;
    std::unordered_map<std::string, std::shared_ptr<model::DocumentObject>> objects_;
};

} // namespace app
