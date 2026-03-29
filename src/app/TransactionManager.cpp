#include "app/TransactionManager.h"
#include "model/DisplacementLoad.h"
#include "model/LoadCombination.h"
#include "model/PressureLoad.h"
#include "model/PropertyObject.h"
#include "model/SeismicLoad.h"
#include "model/ThermalLoad.h"
#include "model/UserDefinedLoad.h"
#include "model/WindLoad.h"
#include "model/PipePoint.h"

#include <gp_Pnt.hxx>

namespace app {
namespace {

bool variantToBool(const foundation::Variant& value) {
    if (const auto* i = std::get_if<int>(&value)) {
        return *i != 0;
    }
    if (const auto* d = std::get_if<double>(&value)) {
        return *d != 0.0;
    }
    if (const auto* s = std::get_if<std::string>(&value)) {
        return (*s == "true") || (*s == "1") || (*s == "True");
    }
    return false;
}

} // namespace

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
            continue;
        }

        auto* pp = dynamic_cast<model::PipePoint*>(obj);
        if (pp && (ch.key == "x" || ch.key == "y" || ch.key == "z")) {
            const double coordinate = foundation::variantToDouble(value);
            const gp_Pnt current = pp->position();
            if (ch.key == "x") {
                pp->setPosition(gp_Pnt(coordinate, current.Y(), current.Z()));
            } else if (ch.key == "y") {
                pp->setPosition(gp_Pnt(current.X(), coordinate, current.Z()));
            } else {
                pp->setPosition(gp_Pnt(current.X(), current.Y(), coordinate));
            }
            continue;
        }

        if (pp && ch.key == "type") {
            pp->setType(static_cast<model::PipePointType>(foundation::variantToInt(value)));
            continue;
        }

        if (pp && ch.key == "pipeSpecId") {
            if (auto* specId = std::get_if<std::string>(&value)) {
                pp->setParam("pipeSpecId", *specId);
            }
            continue;
        }

        if (auto* thermal = dynamic_cast<model::ThermalLoad*>(obj)) {
            if (ch.key == "installTemp") {
                thermal->setInstallTemp(foundation::variantToDouble(value));
                continue;
            }
            if (ch.key == "operatingTemp") {
                thermal->setOperatingTemp(foundation::variantToDouble(value));
                continue;
            }
        }

        if (auto* pressure = dynamic_cast<model::PressureLoad*>(obj)) {
            if (ch.key == "pressure") {
                pressure->setPressure(foundation::variantToDouble(value));
                continue;
            }
            if (ch.key == "isExternal") {
                pressure->setIsExternal(variantToBool(value));
                continue;
            }
        }

        if (auto* wind = dynamic_cast<model::WindLoad*>(obj)) {
            if (ch.key == "speed") {
                wind->setSpeed(foundation::variantToDouble(value));
                continue;
            }
            if (ch.key == "windDirectionX" || ch.key == "windDirectionY" || ch.key == "windDirectionZ") {
                foundation::math::Vec3 direction = wind->direction();
                const double component = foundation::variantToDouble(value);
                if (ch.key == "windDirectionX") direction.x = component;
                if (ch.key == "windDirectionY") direction.y = component;
                if (ch.key == "windDirectionZ") direction.z = component;
                wind->setDirection(direction);
                continue;
            }
        }

        if (auto* seismic = dynamic_cast<model::SeismicLoad*>(obj)) {
            if (ch.key == "acceleration") {
                seismic->setAcceleration(foundation::variantToDouble(value));
                continue;
            }
            if (ch.key == "seismicDirectionX" || ch.key == "seismicDirectionY" || ch.key == "seismicDirectionZ") {
                foundation::math::Vec3 direction = seismic->direction();
                const double component = foundation::variantToDouble(value);
                if (ch.key == "seismicDirectionX") direction.x = component;
                if (ch.key == "seismicDirectionY") direction.y = component;
                if (ch.key == "seismicDirectionZ") direction.z = component;
                seismic->setDirection(direction);
                continue;
            }
        }

        if (auto* displacement = dynamic_cast<model::DisplacementLoad*>(obj)) {
            if (ch.key == "translationX" || ch.key == "translationY" || ch.key == "translationZ") {
                foundation::math::Vec3 translation = displacement->translation();
                const double component = foundation::variantToDouble(value);
                if (ch.key == "translationX") translation.x = component;
                if (ch.key == "translationY") translation.y = component;
                if (ch.key == "translationZ") translation.z = component;
                displacement->setTranslation(translation);
                continue;
            }
            if (ch.key == "rotationX" || ch.key == "rotationY" || ch.key == "rotationZ") {
                foundation::math::Vec3 rotation = displacement->rotation();
                const double component = foundation::variantToDouble(value);
                if (ch.key == "rotationX") rotation.x = component;
                if (ch.key == "rotationY") rotation.y = component;
                if (ch.key == "rotationZ") rotation.z = component;
                displacement->setRotation(rotation);
                continue;
            }
        }

        if (auto* userDefined = dynamic_cast<model::UserDefinedLoad*>(obj)) {
            if (ch.key == "forceX" || ch.key == "forceY" || ch.key == "forceZ") {
                foundation::math::Vec3 force = userDefined->force();
                const double component = foundation::variantToDouble(value);
                if (ch.key == "forceX") force.x = component;
                if (ch.key == "forceY") force.y = component;
                if (ch.key == "forceZ") force.z = component;
                userDefined->setForce(force);
                continue;
            }
            if (ch.key == "momentX" || ch.key == "momentY" || ch.key == "momentZ") {
                foundation::math::Vec3 moment = userDefined->moment();
                const double component = foundation::variantToDouble(value);
                if (ch.key == "momentX") moment.x = component;
                if (ch.key == "momentY") moment.y = component;
                if (ch.key == "momentZ") moment.z = component;
                userDefined->setMoment(moment);
                continue;
            }
        }

        if (auto* combination = dynamic_cast<model::LoadCombination*>(obj)) {
            if (ch.key == "category") {
                combination->setCategory(static_cast<model::StressCategory>(foundation::variantToInt(value)));
                continue;
            }
            if (ch.key == "method") {
                combination->setMethod(static_cast<model::CombineMethod>(foundation::variantToInt(value)));
                continue;
            }
        }

        if (auto* propObj = dynamic_cast<model::PropertyObject*>(obj)) {
            propObj->setField(ch.key, value);
            continue;
        }

        if (pp) {
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
