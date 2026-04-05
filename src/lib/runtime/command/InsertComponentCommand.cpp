// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "command/InsertComponentCommand.h"
#include "command/CreatePipePointCommand.h"

#include <limits>
#include <stdexcept>
#include <unordered_map>

namespace {

// componentType → 中文描述
std::string componentLabel(const std::string& componentType) {
    static const std::unordered_map<std::string, std::string> labels = {
        {"insert-pipe",    "插入直管段"},
        {"insert-elbow",   "插入弯头"},
        {"insert-tee",     "插入三通"},
        {"insert-reducer", "插入大小头"},
        {"insert-valve",   "插入阀门"},
    };
    auto it = labels.find(componentType);
    return (it != labels.end()) ? it->second : ("插入 " + componentType);
}

// 自动生成管点名称前缀
std::string namePrefix(model::PipePointType type) {
    switch (type) {
        case model::PipePointType::Run:       return "P";
        case model::PipePointType::Bend:      return "A";
        case model::PipePointType::Tee:       return "T";
        case model::PipePointType::Reducer:   return "R";
        case model::PipePointType::Valve:     return "V";
        case model::PipePointType::FlexJoint: return "F";
    }
    return "P";
}

// UUID 解析
int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    throw std::runtime_error("InsertComponentCommand: invalid hex char in UUID");
}

foundation::UUID parseUuid(const std::string& s) {
    if (s.empty()) return foundation::UUID{};
    std::string hex;
    hex.reserve(32);
    for (char c : s) {
        if (c == '-') continue;
        hex.push_back(c);
    }
    if (hex.size() != 32) {
        throw std::runtime_error("InsertComponentCommand: invalid UUID string length");
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
// mapComponentType
// ============================================================

model::PipePointType InsertComponentCommand::mapComponentType(const std::string& componentType) {
    static const std::unordered_map<std::string, model::PipePointType> mapping = {
        {"insert-pipe",    model::PipePointType::Run},
        {"insert-elbow",   model::PipePointType::Bend},
        {"insert-tee",     model::PipePointType::Tee},
        {"insert-reducer", model::PipePointType::Reducer},
        {"insert-valve",   model::PipePointType::Valve},
    };
    auto it = mapping.find(componentType);
    if (it == mapping.end()) {
        throw std::runtime_error("InsertComponentCommand: unknown componentType: " + componentType);
    }
    return it->second;
}

// ============================================================
// 私有构造函数
// ============================================================

InsertComponentCommand::InsertComponentCommand(std::string componentType, std::string desc)
    : MacroCommand(std::move(desc))
    , componentType_(std::move(componentType)) {}

// ============================================================
// create — 工厂方法
// ============================================================

std::unique_ptr<InsertComponentCommand> InsertComponentCommand::create(
    const std::string& componentType,
    const foundation::UUID& routeId,
    const foundation::UUID& segmentId,
    double x, double y, double z,
    const std::string& pipeSpecId,
    std::size_t insertIndex)
{
    auto pointType = mapComponentType(componentType);
    auto desc = componentLabel(componentType);

    auto cmd = std::unique_ptr<InsertComponentCommand>(
        new InsertComponentCommand(componentType, desc));

    // 子命令：CreatePipePointCommand
    auto name = namePrefix(pointType) + "xx";  // 自动命名
    auto createCmd = CreatePipePointCommand::create(
        routeId, segmentId, name, pointType,
        x, y, z, pipeSpecId, insertIndex);

    cmd->addCommand(std::move(createCmd));
    return cmd;
}

// ============================================================
// fromJson — 反序列化
// ============================================================

std::unique_ptr<InsertComponentCommand> InsertComponentCommand::fromJson(const nlohmann::json& j) {
    auto compType = j.at("componentType").get<std::string>();
    auto desc = j.value("description", componentLabel(compType));

    auto cmd = std::unique_ptr<InsertComponentCommand>(
        new InsertComponentCommand(compType, desc));

    // 反序列化子命令
    if (j.contains("children")) {
        for (const auto& child : j.at("children")) {
            const auto childType = child.at("type").get<std::string>();
            if (childType == "CreatePipePoint") {
                cmd->addCommand(CreatePipePointCommand::fromJson(child));
            } else {
                throw std::runtime_error(
                    "InsertComponentCommand::fromJson: unsupported child type: " + childType);
            }
        }
    }

    // 恢复命令 ID
    if (j.contains("id")) {
        cmd->id_ = parseUuid(j.at("id").get<std::string>());
    }

    return cmd;
}

// ============================================================
// toJson — 序列化（额外输出 componentType）
// ============================================================

nlohmann::json InsertComponentCommand::toJson() const {
    auto j = MacroCommand::toJson();
    j["type"] = "InsertComponent";
    j["componentType"] = componentType_;
    return j;
}

} // namespace command
