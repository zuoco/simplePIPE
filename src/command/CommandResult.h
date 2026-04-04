// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "foundation/Types.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace command {

/// 命令执行结果：每个命令执行后产生的结构化结果，用于 UI 反馈
struct CommandResult {
    bool success = false;
    std::string errorMessage;

    std::vector<foundation::UUID> createdIds;   ///< 新创建的对象 ID
    std::vector<foundation::UUID> deletedIds;   ///< 被删除的对象 ID（execute 和 undo 均可写入）
    std::vector<foundation::UUID> affectedIds;  ///< 受影响的对象 ID（含脏对象）

    /// 创建的对象快照（供远程客户端确认，早期始终为空）
    std::vector<nlohmann::json> createdObjects;

    nlohmann::json toJson() const {
        auto uuidsToJson = [](const std::vector<foundation::UUID>& ids) {
            nlohmann::json arr = nlohmann::json::array();
            for (const auto& id : ids) arr.push_back(id.toString());
            return arr;
        };

        nlohmann::json j;
        j["success"]       = success;
        j["errorMessage"]  = errorMessage;
        j["createdIds"]    = uuidsToJson(createdIds);
        j["deletedIds"]    = uuidsToJson(deletedIds);
        j["affectedIds"]   = uuidsToJson(affectedIds);
        j["createdObjects"] = nlohmann::json(createdObjects);
        return j;
    }
};

} // namespace command
