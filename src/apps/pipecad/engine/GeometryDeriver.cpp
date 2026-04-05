// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/GeometryDeriver.h"
#include "engine/ComponentCatalog.h"
#include "engine/RunBuilder.h"
#include "engine/BendBuilder.h"
#include "engine/BendCalculator.h"
#include "engine/ReducerBuilder.h"
#include "engine/TeeBuilder.h"
#include "engine/ValveBuilder.h"
#include "engine/FlexJointBuilder.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "foundation/Types.h"

// DocumentSnapshot 通过 src 包含路径访问（lib_runtime 非强制 CMake 依赖）
#include "lib/runtime/app/DocumentSnapshot.h"

#include <gp_Trsf.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <cmath>

namespace engine {

// 辅助函数: 将原点建造的形体定位到 prevPoint→nextPoint
static TopoDS_Shape positionShape(
    const TopoDS_Shape& shape,
    const gp_Pnt& prevPoint,
    const gp_Pnt& nextPoint)
{
    gp_Vec dir(prevPoint, nextPoint);
    if (dir.Magnitude() < 1e-9) return shape;

    gp_Dir targetDir(dir);
    gp_Dir sourceDir(0, 0, 1);

    gp_Trsf trsf;
    // 如果方向相同（或反向），只需平移
    gp_Vec cross = gp_Vec(sourceDir).Crossed(gp_Vec(targetDir));
    if (cross.Magnitude() < 1e-12) {
        // 平行或反平行
        if (gp_Vec(sourceDir).Dot(gp_Vec(targetDir)) > 0) {
            trsf.SetTranslation(gp_Vec(gp_Pnt(0,0,0), prevPoint));
        } else {
            // 反向: 绕任意垂直轴旋转 180°
            gp_Ax1 rotAx(gp_Pnt(0,0,0), gp_Dir(1,0,0));
            trsf.SetRotation(rotAx, M_PI);
            gp_Trsf trans;
            trans.SetTranslation(gp_Vec(gp_Pnt(0,0,0), prevPoint));
            trsf = trans * trsf;
        }
    } else {
        // 一般情况: 旋转 + 平移
        gp_Ax1 rotAx(gp_Pnt(0,0,0), gp_Dir(cross));
        double angle = sourceDir.Angle(targetDir);
        trsf.SetRotation(rotAx, angle);
        gp_Trsf trans;
        trans.SetTranslation(gp_Vec(gp_Pnt(0,0,0), prevPoint));
        trsf = trans * trsf;
    }

    BRepBuilderAPI_Transform xform(shape, trsf, Standard_True);
    return xform.IsDone() ? xform.Shape() : shape;
}

TopoDS_Shape GeometryDeriver::deriveGeometry(
    const gp_Pnt& prevPoint,
    const std::shared_ptr<model::PipePoint>& current,
    const gp_Pnt& nextPoint)
{
    if (!current) return {};
    auto spec = current->pipeSpec();
    if (!spec) return {};

    double od = spec->od();
    double wt = spec->wallThickness();

    switch (current->type()) {
    case model::PipePointType::Run: {
        // Run: prevPoint → nextPoint straight pipe
        return RunBuilder::build(prevPoint, nextPoint, od, wt);
    }
    case model::PipePointType::Bend: {
        // Bend: calculate bend geometry, then build torus shell
        double multiplier = 1.5; // default
        if (current->hasParam("bendMultiplier")) {
            multiplier = foundation::variantToDouble(current->param("bendMultiplier"));
        }
        auto bendResult = BendCalculator::calculateBend(
            prevPoint, current->position(), nextPoint, od, multiplier);
        if (!bendResult) return {};
        return BendBuilder::build(*bendResult, od, wt);
    }
    case model::PipePointType::Reducer: {
        // Reducer: use current PipeSpec for startOD, need endOD from typeParams
        double endOD = od; // fallback
        if (current->hasParam("endOD")) {
            endOD = foundation::variantToDouble(current->param("endOD"));
        }
        return ReducerBuilder::build(prevPoint, nextPoint, od, endOD, wt);
    }
    case model::PipePointType::Tee: {
        // Tee: main run + branch; branch direction from typeParams
        // For now, treat prevPoint→nextPoint as main pipe
        // Branch endpoint from typeParams or default perpendicular
        gp_Pnt branchEnd = current->position(); // fallback
        if (current->hasParam("branchEndX") &&
            current->hasParam("branchEndY") &&
            current->hasParam("branchEndZ")) {
            branchEnd = gp_Pnt(
                foundation::variantToDouble(current->param("branchEndX")),
                foundation::variantToDouble(current->param("branchEndY")),
                foundation::variantToDouble(current->param("branchEndZ")));
        }
        double branchOD = od; // default same as main
        if (current->hasParam("branchOD")) {
            branchOD = foundation::variantToDouble(current->param("branchOD"));
        }
        return TeeBuilder::build(
            prevPoint, nextPoint, current->position(), branchEnd, od, branchOD, wt);
    }
    case model::PipePointType::Valve: {
        std::string valveType = "gate";
        if (current->hasParam("valveType")) {
            valveType = foundation::variantToString(current->param("valveType"));
        }
        // 尝试通过 ComponentCatalog 查表
        std::string templateId;
        if (valveType == "gate") templateId = "GateValve";
        // 其他阀门类型暂用原 Builder
        if (!templateId.empty()) {
            auto* tpl = ComponentCatalog::instance().getTemplate(templateId);
            if (tpl) {
                auto params = tpl->deriveParams(od, wt);
                // 设置实际长度: 两管点间距
                gp_Vec dir(prevPoint, nextPoint);
                double totalLen = dir.Magnitude();
                if (totalLen > 1e-9) {
                    params.bodyLength = totalLen - 2.0 * od; // 减去两端 stub
                    if (params.bodyLength < od) params.bodyLength = od;
                }
                auto shape = tpl->buildShape(params);
                if (!shape.IsNull()) {
                    // 将原点对齐到 prevPoint，轴向对齐到 prevPoint→nextPoint
                    return positionShape(shape, prevPoint, nextPoint);
                }
            }
        }
        return ValveBuilder::build(prevPoint, nextPoint, od, wt, valveType);
    }
    case model::PipePointType::FlexJoint: {
        int segments = 3;
        if (current->hasParam("segmentCount")) {
            segments = foundation::variantToInt(current->param("segmentCount"));
        }
        return FlexJointBuilder::build(prevPoint, nextPoint, od, wt, segments);
    }
    default:
        return {};
    }
}

// ——— 快照版本：基于只读 DocumentSnapshot 推导几何（后台线程安全）———

/// 快照辅助：从 typeParams 读 double，不存在时返回 defaultVal
static double snapParamDouble(
    const std::map<std::string, foundation::Variant>& params,
    const std::string& key,
    double defaultVal = 0.0)
{
    auto it = params.find(key);
    return (it != params.end()) ? foundation::variantToDouble(it->second) : defaultVal;
}

/// 快照辅助：从 typeParams 读 string，不存在时返回 defaultVal
static std::string snapParamString(
    const std::map<std::string, foundation::Variant>& params,
    const std::string& key,
    const std::string& defaultVal = {})
{
    auto it = params.find(key);
    return (it != params.end()) ? foundation::variantToString(it->second) : defaultVal;
}

/// 快照辅助：从 typeParams 读 int
static int snapParamInt(
    const std::map<std::string, foundation::Variant>& params,
    const std::string& key,
    int defaultVal = 0)
{
    auto it = params.find(key);
    return (it != params.end()) ? foundation::variantToInt(it->second) : defaultVal;
}

TopoDS_Shape GeometryDeriver::deriveFromSnapshot(
    const gp_Pnt& prevPoint,
    const app::PipePointSnapshot& current,
    const app::PipeSpecSnapshot* spec,
    const gp_Pnt& nextPoint)
{
    if (!spec) return {};

    // 从 PipeSpec 快照读取核心参数
    double od = snapParamDouble(spec->fields, "OD", 0.0);
    double wt = snapParamDouble(spec->fields, "wallThickness", 0.0);
    if (od <= 0.0) return {};

    const auto& params = current.typeParams;

    switch (current.type) {
    case model::PipePointType::Run: {
        return RunBuilder::build(prevPoint, nextPoint, od, wt);
    }
    case model::PipePointType::Bend: {
        double multiplier = snapParamDouble(params, "bendMultiplier", 1.5);
        auto bendResult = BendCalculator::calculateBend(
            prevPoint, current.position, nextPoint, od, multiplier);
        if (!bendResult) return {};
        return BendBuilder::build(*bendResult, od, wt);
    }
    case model::PipePointType::Reducer: {
        double endOD = snapParamDouble(params, "endOD", od);
        return ReducerBuilder::build(prevPoint, nextPoint, od, endOD, wt);
    }
    case model::PipePointType::Tee: {
        gp_Pnt branchEnd = current.position;
        if (params.count("branchEndX") && params.count("branchEndY") && params.count("branchEndZ")) {
            branchEnd = gp_Pnt(
                snapParamDouble(params, "branchEndX"),
                snapParamDouble(params, "branchEndY"),
                snapParamDouble(params, "branchEndZ"));
        }
        double branchOD = snapParamDouble(params, "branchOD", od);
        return TeeBuilder::build(
            prevPoint, nextPoint, current.position, branchEnd, od, branchOD, wt);
    }
    case model::PipePointType::Valve: {
        std::string valveType = snapParamString(params, "valveType", "gate");
        std::string templateId;
        if (valveType == "gate") templateId = "GateValve";
        if (!templateId.empty()) {
            auto* tpl = ComponentCatalog::instance().getTemplate(templateId);
            if (tpl) {
                auto tplParams = tpl->deriveParams(od, wt);
                gp_Vec dir(prevPoint, nextPoint);
                double totalLen = dir.Magnitude();
                if (totalLen > 1e-9) {
                    tplParams.bodyLength = totalLen - 2.0 * od;
                    if (tplParams.bodyLength < od) tplParams.bodyLength = od;
                }
                auto shape = tpl->buildShape(tplParams);
                if (!shape.IsNull()) {
                    return positionShape(shape, prevPoint, nextPoint);
                }
            }
        }
        return ValveBuilder::build(prevPoint, nextPoint, od, wt, valveType);
    }
    case model::PipePointType::FlexJoint: {
        int segments = snapParamInt(params, "segmentCount", 3);
        return FlexJointBuilder::build(prevPoint, nextPoint, od, wt, segments);
    }
    default:
        return {};
    }
}

} // namespace engine
