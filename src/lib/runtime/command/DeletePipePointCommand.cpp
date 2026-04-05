// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "command/DeletePipePointCommand.h"
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
    throw std::runtime_error("DeletePipePointCommand: unknown point type: " + s);
}

// Variant JSON 序列化

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
    throw std::runtime_error("DeletePipePointCommand: unsupported variant type: " + type);
}

// UUID 解析
int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    throw std::runtime_error("DeletePipePointCommand: invalid hex char in UUID");
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
        throw std::runtime_error("DeletePipePointCommand: invalid UUID string");
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

std::unique_ptr<DeletePipePointCommand> DeletePipePointCommand::create(
    const foundation::UUID& pointId)
{
    auto cmd = std::unique_ptr<DeletePipePointCommand>(new DeletePipePointCommand());
    cmd->pointId_ = pointId;
    return cmd;
}

std::unique_ptr<DeletePipePointCommand> DeletePipePointCommand::fromJson(const nlohmann::json& j) {
    auto cmd = std::unique_ptr<DeletePipePointCommand>(new DeletePipePointCommand());
    cmd->pointId_ = parseUuid(j.at("pointId").get<std::string>());

    // 反序列化已捕获的状态
    if (j.contains("savedState")) {
        const auto& st = j.at("savedState");
        auto& s = cmd->savedState_;
        s.id              = parseUuid(st.at("id").get<std::string>());
        s.name            = st.at("name").get<std::string>();
        s.type            = stringToPointType(st.at("type").get<std::string>());
        s.x               = st.at("x").get<double>();
        s.y               = st.at("y").get<double>();
        s.z               = st.at("z").get<double>();
        s.pipeSpecId      = st.value("pipeSpecId", "");
        s.routeId         = parseUuid(st.at("routeId").get<std::string>());
        s.segmentId       = parseUuid(st.at("segmentId").get<std::string>());
        s.indexInSegment  = st.at("indexInSegment").get<std::size_t>();
        s.branchSegmentId = st.value("branchSegmentId", "");

        if (st.contains("typeParams")) {
            for (auto& [k, v] : st["typeParams"].items()) {
                s.typeParams[k] = jsonToVariant(v);
            }
        }

        if (st.contains("accessories")) {
            for (const auto& acc : st["accessories"]) {
                PipePointState::AccessoryState as;
                as.id   = parseUuid(acc.at("id").get<std::string>());
                as.name = acc.at("name").get<std::string>();
                s.accessories.push_back(std::move(as));
            }
        }

        cmd->captured_ = true;
    }

    return cmd;
}

// ============================================================
// execute — 捕获状态 + 删除管点
// ============================================================

