// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "command/CommandRegistry.h"
#include "command/SetPropertyCommand.h"
#include "command/BatchSetPropertyCommand.h"
#include "command/CreatePipePointCommand.h"
#include "command/DeletePipePointCommand.h"
#include "command/InsertComponentCommand.h"
#include "command/MacroCommand.h"

#include <stdexcept>

namespace {

// ---- Variant JSON 序列化辅助（与 ProjectSerializer / SetPropertyCommand 保持一致）----

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
    throw std::runtime_error("CommandRegistry: unsupported variant type: " + type);
}

// ---- UUID 解析（同 ProjectSerializer::parseUuid）----

int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    throw std::runtime_error("CommandRegistry: invalid hex character in UUID");
}

foundation::UUID parseUuid(const std::string& s) {
    std::string hex;
    hex.reserve(32);
    for (char c : s) {
        if (c == '-') continue;
        hex.push_back(c);
    }
    if (hex.size() != 32) {
        throw std::runtime_error("CommandRegistry: invalid UUID string length");
    }
    foundation::UUID id;
    for (std::size_t i = 0; i < 16; ++i) {
        const int hi = hexVal(hex[2 * i]);
        const int lo = hexVal(hex[2 * i + 1]);
        id.data[i] = static_cast<uint8_t>((hi << 4) | lo);
    }
    return id;
}

} // anonymous namespace

namespace command {

// ============================================================
// registerFactory
// ============================================================

void CommandRegistry::registerFactory(const std::string& typeKey, Factory factory) {
    factories_[typeKey] = std::move(factory);
}

// ============================================================
// createFromParams
// 将 params 平铺后加入 "type" 字段，传递给工厂 lambda
// ============================================================

std::unique_ptr<Command> CommandRegistry::createFromParams(
    const std::string& commandName,
    const nlohmann::json& params) const
{
    auto it = factories_.find(commandName);
    if (it == factories_.end()) {
        throw std::out_of_range(
            "CommandRegistry: unknown command '" + commandName + "'");
    }

    // 将 params 字段平铺到顶层，加入 "type"
    nlohmann::json full = params;
    full["type"] = commandName;

    return it->second(full);
}

// ============================================================
// createFromFullJson
// j 已包含 "type" 字段，直接分派到工厂 lambda
// ============================================================

std::unique_ptr<Command> CommandRegistry::createFromFullJson(
    const nlohmann::json& j) const
{
    const std::string typeName = j.at("type").get<std::string>();
    auto it = factories_.find(typeName);
    if (it == factories_.end()) {
        throw std::out_of_range(
            "CommandRegistry: unknown command '" + typeName + "'");
    }
    return it->second(j);
}

// ============================================================
// serialize（静态，委托 cmd.toJson()）
// ============================================================

nlohmann::json CommandRegistry::serialize(const Command& cmd) {
    return cmd.toJson();
}

// ============================================================
// deserialize（委托 createFromFullJson）
// ============================================================

std::unique_ptr<Command> CommandRegistry::deserialize(
    const nlohmann::json& j) const
{
    return createFromFullJson(j);
}

// ============================================================
// serializeSequence（静态，构建 JSON 数组）
// ============================================================

nlohmann::json CommandRegistry::serializeSequence(
    const std::vector<const Command*>& cmds)
{
    nlohmann::json arr = nlohmann::json::array();
    for (const auto* cmd : cmds) {
        arr.push_back(cmd->toJson());
    }
    return arr;
}

// ============================================================
// deserializeSequence
// ============================================================

std::vector<std::unique_ptr<Command>> CommandRegistry::deserializeSequence(
    const nlohmann::json& j) const
{
    std::vector<std::unique_ptr<Command>> result;
    result.reserve(j.size());
    for (const auto& item : j) {
        result.push_back(deserialize(item));
    }
    return result;
}

// ============================================================
// registeredCommands
// ============================================================

std::vector<std::string> CommandRegistry::registeredCommands() const {
    std::vector<std::string> names;
    names.reserve(factories_.size());
    for (const auto& kv : factories_) {
        names.push_back(kv.first);
    }
    return names;
}

// ============================================================
// hasCommand
// ============================================================

bool CommandRegistry::hasCommand(const std::string& name) const {
    return factories_.count(name) > 0;
}

// ============================================================
// registerBuiltins — 注册内置命令工厂
// ============================================================

void CommandRegistry::registerBuiltins() {
    // ---- SetProperty ----
    registerFactory("SetProperty", [](const nlohmann::json& j) -> std::unique_ptr<Command> {
        auto objId   = parseUuid(j.at("objectId").get<std::string>());
        auto key     = j.at("key").get<std::string>();
        auto newVal  = jsonToVariant(j.at("newValue"));
        if (j.contains("oldValue")) {
            return SetPropertyCommand::createWithOldValue(
                objId, key, jsonToVariant(j.at("oldValue")), newVal);
        }
        return SetPropertyCommand::createAutoCapture(objId, key, newVal);
    });

    // ---- BatchSetProperty ----
    registerFactory("BatchSetProperty", [](const nlohmann::json& j) -> std::unique_ptr<Command> {
        std::string desc = j.value("description", "");
        std::vector<BatchSetPropertyCommand::Change> changes;
        for (const auto& ch : j.at("changes")) {
            BatchSetPropertyCommand::Change c;
            c.objectId = parseUuid(ch.at("objectId").get<std::string>());
            c.key      = ch.at("key").get<std::string>();
            c.oldValue = jsonToVariant(ch.at("oldValue"));
            c.newValue = jsonToVariant(ch.at("newValue"));
            changes.push_back(std::move(c));
        }
        return std::make_unique<BatchSetPropertyCommand>(std::move(desc), std::move(changes));
    });

    // ---- CreatePipePoint ----
    registerFactory("CreatePipePoint", [](const nlohmann::json& j) -> std::unique_ptr<Command> {
        return CreatePipePointCommand::fromJson(j);
    });

    // ---- DeletePipePoint ----
    registerFactory("DeletePipePoint", [](const nlohmann::json& j) -> std::unique_ptr<Command> {
        return DeletePipePointCommand::fromJson(j);
    });

    // ---- Macro ----
    // 指向 this 会导致 lambda 捕获问题，此处通过 factory 参数访问 *this
    // 使用 std::function 存储后下面需递归反序列化，因此延迟到 deserialize 时调用
    registerFactory("Macro", [this](const nlohmann::json& j) -> std::unique_ptr<Command> {
        // 检查是否为 InsertComponent 子类型
        if (j.contains("componentType")) {
            return InsertComponentCommand::fromJson(j);
        }
        std::string desc = j.value("description", "");
        auto macro = std::make_unique<MacroCommand>(desc);
        // 若有 children 字段则递归反序列化子命令
        if (j.contains("children")) {
            for (const auto& child : j.at("children")) {
                macro->addCommand(deserialize(child));
            }
        }
        return macro;
    });

    // ---- InsertComponent（type 写为 "InsertComponent"，路由到专用反序列化）----
    registerFactory("InsertComponent", [](const nlohmann::json& j) -> std::unique_ptr<Command> {
        return InsertComponentCommand::fromJson(j);
    });
}

} // namespace command
