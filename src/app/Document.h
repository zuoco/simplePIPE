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

    /// 按类型查找所有对象
    template<typename T>
    std::vector<T*> findByType() const {
        std::vector<T*> result;
        for (auto& [id, obj] : objects_) {
            if (auto* ptr = dynamic_cast<T*>(obj.get()))
                result.push_back(ptr);
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
