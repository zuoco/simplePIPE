// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "command/SetPropertyCommand.h"
#include "command/PropertyApplier.h"
#include "app/Document.h"

#include <stdexcept>

namespace {

// ---- Variant JSON 序列化辅助（与 ProjectSerializer 保持一致）----

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

foundation::Variant jsonToVariant(const nlohmann::json& j) {
    const std::string type = j.at("type").get<std::string>();
    if (type == "double") return j.at("value").get<double>();
    if (type == "int")    return j.at("value").get<int>();
    if (type == "string") return j.at("value").get<std::string>();
    if (type == "bool")   return j.at("value").get<bool>();
    if (type == "vec3")   return foundation::math::Vec3{
        j.at("x").get<double>(), j.at("y").get<double>(), j.at("z").get<double>()};
    throw std::runtime_error("SetPropertyCommand: unsupported variant type: " + type);
}

} // anonymous namespace

namespace command {

// ============================================================
// 静态工厂方法
// ============================================================

std::unique_ptr<SetPropertyCommand> SetPropertyCommand::createAutoCapture(
    const foundation::UUID& objectId,
    const std::string& key,
    const foundation::Variant& newValue)
{
    auto cmd = std::unique_ptr<SetPropertyCommand>(new SetPropertyCommand());
    cmd->objectId_ = objectId;
    cmd->key_      = key;
    cmd->newValue_ = newValue;
    // oldValue_ 留为空，execute 时自动捕获
    return cmd;
}

std::unique_ptr<SetPropertyCommand> SetPropertyCommand::createWithOldValue(
    const foundation::UUID& objectId,
    const std::string& key,
    const foundation::Variant& oldValue,
    const foundation::Variant& newValue)
{
    auto cmd = std::unique_ptr<SetPropertyCommand>(new SetPropertyCommand());
    cmd->objectId_ = objectId;
    cmd->key_      = key;
    cmd->oldValue_ = oldValue;
    cmd->newValue_ = newValue;
    return cmd;
}

// ============================================================
// Command 接口实现
// ============================================================

void SetPropertyCommand::execute(CommandContext& ctx)
{
    model::DocumentObject* obj = ctx.document->findObject(objectId_);
    if (!obj) {
        throw std::runtime_error(
            "SetPropertyCommand: object not found: " + objectId_.toString());
    }

    // 自动捕获 oldValue（首次 execute 时）
    if (!oldValue_.has_value()) {
        oldValue_ = PropertyApplier::read(obj, key_);
    }

    PropertyApplier::apply(obj, key_, newValue_);

    lastResult_ = CommandResult{};
    lastResult_.success = true;
    lastResult_.affectedIds = {objectId_};
}

void SetPropertyCommand::undo(CommandContext& ctx)
{
    model::DocumentObject* obj = ctx.document->findObject(objectId_);
    if (!obj) {
        // 对象已不存在（例如被 DeletePipePointCommand undo 前置恢复）
        lastResult_ = CommandResult{};
        lastResult_.success = true;
        return;
    }

    PropertyApplier::apply(obj, key_, oldValue_.value());

    lastResult_ = CommandResult{};
    lastResult_.success = true;
    lastResult_.affectedIds = {objectId_};
}

std::string SetPropertyCommand::description() const
{
    return "设置属性 " + key_;
}

CommandType SetPropertyCommand::type() const
{
    return CommandType::SetProperty;
}

nlohmann::json SetPropertyCommand::toJson() const
{
    if (!oldValue_.has_value()) {
        throw std::logic_error(
            "SetPropertyCommand::toJson: execute 未调用，oldValue 尚未捕获");
    }

    nlohmann::json j;
    j["type"]     = "SetProperty";
    j["id"]       = id_.toString();
    j["objectId"] = objectId_.toString();
    j["key"]      = key_;
    j["oldValue"] = variantToJson(*oldValue_);
    j["newValue"] = variantToJson(newValue_);
    return j;
}

// ============================================================
// tryMerge — 连续输入合并（时间窗 500ms）
// ============================================================

bool SetPropertyCommand::tryMerge(const Command& next)
{
    const auto* n = dynamic_cast<const SetPropertyCommand*>(&next);
    if (!n) return false;
    if (n->objectId_ != objectId_) return false;
    if (n->key_      != key_)      return false;

    using namespace std::chrono;
    auto diff = duration_cast<milliseconds>(n->timestamp() - timestamp()).count();
    if (diff >= 500) return false;

    // 合并：保留最旧 oldValue，更新为最新 newValue
    newValue_ = n->newValue_;
    return true;
}

} // namespace command