void DeletePipePointCommand::execute(CommandContext& ctx) {
    if (!ctx.document || !ctx.topologyManager) {
        throw std::runtime_error("DeletePipePointCommand: CommandContext 缺少 document 或 topologyManager");
    }

    // 1. 查找管点
    auto* obj = ctx.document->findObject(pointId_);
    auto* pp = dynamic_cast<model::PipePoint*>(obj);
    if (!pp) {
        throw std::runtime_error("DeletePipePointCommand: PipePoint not found: " + pointId_.toString());
    }

    // 2. 捕获快照（仅首次 execute 时捕获；redo 时使用已有快照）
    if (!captured_) {
        savedState_.id   = pp->id();
        savedState_.name = pp->name();
        savedState_.type = pp->type();
        savedState_.x    = pp->position().X();
        savedState_.y    = pp->position().Y();
        savedState_.z    = pp->position().Z();
        savedState_.typeParams = pp->typeParams();

        // PipeSpec ID
        if (pp->pipeSpec()) {
            savedState_.pipeSpecId = pp->pipeSpec()->id().toString();
        }

        // 找到所属 Route 和 Segment
        auto routes = ctx.document->findByType<model::Route>();
        for (auto* route : routes) {
            for (const auto& seg : route->segments()) {
                for (std::size_t i = 0; i < seg->pointCount(); ++i) {
                    if (seg->pointAt(i)->id() == pointId_) {
                        savedState_.routeId        = route->id();
                        savedState_.segmentId      = seg->id();
                        savedState_.indexInSegment  = i;
                        goto found;
                    }
                }
            }
        }
        found:

        // Tee 分支段
        if (pp->type() == model::PipePointType::Tee) {
            savedState_.branchSegmentId = ctx.topologyManager->branchSegmentId(pointId_);
        }

        // 附属构件
        for (const auto& acc : pp->accessories()) {
            PipePointState::AccessoryState as;
            as.id   = acc->id();
            as.name = acc->name();
            savedState_.accessories.push_back(std::move(as));
        }

        captured_ = true;
    }

    // 3. 收集相邻管点 IDs（在移除之前）
    std::vector<foundation::UUID> neighborIds;
    auto* routeObj = ctx.document->findObject(savedState_.routeId);
    auto* route = dynamic_cast<model::Route*>(routeObj);
    if (route) {
        auto* segObj = ctx.document->findObject(savedState_.segmentId);
        auto* segment = dynamic_cast<model::Segment*>(segObj);
        if (segment) {
            std::size_t idx = savedState_.indexInSegment;
            if (idx > 0 && segment->pointAt(idx - 1))
                neighborIds.push_back(segment->pointAt(idx - 1)->id());
            if (idx + 1 < segment->pointCount() && segment->pointAt(idx + 1))
                neighborIds.push_back(segment->pointAt(idx + 1)->id());
        }
    }

    // 4. 通过 TopologyManager 移除管点（处理 Tee 分支清理）
    if (route) {
        ctx.topologyManager->removePoint(*route, pointId_);
    }

    // 5. 从文档移除
    ctx.document->removeObject(pointId_);

    // 6. 从 DependencyGraph 移除
    if (ctx.dependencyGraph) {
        ctx.dependencyGraph->removeObject(pointId_);
    }

    // 7. 设置结果
    lastResult_ = CommandResult{};
    lastResult_.success = true;
    lastResult_.deletedIds = {pointId_};
    lastResult_.affectedIds = neighborIds;
}

// ============================================================
// undo — 从快照恢复管点
// ============================================================

