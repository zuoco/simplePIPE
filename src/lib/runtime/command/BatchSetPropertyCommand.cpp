// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "command/BatchSetPropertyCommand.h"
#include "command/PropertyApplier.h"
#include "app/Document.h"

#include <stdexcept>

namespace {

nlohmann::json variantToJson(const foundation::Variant& value) {
    if (const auto* d = std::get_if<double>(&value))
        return {{"type", "double"}, {"value", *d}};
    if (const auto* i = std::get_if<int>(&value))
        return {{"type", "int"}, {"value", *i}};
    if (const auto* b = std::get_if<bool>(&value))
        return {{"type", "bool"}, {"value", *b}};
    if (const auto* vec = std::get_if<foundation::math::Vec3>(&value))
        return {{"type", "vec3"}, {"x", vec->x}, {"y", vec->y}, {"z", vec->z}};
    return {{"type", "string"}, {"value", std::get<std::string>(value)}};
}

} // anonymous namespace

namespace command {

BatchSetPropertyCommand::BatchSetPropertyCommand(std::string description,
                                                 std::vector<Change> changes)
    : description_(std::move(description))
    , changes_(std::move(changes))
{}

// ============================================================
// execute: 顺序 apply，失败时回滚已 apply 的修改
// ============================================================

void BatchSetPropertyCommand::execute(CommandContext& ctx)
{
    lastResult_ = CommandResult{};

    size_t applied = 0;
    try {
        for (auto& ch : changes_) {
            model::DocumentObject* obj = ctx.document->findObject(ch.objectId);
            if (!obj) {
                throw std::runtime_error(
                    "BatchSetPropertyCommand: object not found: " +
                    ch.objectId.toString());
            }
            PropertyApplier::apply(obj, ch.key, ch.newValue);
            lastResult_.affectedIds.push_back(ch.objectId);
            ++applied;
        }
    } catch (...) {
        // 回滚已 apply 的变更（逆序）
        for (size_t i = applied; i > 0; --i) {
            const auto& ch = changes_[i - 1];
            if (auto* obj = ctx.document->findObject(ch.objectId)) {
                try {
                    PropertyApplier::apply(obj, ch.key, ch.oldValue);
                } catch (...) {
                    // 回滚失败时不掩盖原始异常
                }
            }
        }
        lastResult_.affectedIds.clear();
        throw;
    }

    lastResult_.success = true;
}

// ============================================================
// undo: 逆序恢复旧值
// ============================================================

void BatchSetPropertyCommand::undo(CommandContext& ctx)
{
    lastResult_ = CommandResult{};

    for (auto it = changes_.rbegin(); it != changes_.rend(); ++it) {
        model::DocumentObject* obj = ctx.document->findObject(it->objectId);
        if (!obj) continue; // 对象已不存在，跳过
        PropertyApplier::apply(obj, it->key, it->oldValue);
        lastResult_.affectedIds.push_back(it->objectId);
    }

    lastResult_.success = true;
}

std::string BatchSetPropertyCommand::description() const
{
    return description_;
}

CommandType BatchSetPropertyCommand::type() const
{
    return CommandType::BatchSetProperty;
}

nlohmann::json BatchSetPropertyCommand::toJson() const
{
    nlohmann::json changesArr = nlohmann::json::array();
    for (const auto& ch : changes_) {
        changesArr.push_back({
            {"objectId", ch.objectId.toString()},
            {"key",      ch.key},
            {"oldValue", variantToJson(ch.oldValue)},
            {"newValue", variantToJson(ch.newValue)}
        });
    }

    return {
        {"type",        "BatchSetProperty"},
        {"id",          id_.toString()},
        {"description", description_},
        {"changes",     changesArr}
    };
}

} // namespace command
