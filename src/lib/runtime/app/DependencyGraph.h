// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

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
    ///
    /// **[主线程独占]** 仅允许在主线程（Qt 事件线程）调用。
    /// 命令执行完成后由 RecomputeEngine 或 CommandStack 触发。
    void markDirty(const foundation::UUID& id);

    /// 清除所有脏标记
    ///
    /// **[主线程独占]** 必须与 collectDirty() 在同一主线程调用序列中执行。
    /// 典型模式：collectDirty() → makeDocumentSnapshot() → clearDirty() →
    /// 向后台线程提交任务（T70 同步策略）。
    void clearDirty();

    /// 查询对象是否为脏
    ///
    /// **[主线程独占]** dirty_ 集合非线程安全，仅允许在主线程读取。
    bool isDirty(const foundation::UUID& id) const;

    /// 返回当前所有脏对象（拓扑排序：被依赖的先）
    ///
    /// **[主线程独占]** 调用后不清除脏标记（由 RecomputeEngine 决定何时清除）。
    /// **原子序列要求**：collectDirty() 与 clearDirty() 必须在同一主线程帧内
    /// 不被其他写操作打断，以确保快照与脏集合一致性（T70 同步策略）。
    std::vector<foundation::UUID> collectDirty() const;

    /// 查询对象直接依赖的上游节点。
    std::vector<foundation::UUID> directDependencies(const foundation::UUID& id) const;

    /// 查询对象的直接下游依赖者。
    std::vector<foundation::UUID> directDependents(const foundation::UUID& id) const;

    /// 返回图中已知的全部节点。
    std::vector<foundation::UUID> allNodes() const;

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
