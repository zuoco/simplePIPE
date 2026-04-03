// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "foundation/Types.h"

#include <functional>
#include <string>
#include <vector>

namespace app {

/// 单个属性变更记录
struct PropertyChange {
    foundation::UUID objectId;   ///< 被修改的对象 UUID
    std::string      key;        ///< 属性名
    foundation::Variant oldValue; ///< 旧值（undo 用）
    foundation::Variant newValue; ///< 新值（redo 用）
};

/// 单个事务：包含描述 + 属性变更列表
struct Transaction {
    std::string description;
    std::vector<PropertyChange> changes;
};

/// 事务管理器：open/commit/abort + undo/redo
///
/// 与 Document 和 DependencyGraph 协作：
/// - commit 后调用 DependencyGraph::markDirty 并触发 recomputeCallback
/// - undo/redo 反向/正向回放属性变更并重算
class TransactionManager {
public:
    /// recomputeCallback: commit/undo/redo 结束后调用，传入脏对象列表
    using RecomputeCallback = std::function<void(const std::vector<foundation::UUID>&)>;

    explicit TransactionManager(Document& doc, DependencyGraph& graph);

    /// 设置重算回调（由 RecomputeEngine 注册）
    void setRecomputeCallback(RecomputeCallback cb);

    // ---- 事务生命周期 ----

    /// 开启新事务（若已有未提交事务，先隐式 abort）
    void open(const std::string& description);

    /// 记录属性变更到当前事务（在 open 和 commit 之间调用）
    /// @param objectId 被修改对象的 UUID
    /// @param key      属性名
    /// @param oldValue 修改前的旧值
    /// @param newValue 修改后的新值
    void recordChange(const foundation::UUID& objectId,
                      const std::string& key,
                      const foundation::Variant& oldValue,
                      const foundation::Variant& newValue);

    /// 提交当前事务：保存日志、标脏相关对象、触发重算
    /// 若事务无变更，仍提交（空事务）
    void commit();

    /// 中止当前事务：回滚已应用的属性变更
    void abort();

    // ---- Undo / Redo ----

    bool canUndo() const;
    bool canRedo() const;

    /// 撤销最近一次事务
    void undo();

    /// 重做最近一次撤销
    void redo();

    // ---- 查询 ----

    bool isOpen() const { return open_; }

    std::size_t undoStackSize() const { return undoStack_.size(); }
    std::size_t redoStackSize() const { return redoStack_.size(); }

    /// 最近一次提交的事务描述（供 UI 显示）
    std::string lastDescription() const;

private:
    Document&         doc_;
    DependencyGraph&  graph_;
    RecomputeCallback recomputeCb_;

    bool        open_ = false;
    Transaction current_;

    std::vector<Transaction> undoStack_;
    std::vector<Transaction> redoStack_;

    /// 应用一组变更（正向：使用 newValue）
    void applyChanges(const std::vector<PropertyChange>& changes, bool forward);

    /// 提交后统一标脏并触发重算
    void markAndRecompute(const std::vector<PropertyChange>& changes);
};

} // namespace app
