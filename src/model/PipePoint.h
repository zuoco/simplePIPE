#pragma once

#include "model/SpatialObject.h"
#include "foundation/Types.h"

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace model {

class PipeSpec;

/// 管点类型枚举
enum class PipePointType {
    Run,        ///< 直管点
    Bend,       ///< 弯头
    Reducer,    ///< 异径管
    Tee,        ///< 三通
    Valve,      ///< 阀门
    FlexJoint   ///< 柔性接头
};

/// 管点：管道系统的基本拓扑节点
class PipePoint : public SpatialObject {
public:
    explicit PipePoint(const std::string& name = "",
                       PipePointType type = PipePointType::Run,
                       const gp_Pnt& position = gp_Pnt(0, 0, 0))
        : SpatialObject(name, position), type_(type) {}

    PipePointType type() const { return type_; }
    void setType(PipePointType type) {
        if (type_ != type) {
            type_ = type;
            changed.emit();
        }
    }

    // PipeSpec 引用
    std::shared_ptr<PipeSpec> pipeSpec() const { return pipeSpec_; }
    void setPipeSpec(std::shared_ptr<PipeSpec> spec) {
        pipeSpec_ = std::move(spec);
        changed.emit();
    }

    // 类型参数（Bend: bendMultiplier; Valve: valveType 等）
    bool hasParam(const std::string& key) const {
        return typeParams_.count(key) > 0;
    }

    const foundation::Variant& param(const std::string& key) const {
        return typeParams_.at(key);
    }

    void setParam(const std::string& key, const foundation::Variant& value) {
        typeParams_[key] = value;
        changed.emit();
    }

    const std::map<std::string, foundation::Variant>& typeParams() const {
        return typeParams_;
    }

    // --- Accessory management (T06) ---
    void addAccessory(std::shared_ptr<DocumentObject> acc) {
        accessories_.push_back(std::move(acc));
        changed.emit();
    }

    bool removeAccessory(const foundation::UUID& accId) {
        auto it = std::find_if(accessories_.begin(), accessories_.end(),
            [&](const auto& a) { return a->id() == accId; });
        if (it != accessories_.end()) {
            accessories_.erase(it);
            changed.emit();
            return true;
        }
        return false;
    }

    const std::vector<std::shared_ptr<DocumentObject>>& accessories() const {
        return accessories_;
    }

    size_t accessoryCount() const { return accessories_.size(); }

private:
    PipePointType type_;
    std::shared_ptr<PipeSpec> pipeSpec_;
    std::map<std::string, foundation::Variant> typeParams_;
    std::vector<std::shared_ptr<DocumentObject>> accessories_;
};

} // namespace model
