#include "engine/GeometryDeriver.h"
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

namespace engine {

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

} // namespace engine
