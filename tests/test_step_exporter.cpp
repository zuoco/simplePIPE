// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "app/Document.h"
#include "app/StepExporter.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Route.h"
#include "model/Segment.h"

#include <IFSelect_ReturnStatus.hxx>
#include <NCollection_Sequence.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_Label.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace {

std::string makeTempStepPath(const std::string& suffix) {
    return std::string("/tmp/pipecad_step_exporter_") + suffix + ".step";
}

std::shared_ptr<model::PipeSpec> makeDefaultSpec() {
    auto spec = std::make_shared<model::PipeSpec>("Spec-Default");
    spec->setOd(114.3);
    spec->setWallThickness(6.0);
    return spec;
}

void addPointWithSpec(const std::shared_ptr<model::Segment>& seg,
                      const std::shared_ptr<model::PipeSpec>& spec,
                      const std::string& name,
                      model::PipePointType type,
                      const gp_Pnt& pos) {
    auto pp = std::make_shared<model::PipePoint>(name, type, pos);
    pp->setPipeSpec(spec);
    seg->addPoint(pp);
}

} // namespace

TEST(StepExporter, ExportAll_WritesStepAndPreservesHierarchy) {
    app::Document doc;

    auto spec = makeDefaultSpec();
    auto route = std::make_shared<model::Route>("Route-A");
    auto seg = std::make_shared<model::Segment>("Seg-001");

    // 中间点为 Bend，确保至少有一个可推导几何。
    addPointWithSpec(seg, spec, "A00", model::PipePointType::Run, gp_Pnt(0, 0, 0));
    addPointWithSpec(seg, spec, "A01", model::PipePointType::Bend, gp_Pnt(300, 0, 0));
    addPointWithSpec(seg, spec, "A02", model::PipePointType::Run, gp_Pnt(300, 300, 0));
    route->addSegment(seg);

    doc.addObject(spec);
    doc.addObject(route);
    doc.addObject(seg);
    for (const auto& pp : seg->points()) {
        doc.addObject(pp);
    }

    const std::string path = makeTempStepPath("hierarchy");
    ASSERT_TRUE(app::StepExporter::exportAll(doc, path));

    STEPCAFControl_Reader reader;
    ASSERT_EQ(reader.ReadFile(path.c_str()), IFSelect_RetDone);

    auto xdeApp = XCAFApp_Application::GetApplication();
    ASSERT_FALSE(xdeApp.IsNull());

    occ::handle<TDocStd_Document> imported;
    xdeApp->NewDocument(TCollection_ExtendedString("XmlXCAF"), imported);
    ASSERT_FALSE(imported.IsNull());
    ASSERT_TRUE(reader.Transfer(imported));

    auto shapeTool = XCAFDoc_DocumentTool::ShapeTool(imported->Main());
    ASSERT_FALSE(shapeTool.IsNull());

    NCollection_Sequence<TDF_Label> freeShapes;
    shapeTool->GetFreeShapes(freeShapes);
    ASSERT_GE(freeShapes.Length(), 1);

    NCollection_Sequence<TDF_Label> routeComps;
    ASSERT_TRUE(XCAFDoc_ShapeTool::GetComponents(freeShapes.Value(1), routeComps, false));
    EXPECT_GE(routeComps.Length(), 1);

    TDF_Label routeDef;
    ASSERT_TRUE(XCAFDoc_ShapeTool::GetReferredShape(routeComps.Value(1), routeDef));

    NCollection_Sequence<TDF_Label> segmentComps;
    ASSERT_TRUE(XCAFDoc_ShapeTool::GetComponents(routeDef, segmentComps, false));
    EXPECT_GE(segmentComps.Length(), 1);

    TDF_Label segmentDef;
    ASSERT_TRUE(XCAFDoc_ShapeTool::GetReferredShape(segmentComps.Value(1), segmentDef));

    NCollection_Sequence<TDF_Label> componentComps;
    ASSERT_TRUE(XCAFDoc_ShapeTool::GetComponents(segmentDef, componentComps, false));
    EXPECT_GE(componentComps.Length(), 1);

    std::remove(path.c_str());
}

TEST(StepExporter, ExportAll_TenPointsUnderFiveSeconds) {
    app::Document doc;

    auto spec = makeDefaultSpec();
    auto route = std::make_shared<model::Route>("Route-Perf");
    auto seg = std::make_shared<model::Segment>("Seg-Perf");

    // 10 个点，类型用 Run；首末点会被跳过，中间点可生成直管几何。
    for (int i = 0; i < 10; ++i) {
        addPointWithSpec(
            seg,
            spec,
            "P" + std::to_string(i),
            model::PipePointType::Run,
            gp_Pnt(i * 100.0, 0.0, 0.0));
    }
    route->addSegment(seg);

    doc.addObject(spec);
    doc.addObject(route);
    doc.addObject(seg);
    for (const auto& pp : seg->points()) {
        doc.addObject(pp);
    }

    const std::string path = makeTempStepPath("perf");

    const auto start = std::chrono::steady_clock::now();
    const bool ok = app::StepExporter::exportAll(doc, path);
    const auto end = std::chrono::steady_clock::now();
    const auto elapsedMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    ASSERT_TRUE(ok);
    EXPECT_LT(elapsedMs, 5000);

    std::remove(path.c_str());
}
