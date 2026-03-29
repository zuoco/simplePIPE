#pragma once

#include "model/DocumentObject.h"
#include "foundation/Types.h"

#include <string>
#include <vector>
#include <algorithm>

namespace model {

/// 载荷基类：所有载荷类型的公共接口
class Load : public DocumentObject {
public:
    explicit Load(const std::string& name = "") : DocumentObject(name) {}
    virtual ~Load() = default;

    /// 类型标识字符串，用于序列化 / UI 显示
    virtual std::string loadType() const = 0;

    /// 返回此载荷作用的文档对象 UUID 列表（管段 / 管点）
    std::vector<foundation::UUID> affectedObjects() const {
        return affectedObjectIds_;
    }

    void addAffectedObject(const foundation::UUID& id) {
        auto it = std::find(affectedObjectIds_.begin(), affectedObjectIds_.end(), id);
        if (it == affectedObjectIds_.end()) {
            affectedObjectIds_.push_back(id);
            changed.emit();
        }
    }

    bool removeAffectedObject(const foundation::UUID& id) {
        auto it = std::find(affectedObjectIds_.begin(), affectedObjectIds_.end(), id);
        if (it != affectedObjectIds_.end()) {
            affectedObjectIds_.erase(it);
            changed.emit();
            return true;
        }
        return false;
    }

protected:
    std::vector<foundation::UUID> affectedObjectIds_;
};

} // namespace model
