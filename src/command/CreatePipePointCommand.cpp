// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "command/CreatePipePointCommand.h"
#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "engine/TopologyManager.h"
#include "model/Route.h"
#include "model/Segment.h"
#include "model/PipeSpec.h"

#include <stdexcept>

namespace {

// PipePointType 与字符串的互相转换

std::string pointTypeToString(model::PipePointType t) {
    switch (t) {
        case model::PipePointType::Run:       return "Run";
        case model::PipePointType::Bend:      return "Bend";
        case model::PipePointType::Reducer:   return "Reducer";
        case model::PipePointType::Tee:       return "Tee";
        case model::PipePointType::Valve:     return "Valve";
        case model::PipePointType::FlexJoint: return "FlexJoint";
    }
    return "Run";
}

model::PipePointType stringToPointType(const std::string& s) {
    if (s == "Run")       return model::PipePointType::Run;
    if (s == "Bend")      return model::PipePointType::Bend;
    if (s == "Reducer")   return model::PipePointType::Reducer;
    if (s == "Tee")       return model::PipePointType::Tee;
    if (s == "Valve")     return model::PipePointType::Valve;
    if (s == "FlexJoint") return model::PipePointType::FlexJoint;
    throw std::runtime_error("CreatePipePointCommand: unknown point type: " + s);
}

// UUID 解析
int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    throw std::runtime_error("CreatePipePointCommand: invalid hex char in UUID");
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
        throw std::runtime_error("CreatePipePointCommand: invalid UUID string");
    }
    foundation::UUID id;
    for (std::size_t i = 0; i < 16; ++i) {
        id.data[i] = static_cast<uint8_t>((hexVal(hex[2 * i]) << 4) | hexVal(hex[2 * i + 1]));
    }
    return id;
}

} // anonymous namespace

