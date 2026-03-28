#include "app/StepExporter.h"

#include "engine/GeometryDeriver.h"
#include "model/PipePoint.h"
#include "model/Route.h"
#include "model/Segment.h"

#include <BRep_Builder.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <NCollection_Sequence.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <STEPControl_StepModelType.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_Label.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Compound.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace app {
namespace {

std::string pipePointTypeToString(model::PipePointType type) {
    switch (type) {
    case model::PipePointType::Run:
        return "Run";
    case model::PipePointType::Bend:
        return "Bend";
    case model::PipePointType::Reducer:
        return "Reducer";
    case model::PipePointType::Tee:
        return "Tee";
    case model::PipePointType::Valve:
        return "Valve";
    case model::PipePointType::FlexJoint:
        return "FlexJoint";
    default:
        return "Unknown";
    }
}

std::string preferredName(const std::string& value, const std::string& fallback) {
    return value.empty() ? fallback : value;
}

void setLabelName(const TDF_Label& label, const std::string& name) {
    TDataStd_Name::Set(label, TCollection_ExtendedString(name.c_str()));
}

TDF_Label newAssemblyLabel(const occ::handle<XCAFDoc_ShapeTool>& shapeTool,
                          const std::string& name) {
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);

    const TDF_Label label = shapeTool->AddShape(compound, true, false);
    setLabelName(label, name);
    return label;
}

bool exportSegmentsAsRoute(const std::vector<model::Segment*>& segments,
                           const std::string& routeName,
                           const occ::handle<XCAFDoc_ShapeTool>& shapeTool,
                           const TDF_Label& rootAssembly) {
    const TDF_Label routeLabel = newAssemblyLabel(shapeTool, routeName);
    bool routeHasGeometry = false;

    for (std::size_t segIndex = 0; segIndex < segments.size(); ++segIndex) {
        model::Segment* seg = segments[segIndex];
        if (!seg) continue;

        const std::string segmentName = preferredName(
            seg->name(), "Segment_" + std::to_string(segIndex + 1));
        const TDF_Label segmentLabel = newAssemblyLabel(shapeTool, segmentName);
        bool segmentHasGeometry = false;

        const auto& points = seg->points();
        for (std::size_t i = 0; i < points.size(); ++i) {
            const std::shared_ptr<model::PipePoint>& current = points[i];
            if (!current) continue;

            const gp_Pnt prev = (i > 0) ? points[i - 1]->position() : current->position();
            const gp_Pnt next = (i + 1 < points.size()) ? points[i + 1]->position() : current->position();

            TopoDS_Shape shape = engine::GeometryDeriver::deriveGeometry(prev, current, next);
            if (shape.IsNull()) continue;

            const TDF_Label componentShape = shapeTool->AddShape(shape, false, false);
            const std::string componentName = preferredName(
                current->name(),
                pipePointTypeToString(current->type()) + "_" + std::to_string(i + 1));
            setLabelName(componentShape, componentName);

            shapeTool->AddComponent(segmentLabel, componentShape, TopLoc_Location());
            segmentHasGeometry = true;
        }

        if (!segmentHasGeometry) continue;

        shapeTool->AddComponent(routeLabel, segmentLabel, TopLoc_Location());
        routeHasGeometry = true;
    }

    if (!routeHasGeometry) return false;

    shapeTool->AddComponent(rootAssembly, routeLabel, TopLoc_Location());
    return true;
}

} // namespace

bool StepExporter::exportAll(const Document& document, const std::string& filePath) {
    if (filePath.empty()) return false;

    const occ::handle<XCAFApp_Application> application = XCAFApp_Application::GetApplication();
    if (application.IsNull()) return false;

    occ::handle<TDocStd_Document> xdeDoc;
    application->NewDocument(TCollection_ExtendedString("XmlXCAF"), xdeDoc);
    if (xdeDoc.IsNull()) return false;

    const occ::handle<XCAFDoc_ShapeTool> shapeTool =
        XCAFDoc_DocumentTool::ShapeTool(xdeDoc->Main());
    if (shapeTool.IsNull()) return false;

    const TDF_Label rootAssembly = newAssemblyLabel(shapeTool, "PipeCAD");
    bool hasGeometry = false;

    auto routes = document.findByType<model::Route>();
    std::sort(routes.begin(), routes.end(), [](const model::Route* lhs, const model::Route* rhs) {
        if (!lhs || !rhs) return lhs < rhs;
        return lhs->name() < rhs->name();
    });

    for (std::size_t routeIndex = 0; routeIndex < routes.size(); ++routeIndex) {
        model::Route* route = routes[routeIndex];
        if (!route) continue;

        std::vector<model::Segment*> segments;
        segments.reserve(route->segments().size());
        for (const auto& seg : route->segments()) {
            segments.push_back(seg.get());
        }

        const std::string routeName = preferredName(
            route->name(), "Route_" + std::to_string(routeIndex + 1));
        if (exportSegmentsAsRoute(segments, routeName, shapeTool, rootAssembly)) {
            hasGeometry = true;
        }
    }

    if (!hasGeometry) {
        std::vector<model::Segment*> standaloneSegments = document.allSegments();
        if (exportSegmentsAsRoute(standaloneSegments, "Route_Standalone", shapeTool, rootAssembly)) {
            hasGeometry = true;
        }
    }

    if (!hasGeometry) return false;

    shapeTool->UpdateAssemblies();

    STEPCAFControl_Writer writer;
    writer.SetNameMode(true);
    writer.SetLayerMode(false);
    writer.SetColorMode(false);
    writer.SetPropsMode(false);

    if (!writer.Transfer(xdeDoc, STEPControl_AsIs)) return false;
    return writer.Write(filePath.c_str()) == IFSelect_RetDone;
}

} // namespace app