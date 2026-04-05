// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "command/Command.h"

#include <nlohmann/json.hpp>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace command {

/// 统一命令工厂注册表
///
/// 合并了原 CommandDispatcher + CommandFactory + CommandSerializer 的职责。
/// 每种命令类型注册一个 `json → unique_ptr<Command>` 的 lambda，运行时命令执行
/// 和序列化反序列化共享同一套注册表。
///
/// 使用方式（在 Application 初始化时）：
///   CommandRegistry reg;
///   reg.registerBuiltins();
///   auto cmd = reg.createFromParams("SetProperty", params);
///   stack.execute(std::move(cmd), ctx);
class CommandRegistry {
public:
    /// 工厂函数类型：接收完整命令 JSON，返回命令实例
    /// JSON 格式与 Command::toJson() 一致（字段平铺于顶层）
    using Factory = std::function<std::unique_ptr<Command>(const nlohmann::json&)>;

    /// 注册命令工厂
    /// @param typeKey 命令类型字符串（如 "SetProperty"、"Macro"）
    /// @param factory json → unique_ptr<Command> 工厂 lambda
    void registerFactory(const std::string& typeKey, Factory factory);

    /// 从外部参数 JSON 创建命令（外部协议 / 脚本分派用途）
    /// @param commandName 命令类型名称
    /// @param params      已解析的参数 JSON 对象（字段将平铺到完整 JSON 顶层）
    /// @throws std::out_of_range 若 commandName 未注册
    std::unique_ptr<Command> createFromParams(const std::string& commandName,
                                              const nlohmann::json& params) const;

    /// 从完整命令 JSON 创建命令（反序列化用途）
    /// @param j 完整命令 JSON（含 "type" 字段）
    /// @throws std::out_of_range 若 j["type"] 对应的命令未注册
    std::unique_ptr<Command> createFromFullJson(const nlohmann::json& j) const;

    /// 序列化单个命令为 JSON（静态，委托 cmd.toJson()）
    static nlohmann::json serialize(const Command& cmd);

    /// 反序列化单个命令（等价于 createFromFullJson）
    std::unique_ptr<Command> deserialize(const nlohmann::json& j) const;

    /// 序列化命令序列为 JSON 数组（脚本/宏文件用）
    static nlohmann::json serializeSequence(const std::vector<const Command*>& cmds);

    /// 反序列化命令序列
    std::vector<std::unique_ptr<Command>> deserializeSequence(const nlohmann::json& j) const;

    /// 查询已注册的命令名称列表（顺序不定）
    std::vector<std::string> registeredCommands() const;

    /// 检查命令名称是否已注册
    bool hasCommand(const std::string& name) const;

    /// 注册内置命令工厂：SetProperty、BatchSetProperty、Macro
    /// 建议在 Application 初始化时调用一次
    void registerBuiltins();

private:
    std::unordered_map<std::string, Factory> factories_;
};

} // namespace command