void DeletePipePointCommand::undo(CommandContext& ctx) {
    if (!ctx.document || !ctx.topologyManager) {
        throw std::runtime_error("DeletePipePointCommand::undo: CommandContext 缺少 document 或 topologyManager");
    }

    // 1. 重建 PipePoint
    auto pp = std::make_shared<model::PipePoint>(
        savedState_.name, savedState_.type,
        gp_Pnt(savedState_.x, savedState_.y, savedState_.z));

    // 2. 恢复原 UUID
    pp->setIdForDeserialization(savedState_.id);

    // 3. 恢复 typeParams
    for (const auto& [key, val] : savedState_.typeParams) {
        pp->setParam(key, val);
    }

    // 4. 关联 PipeSpec
    if (!savedState_.pipeSpecId.empty()) {
        auto specObj = ctx.document->findObjectShared(parseUuid(savedState_.pipeSpecId));
        if (specObj) {
            auto spec = std::dynamic_pointer_cast<model::PipeSpec>(specObj);
            if (spec) pp->setPipeSpec(spec);
        }
    }

    // 5. 恢复附属构件（基本状态：UUID 和名称）
    for (const auto& accState : savedState_.accessories) {
        auto acc = std::make_shared<model::DocumentObject>(accState.name);
        acc->setIdForDeserialization(accState.id);
        pp->addAccessory(acc);
    }

    // 6. 加入文档
    ctx.document->addObject(pp);

    // 7. 恢复到 Segment 中的原位置
    auto* routeObj = ctx.document->findObject(savedState_.routeId);
    auto* route = dynamic_cast<model::Route*>(routeObj);
    if (!route) {
        throw std::runtime_error("DeletePipePointCommand::undo: route not found: " + savedState_.routeId.toString());
    }

    auto* segObj = ctx.document->findObject(savedState_.segmentId);
    auto* segment = dynamic_cast<model::Segment*>(segObj);
    if (!segment) {
        throw std::runtime_error("DeletePipePointCommand::undo: segment not found: " + savedState_.segmentId.toString());
    }

    // 使用 TopologyManager insertPoint 恢复到原位置
    if (savedState_.indexInSegment >= segment->pointCount()) {
        ctx.topologyManager->appendPoint(*route, *segment, pp);
    } else {
        ctx.topologyManager->insertPoint(*route, *segment, savedState_.indexInSegment, pp);
    }

    // 8. 若 Tee，重建分支段（TopologyManager::appendPoint/insertPoint 会自动处理 Tee 分支创建）
    //    分支段的 UUID 在这里不需要手动恢复，因为 TopologyManager 自动创建了新的分支段
    //    但我们需要在文档中注册分支段
    if (savedState_.type == model::PipePointType::Tee) {
        std::string newBranchId = ctx.topologyManager->branchSegmentId(savedState_.id);
        if (!newBranchId.empty()) {
            // 检查分支段是否已注册到文档（TopologyManager 创建但不注册文档）
            auto branchUuid = parseUuid(newBranchId);
            if (!ctx.document->findObject(branchUuid)) {
                // 在路由中找到分支段并注册到文档
                for (const auto& seg : route->segments()) {
                    if (seg->id() == branchUuid) {
                        ctx.document->addObject(seg);
                        break;
                    }
                }
            }
        }
    }

    // 9. 注册 DependencyGraph 依赖
    if (ctx.dependencyGraph) {
        std::size_t idx = savedState_.indexInSegment;
        if (idx > 0 && segment->pointAt(idx - 1)) {
            ctx.dependencyGraph->addDependency(savedState_.id, segment->pointAt(idx - 1)->id());
        }
        if (idx + 1 < segment->pointCount() && segment->pointAt(idx + 1)) {
            ctx.dependencyGraph->addDependency(savedState_.id, segment->pointAt(idx + 1)->id());
        }
    }

    // 10. 设置结果
    lastResult_ = CommandResult{};
    lastResult_.success = true;
    lastResult_.createdIds = {savedState_.id};
    lastResult_.affectedIds = {savedState_.id};

    // 添加相邻管点
    std::size_t idx = savedState_.indexInSegment;
    if (idx > 0 && segment->pointAt(idx - 1))
        lastResult_.affectedIds.push_back(segment->pointAt(idx - 1)->id());
    if (idx + 1 < segment->pointCount() && segment->pointAt(idx + 1))
        lastResult_.affectedIds.push_back(segment->pointAt(idx + 1)->id());
}

// ============================================================
// description / type / toJson
// ============================================================

std::string DeletePipePointCommand::description() const {
    return "删除管点 " + (captured_ ? savedState_.name : pointId_.toString());
}

CommandType DeletePipePointCommand::type() const {
    return CommandType::DeletePipePoint;
}

nlohmann::json DeletePipePointCommand::toJson() const {
    nlohmann::json j;
    j["type"]    = "DeletePipePoint";
    j["id"]      = id_.toString();
    j["pointId"] = pointId_.toString();

    if (captured_) {
        nlohmann::json st;
        st["id"]              = savedState_.id.toString();
        st["name"]            = savedState_.name;
        st["type"]            = pointTypeToString(savedState_.type);
        st["x"]               = savedState_.x;
        st["y"]               = savedState_.y;
        st["z"]               = savedState_.z;
        st["pipeSpecId"]      = savedState_.pipeSpecId;
        st["routeId"]         = savedState_.routeId.toString();
        st["segmentId"]       = savedState_.segmentId.toString();
        st["indexInSegment"]  = savedState_.indexInSegment;
        st["branchSegmentId"] = savedState_.branchSegmentId;

        // typeParams
        nlohmann::json params;
        for (const auto& [k, v] : savedState_.typeParams) {
            params[k] = variantToJson(v);
        }
        st["typeParams"] = params;

        // accessories
        nlohmann::json accs = nlohmann::json::array();
        for (const auto& acc : savedState_.accessories) {
            nlohmann::json a;
            a["id"]   = acc.id.toString();
            a["name"] = acc.name;
            accs.push_back(a);
        }
        st["accessories"] = accs;

        j["savedState"] = st;
    }

    return j;
}

} // namespace command
