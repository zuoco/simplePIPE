#include "app/TransactionManager.h"
#include "model/PropertyObject.h"
#include "model/PipePoint.h"

namespace app {

TransactionManager::TransactionManager(Document& doc, DependencyGraph& graph)
    : doc_(doc), graph_(graph) {}

void TransactionManager::setRecomputeCallback(RecomputeCallback cb) {
    recomputeCb_ = std::move(cb);
}

void TransactionManager::open(const std::string& description) {
    if (open_) {
        // 隐式中止之前未提交的事务
        abort();
    }
    open_ = true;
    current_.description = description;
    current_.changes.clear();
}

void TransactionManager::recordChange(const foundation::UUID& objectId,
                                       const std::string& key,
                                       const foundation::Variant& oldValue,
                                       const foundation::Variant& newValue) {
    if (!open_) return;
    current_.changes.push_back({objectId, key, oldValue, newValue});
}

void TransactionManager::commit() {
    if (!open_) return;
    open_ = false;

    // 标脏并触发重算
    markAndRecompute(current_.changes);

    undoStack_.push_back(std::move(current_));
    current_ = {};

    // 新提交后清空 redo 栈
    redoStack_.clear();
}

void TransactionManager::abort() {
    if (!open_) return;
    open_ = false;

    // 回滚：反向应用变更（恢复旧值）
    applyChanges(current_.changes, /*forward=*/false);
    current_ = {};
}

bool TransactionManager::canUndo() const {
    return !undoStack_.empty();
}

bool TransactionManager::canRedo() const {
    return !redoStack_.empty();
}

void TransactionManager::undo() {
    if (!canUndo()) return;

    Transaction txn = std::move(undoStack_.back());
    undoStack_.pop_back();

    // 反向回放：恢复旧值
    applyChanges(txn.changes, /*forward=*/false);
    markAndRecompute(txn.changes);

    redoStack_.push_back(std::move(txn));
}

void TransactionManager::redo() {
    if (!canRedo()) return;

    Transaction txn = std::move(redoStack_.back());
    redoStack_.pop_back();

    // 正向回放：恢复新值
    applyChanges(txn.changes, /*forward=*/true);
    markAndRecompute(txn.changes);

    undoStack_.push_back(std::move(txn));
}

std::string TransactionManager::lastDescription() const {
    if (!undoStack_.empty()) return undoStack_.back().description;
    return {};
}

// ---- 私有方法 ----

void TransactionManager::applyChanges(const std::vector<PropertyChange>& changes,
                                       bool forward) {
    for (const auto& ch : changes) {
        model::DocumentObject* obj = doc_.findObject(ch.objectId);
        if (!obj) continue;

        const foundation::Variant& value = forward ? ch.newValue : ch.oldValue;

        if (ch.key == "name") {
            if (auto* s = std::get_if<std::string>(&value)) {
                obj->setName(*s);
            }
        } else if (auto* propObj = dynamic_cast<model::PropertyObject*>(obj)) {
            propObj->setField(ch.key, value);
        } else if (auto* pp = dynamic_cast<model::PipePoint*>(obj)) {
            pp->setParam(ch.key, value);
        }
    }
}

void TransactionManager::markAndRecompute(const std::vector<PropertyChange>& changes) {
    for (const auto& ch : changes) {
        graph_.markDirty(ch.objectId);
    }

    if (recomputeCb_) {
        std::vector<foundation::UUID> dirty = graph_.collectDirty();
        recomputeCb_(dirty);
        graph_.clearDirty();
    }
}

} // namespace app
