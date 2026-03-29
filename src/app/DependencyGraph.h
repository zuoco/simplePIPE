#pragma once

#include "foundation/Types.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace app {

class Document;

/// 依赖图：维护文档对象间的依赖关系，支持脏标记传播和拓扑排序
///
/// 依赖边的语义：A 依赖 B，表示 B 的变化需要触发 A 重算。
/// 即图中存在边 B → A（"B 变化后通知 A"）。
class DependencyGraph {
public:
    DependencyGraph() = default;

    // ---- 依赖边管理 ----

    /// 添加依赖关系：dependent（依赖方）依赖 dependsOn（被依赖方）
    /// 当 dependsOn 变脏时，dependent 也会被标脏。
    void addDependency(const foundation::UUID& dependent,
                       const foundation::UUID& dependsOn);

    /// 移除指定依赖关系
    void removeDependency(const foundation::UUID& dependent,
                          const foundation::UUID& dependsOn);

    /// 移除对象的所有依赖关系（对象删除时调用）
    void removeObject(const foundation::UUID& id);

    // ---- 脏标记 ----

    /// 标记对象为脏，并沿依赖边传播（依赖它的对象也被标脏）
    void markDirty(const foundation::UUID& id);

    /// 清除所有脏标记
    void clearDirty();

    /// 查询对象是否为脏
    bool isDirty(const foundation::UUID& id) const;

    /// 返回当前所有脏对象（拓扑排序：被依赖的先）
    /// 调用后不清除脏标记（由 RecomputeEngine 决定何时清除）
    std::vector<foundation::UUID> collectDirty() const;

    /// 脏对象数量
    std::size_t dirtyCount() const { return dirty_.size(); }

    /// 基于文档中的 LoadCase/LoadCombination 引用重建载荷依赖链。
    /// 关系: Load -> LoadCase -> LoadCombination
    void rebuildLoadDependencyChain(const Document& document);

private:
    // dependents_[A] = {B, C} 表示 A 变脏后，B 和 C 也要标脏
    std::unordered_map<std::string, std::unordered_set<std::string>> dependents_;

    // 反向：dependencies_[B] = {A} 表示 B 依赖 A
    std::unordered_map<std::string, std::unordered_set<std::string>> dependencies_;

    std::unordered_set<std::string> dirty_;

    // 字符串 key → UUID 对象映射，用于 collectDirty() 返回 UUID
    std::unordered_map<std::string, foundation::UUID> uuidCache_;

    void cacheUUID(const foundation::UUID& id);

    // 拓扑排序辅助
    void topoSort(const std::string& node,
                  std::unordered_set<std::string>& visited,
                  std::vector<std::string>& order) const;
};

} // namespace app
