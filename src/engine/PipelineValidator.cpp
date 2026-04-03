// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "engine/PipelineValidator.h"

#include <BRepExtrema_DistShapeShape.hxx>

#include <sstream>

namespace engine {

std::vector<ValidationWarning> PipelineValidator::checkUnconnectedPorts(
    const model::Route& route) const
{
    std::vector<ValidationWarning> warnings;
    for (const auto& seg : route.segments()) {
        if (seg->pointCount() < 2) {
            ValidationWarning w;
            w.severity = ValidationWarning::Severity::Warning;
            w.objectId = seg->id().toString();
            w.message  = "Segment '" + seg->name() +
                          "' has fewer than 2 points and cannot form a valid pipe span.";
            warnings.push_back(w);
        }
    }
    return warnings;
}

std::vector<ValidationWarning> PipelineValidator::checkInterference(
    const std::vector<TopoDS_Shape>& shapes,
    const std::vector<std::string>&  objectIds,
    double                          tolerance) const
{
    std::vector<ValidationWarning> warnings;
    const std::size_t n = shapes.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (shapes[i].IsNull()) continue;
        for (std::size_t j = i + 1; j < n; ++j) {
            if (shapes[j].IsNull()) continue;
            BRepExtrema_DistShapeShape dist(shapes[i], shapes[j]);
            if (!dist.IsDone()) continue;
            if (dist.Value() < tolerance) {
                const std::string& idI = (i < objectIds.size()) ? objectIds[i] : std::to_string(i);
                const std::string& idJ = (j < objectIds.size()) ? objectIds[j] : std::to_string(j);
                std::ostringstream msg;
                msg << "Interference between '" << idI << "' and '" << idJ
                    << "' (min distance=" << dist.Value() << ").";
                warnings.push_back({ValidationWarning::Severity::Error, idI, msg.str()});
            }
        }
    }
    return warnings;
}

std::vector<ValidationWarning> PipelineValidator::validateAll(
    const model::Route& route) const
{
    return checkUnconnectedPorts(route);
}

} // namespace engine
