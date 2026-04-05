// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "command/MacroCommand.h"
#include "command/CommandContext.h"

#include <cstdio>

namespace command {

MacroCommand::MacroCommand(std::string desc)
    : description_(std::move(desc)) {}

void MacroCommand::addCommand(std::unique_ptr<Command> cmd) {
    children_.push_back(std::move(cmd));
}

void MacroCommand::execute(CommandContext& ctx) {
    lastResult_ = CommandResult{};
    size_t executed = 0;
    try {
        for (auto& child : children_) {
            child->execute(ctx);
            ++executed;
            // 累积子命令的影响范围
            const auto& cr = child->lastResult();
            for (const auto& id : cr.createdIds)  lastResult_.createdIds.push_back(id);
            for (const auto& id : cr.deletedIds)  lastResult_.deletedIds.push_back(id);
            for (const auto& id : cr.affectedIds) lastResult_.affectedIds.push_back(id);
        }
        lastResult_.success = true;
    } catch (...) {
        // 逆序回滚已执行的子命令，保证文档处于一致状态
        for (size_t i = executed; i > 0; --i) {
            try {
                children_[i - 1]->undo(ctx);
            } catch (...) {
                // 回滚失败：记录日志但不重新抛出，避免掩盖原始异常
                std::fprintf(stderr,
                    "[MacroCommand] WARNING: undo of child %zu failed during rollback\n",
                    i - 1);
            }
        }
        lastResult_.success = false;
        lastResult_.errorMessage = "MacroCommand: child command failed, rollback performed";
        lastResult_.createdIds.clear();
        lastResult_.deletedIds.clear();
        lastResult_.affectedIds.clear();
        throw;  // 重新抛出原始异常
    }
}

void MacroCommand::undo(CommandContext& ctx) {
    lastResult_ = CommandResult{};
    // 逆序撤销所有子命令
    for (size_t i = children_.size(); i > 0; --i) {
        children_[i - 1]->undo(ctx);
        const auto& cr = children_[i - 1]->lastResult();
        for (const auto& id : cr.deletedIds)  lastResult_.deletedIds.push_back(id);
        for (const auto& id : cr.affectedIds) lastResult_.affectedIds.push_back(id);
    }
    lastResult_.success = true;
}

std::string MacroCommand::description() const {
    return description_;
}

CommandType MacroCommand::type() const {
    return CommandType::Macro;
}

nlohmann::json MacroCommand::toJson() const {
    nlohmann::json j;
    j["type"]        = "Macro";
    j["id"]          = id_.toString();
    j["description"] = description_;
    j["children"]    = nlohmann::json::array();
    for (const auto& child : children_) {
        j["children"].push_back(child->toJson());
    }
    return j;
}

const std::vector<std::unique_ptr<Command>>& MacroCommand::children() const {
    return children_;
}

} // namespace command