namespace command {

// ============================================================
// 工厂方法
// ============================================================

std::unique_ptr<CreatePipePointCommand> CreatePipePointCommand::create(
    const foundation::UUID& routeId,
    const foundation::UUID& segmentId,
    const std::string& name,
    model::PipePointType pointType,
    double x, double y, double z,
    const std::string& pipeSpecId,
    std::size_t insertIndex)
{
    auto cmd = std::unique_ptr<CreatePipePointCommand>(new CreatePipePointCommand());
    cmd->routeId_     = routeId;
    cmd->segmentId_   = segmentId;
    cmd->name_        = name;
    cmd->pointType_   = pointType;
    cmd->x_           = x;
    cmd->y_           = y;
    cmd->z_           = z;
    cmd->pipeSpecId_  = pipeSpecId;
    cmd->insertIndex_ = insertIndex;
    return cmd;
}

std::unique_ptr<CreatePipePointCommand> CreatePipePointCommand::fromJson(const nlohmann::json& j) {
    auto cmd = std::unique_ptr<CreatePipePointCommand>(new CreatePipePointCommand());
    cmd->routeId_     = parseUuid(j.at("routeId").get<std::string>());
    cmd->segmentId_   = parseUuid(j.at("segmentId").get<std::string>());
    cmd->name_        = j.at("name").get<std::string>();
    cmd->pointType_   = stringToPointType(j.at("pointType").get<std::string>());
    cmd->x_           = j.at("x").get<double>();
    cmd->y_           = j.at("y").get<double>();
    cmd->z_           = j.at("z").get<double>();
    cmd->pipeSpecId_  = j.value("pipeSpecId", "");
    cmd->insertIndex_ = j.value("insertIndex", std::numeric_limits<std::size_t>::max());
    // 反序列化时恢复已创建的 ID（若存在）
    if (j.contains("createdPointId") && !j["createdPointId"].get<std::string>().empty()) {
        cmd->createdPointId_ = parseUuid(j["createdPointId"].get<std::string>());
        cmd->executed_ = true;
    }
    if (j.contains("createdBranchId") && !j["createdBranchId"].get<std::string>().empty()) {
        cmd->createdBranchId_ = parseUuid(j["createdBranchId"].get<std::string>());
    }
    return cmd;
}

// ============================================================
// execute
// ============================================================

void CreatePipePointCommand::execute(CommandContext& ctx) {
    if (!ctx.document || !ctx.topologyManager) {
        throw std::runtime_error("CreatePipePointCommand: CommandContext 缺少 document 或 topologyManager");
    }

    // 1. 创建 PipePoint
    auto pp = std::make_shared<model::PipePoint>(name_, pointType_, gp_Pnt(x_, y_, z_));

    // 若是 redo，恢复原 UUID
    if (executed_ && !createdPointId_.isNull()) {
        pp->setIdForDeserialization(createdPointId_);
    }

    // 2. 关联 PipeSpec（若有）
    if (!pipeSpecId_.empty()) {
        auto specObj = ctx.document->findObjectShared(parseUuid(pipeSpecId_));
        if (specObj) {
            auto spec = std::dynamic_pointer_cast<model::PipeSpec>(specObj);
            if (spec) pp->setPipeSpec(spec);
        }
    }

    // 3. 加入文档
    createdPointId_ = pp->id();
    ctx.document->addObject(pp);

    // 4. 查找 Route 和 Segment
    auto* routeObj = ctx.document->findObject(routeId_);
    auto* route = dynamic_cast<model::Route*>(routeObj);
    if (!route) {
        throw std::runtime_error("CreatePipePointCommand: route not found: " + routeId_.toString());
    }

    auto* segObj = ctx.document->findObject(segmentId_);
    auto* segment = dynamic_cast<model::Segment*>(segObj);
    if (!segment) {
        throw std::runtime_error("CreatePipePointCommand: segment not found: " + segmentId_.toString());
    }

    // 5. 通过 TopologyManager 追加/插入管点
    std::shared_ptr<model::Segment> branchSeg;
    if (insertIndex_ >= segment->pointCount()) {
        branchSeg = ctx.topologyManager->appendPoint(*route, *segment, pp);
    } else {
        branchSeg = ctx.topologyManager->insertPoint(*route, *segment, insertIndex_, pp);
    }

    // 记录 Tee 分支段
    if (branchSeg) {
        createdBranchId_ = branchSeg->id();
        // 分支段也加入文档
        ctx.document->addObject(branchSeg);
    }

    // 6. 注册 DependencyGraph 依赖（与相邻管点）
    if (ctx.dependencyGraph) {
        // 找到新管点在段中的索引
        std::size_t idx = 0;
        for (std::size_t i = 0; i < segment->pointCount(); ++i) {
            if (segment->pointAt(i)->id() == createdPointId_) {
                idx = i;
                break;
            }
        }

        // 前邻
        if (idx > 0) {
            auto* prev = segment->pointAt(idx - 1);
            if (prev) {
                ctx.dependencyGraph->addDependency(createdPointId_, prev->id());
            }
        }

        // 后邻
        if (idx + 1 < segment->pointCount()) {
            auto* next = segment->pointAt(idx + 1);
            if (next) {
                ctx.dependencyGraph->addDependency(createdPointId_, next->id());
            }
        }
    }

    // 7. 设置执行结果
    executed_ = true;
    lastResult_ = CommandResult{};
    lastResult_.success = true;
    lastResult_.createdIds = {createdPointId_};
    lastResult_.affectedIds = {createdPointId_};

    // 添加相邻管点到 affectedIds
    if (ctx.dependencyGraph) {
        std::size_t idx = 0;
        for (std::size_t i = 0; i < segment->pointCount(); ++i) {
            if (segment->pointAt(i)->id() == createdPointId_) { idx = i; break; }
        }
        if (idx > 0 && segment->pointAt(idx - 1))
            lastResult_.affectedIds.push_back(segment->pointAt(idx - 1)->id());
        if (idx + 1 < segment->pointCount() && segment->pointAt(idx + 1))
            lastResult_.affectedIds.push_back(segment->pointAt(idx + 1)->id());
    }
}

// ============================================================
// undo
// ============================================================

void CreatePipePointCommand::undo(CommandContext& ctx) {
    if (!ctx.document || !ctx.topologyManager) {
        throw std::runtime_error("CreatePipePointCommand::undo: CommandContext 缺少 document 或 topologyManager");
    }

    // 收集相邻管点 IDs（在移除之前）
    std::vector<foundation::UUID> neighborIds;
    auto* routeObj = ctx.document->findObject(routeId_);
    auto* route = dynamic_cast<model::Route*>(routeObj);
    if (route) {
        // 找到管点所在的段及其索引
        for (const auto& seg : route->segments()) {
            for (std::size_t i = 0; i < seg->pointCount(); ++i) {
                if (seg->pointAt(i)->id() == createdPointId_) {
                    if (i > 0) neighborIds.push_back(seg->pointAt(i - 1)->id());
                    if (i + 1 < seg->pointCount()) neighborIds.push_back(seg->pointAt(i + 1)->id());
                    break;
                }
            }
        }
    }

    // 1. 若有分支段，先从文档和路由中移除
    if (!createdBranchId_.isNull() && route) {
        route->removeSegment(createdBranchId_);
        ctx.document->removeObject(createdBranchId_);
    }

    // 2. 通过 TopologyManager 移除管点
    if (route) {
        ctx.topologyManager->removePoint(*route, createdPointId_);
    }

    // 3. 从文档移除
    ctx.document->removeObject(createdPointId_);

    // 4. 从 DependencyGraph 移除
    if (ctx.dependencyGraph) {
        ctx.dependencyGraph->removeObject(createdPointId_);
    }

    // 5. 设置结果
    lastResult_ = CommandResult{};
    lastResult_.success = true;
    lastResult_.deletedIds = {createdPointId_};
    lastResult_.affectedIds = neighborIds;
}

// ============================================================
// description / type / toJson
// ============================================================

std::string CreatePipePointCommand::description() const {
    return "创建管点 " + name_;
}

CommandType CreatePipePointCommand::type() const {
    return CommandType::CreatePipePoint;
}

nlohmann::json CreatePipePointCommand::toJson() const {
    nlohmann::json j;
    j["type"]           = "CreatePipePoint";
    j["id"]             = id_.toString();
    j["routeId"]        = routeId_.toString();
    j["segmentId"]      = segmentId_.toString();
    j["name"]           = name_;
    j["pointType"]      = pointTypeToString(pointType_);
    j["x"]              = x_;
    j["y"]              = y_;
    j["z"]              = z_;
    j["pipeSpecId"]     = pipeSpecId_;
    j["insertIndex"]    = insertIndex_;
    j["createdPointId"] = createdPointId_.isNull() ? "" : createdPointId_.toString();
    j["createdBranchId"] = createdBranchId_.isNull() ? "" : createdBranchId_.toString();
    return j;
}

} // namespace command
