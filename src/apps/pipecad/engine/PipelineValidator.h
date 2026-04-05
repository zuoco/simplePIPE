// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/Route.h"
#include "model/Segment.h"

#include <TopoDS_Shape.hxx>

#include <string>
#include <vector>

namespace engine {

/// 管道校验结果
struct ValidationWarning {
    enum class Severity { Warning, Error };
    Severity    severity = Severity::Warning;
    std::string objectId;  ///< 关联的对象 UUID 字符串（可为空）
    std::string message;
};

/// 管道校验器：结构完整性检查 + OCCT 干涉检测。
class PipelineValidator {
public:
    PipelineValidator() = default;

    /// 检查是否有管段少于 2 个管点（无法形成有效管道跨度）。
    std::vector<ValidationWarning> checkUnconnectedPorts(
        const model::Route& route) const;

    /// 干涉检测：逐对检查形体最小距离，distance < tolerance 时报告干涉错误。
    /// @param shapes     管件形体列表（与 objectIds 一一对应）
    /// @param objectIds  对应的文档对象 UUID 字符串
    /// @param tolerance  容差，单位与模型单位一致（默认 1e-3）
    std::vector<ValidationWarning> checkInterference(
        const std::vector<TopoDS_Shape>& shapes,
        const std::vector<std::string>&  objectIds,
        double tolerance = 1e-3) const;

    /// 运行所有结构校验（不含需要形体的干涉检测）
    std::vector<ValidationWarning> validateAll(const model::Route& route) const;
};

} // namespace engine
