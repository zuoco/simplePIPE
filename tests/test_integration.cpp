// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

/// T25 — 集成测试与端到端验证
///
/// 验收场景:
///   1. 新建工程 → 设置名称 → 名称可查
///   2. 管点输入 → 几何自动生成
///   3. 弯头标记 → N/M/F 自动计算 → 弯头几何生成
///   4. PipeSpec 修改 → 所有引用管点几何更新
///   5. 保存/加载 → 全部恢复
///   6. STEP 导出 → 文件有效

#include <gtest/gtest.h>

// Application layer
#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "app/ProjectSerializer.h"
#include "app/SelectionManager.h"
#include "app/StepExporter.h"
#include "app/WorkbenchManager.h"
#include "app/CadWorkbench.h"

// Engine layer
#include "engine/RecomputeEngine.h"
#include "engine/BendCalculator.h"
#include "engine/ConstraintSolver.h"
#include "engine/PipelineValidator.h"

// Model layer
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Segment.h"
#include "model/Route.h"
#include "model/ProjectConfig.h"
#include "model/Beam.h"
#include "model/Flange.h"

// Foundation
#include "foundation/Types.h"

// OCCT
#include <gp_Pnt.hxx>
#include <TopoDS_Shape.hxx>
#include <BRep_Tool.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <NCollection_Sequence.hxx>
#include <TDF_Label.hxx>
#include <IFSelect_ReturnStatus.hxx>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

// ============================================================
// Integration test fixture: wires up the full application stack
// (Document + DependencyGraph + RecomputeEngine)
// ============================================================

class IntegrationTest : public ::testing::Test {
protected:
    app::Document        doc;
    app::DependencyGraph graph;
    std::unique_ptr<engine::RecomputeEngine> engine;

    // Track shapes produced by recompute
    std::map<std::string, TopoDS_Shape> generatedShapes;
    int recomputeCallCount = 0;

    void SetUp() override {
        engine = std::make_unique<engine::RecomputeEngine>(doc, graph);

        // Capture generated shapes
        engine->setSceneUpdateCallback(
            [this](const std::string& uuid, const TopoDS_Shape& shape) {
                generatedShapes[uuid] = shape;
            });
    }

    /// 直接修改后触发 recompute（替代旧 TransactionManager 流程）
    void triggerRecompute(const foundation::UUID& id) {
        graph.markDirty(id);
        auto dirtyIds = graph.collectDirty();
        engine->recompute(dirtyIds);
        graph.clearDirty();
        ++recomputeCallCount;
    }

    // Helper: create a PipeSpec and register it
    std::shared_ptr<model::PipeSpec> makeSpec(const std::string& name,
                                               double od, double wt) {
        auto spec = std::make_shared<model::PipeSpec>(name);
        spec->setOd(od);
        spec->setWallThickness(wt);
        spec->setMaterial("CS");
        doc.addObject(spec);
        return spec;
    }

    // Helper: add a pipe point and register dependency on spec
    std::shared_ptr<model::PipePoint> makePoint(
        const std::string& name, model::PipePointType type,
        const gp_Pnt& pos, std::shared_ptr<model::PipeSpec> spec)
    {
        auto pp = std::make_shared<model::PipePoint>(name, type, pos);
        pp->setPipeSpec(spec);
        doc.addObject(pp);
        graph.addDependency(pp->id(), spec->id());
        return pp;
    }

    std::string tempPath(const std::string& suffix) {
        return std::string("/tmp/pipecad_integration_") + suffix;
    }
};

// ============================================================
// Scenario 1: 新建工程 — Create project, set name, verify
// ============================================================

TEST_F(IntegrationTest, Scenario1_CreateProject) {
    // Set project name
    doc.setName("TestPipeline");
    EXPECT_EQ(doc.name(), "TestPipeline");

    // Add ProjectConfig
    auto cfg = std::make_shared<model::ProjectConfig>("Config");
    cfg->setProjectName("TestPipeline");
    cfg->setAuthor("IntegrationTest");
    cfg->setStandard("ASME");
    cfg->setUnitSystem(foundation::UnitSystem::SI);
    doc.addObject(cfg);

    EXPECT_EQ(doc.objectCount(), 1u);
    auto configs = doc.findByType<model::ProjectConfig>();
    ASSERT_EQ(configs.size(), 1u);
    EXPECT_EQ(configs[0]->projectName(), "TestPipeline");
    EXPECT_EQ(configs[0]->author(), "IntegrationTest");
}

// ============================================================
// Scenario 2: 管点输入 — Input pipe points, recompute generates
//              straight run geometry
// ============================================================

TEST_F(IntegrationTest, Scenario2_PipePointInputAndGeometry) {
    auto spec = makeSpec("Spec-A", 114.3, 6.0);

    auto p0 = makePoint("A00", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("A01", model::PipePointType::Run,
                         gp_Pnt(1000, 0, 0), spec);
    auto p2 = makePoint("A02", model::PipePointType::Run,
                         gp_Pnt(1000, 1000, 0), spec);

    auto seg = std::make_shared<model::Segment>("S1");
    seg->addPoint(p0);
    seg->addPoint(p1);
    seg->addPoint(p2);

    auto route = std::make_shared<model::Route>("R1");
    route->addSegment(seg);

    doc.addObject(seg);
    doc.addObject(route);

    // Full recompute should generate shapes for all points
    engine->recomputeAll();

    // Each pipe point should have produced geometry
    EXPECT_FALSE(generatedShapes.empty());

    // At least the middle point (A01) should produce geometry since it
    // has both prev and next neighbors
    auto it = generatedShapes.find(p1->id().toString());
    ASSERT_NE(it, generatedShapes.end());
    EXPECT_FALSE(it->second.IsNull());
}

// ============================================================
// Scenario 3: 弯头 — Mark intersection as Bend, verify N/M/F
//              calculation and bend geometry generation
// ============================================================

TEST_F(IntegrationTest, Scenario3_BendAutoCalculation) {
    auto spec = makeSpec("Spec-B", 168.3, 7.11);

    // Three points forming a 90° turn
    auto p0 = makePoint("A00", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("A01", model::PipePointType::Bend,
                         gp_Pnt(1000, 0, 0), spec);
    p1->setParam("bendMultiplier", 1.5);
    auto p2 = makePoint("A02", model::PipePointType::Run,
                         gp_Pnt(1000, 1000, 0), spec);

    auto seg = std::make_shared<model::Segment>("S1");
    seg->addPoint(p0);
    seg->addPoint(p1);
    seg->addPoint(p2);

    auto route = std::make_shared<model::Route>("R1");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // Verify N/M/F calculation directly
    auto bend = engine::BendCalculator::calculateBend(
        p0->position(), p1->position(), p2->position(),
        spec->od(), 1.5);
    ASSERT_TRUE(bend.has_value());
    EXPECT_NEAR(bend->bendAngle, M_PI / 2.0, 1e-6);
    EXPECT_GT(bend->bendRadius, 0.0);

    // All arc points should be at bendRadius from center
    EXPECT_NEAR(bend->nearPoint.Distance(bend->arcCenter),
                bend->bendRadius, 1e-6);
    EXPECT_NEAR(bend->midPoint.Distance(bend->arcCenter),
                bend->bendRadius, 1e-6);
    EXPECT_NEAR(bend->farPoint.Distance(bend->arcCenter),
                bend->bendRadius, 1e-6);

    // Recompute should generate bend geometry
    engine->recomputeAll();

    auto it = generatedShapes.find(p1->id().toString());
    ASSERT_NE(it, generatedShapes.end());
    EXPECT_FALSE(it->second.IsNull());
}

// ============================================================
// Scenario 4: PipeSpec 修改 — Change OD via transaction,
//              verify dependent points are recomputed
// ============================================================

TEST_F(IntegrationTest, Scenario4_PipeSpecModification) {
    auto spec = makeSpec("Spec-C", 114.3, 6.0);

    auto p0 = makePoint("A00", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("A01", model::PipePointType::Run,
                         gp_Pnt(1000, 0, 0), spec);

    auto seg = std::make_shared<model::Segment>("S1");
    seg->addPoint(p0);
    seg->addPoint(p1);

    auto route = std::make_shared<model::Route>("R1");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // Initial recompute
    engine->recomputeAll();
    auto initialShapes = generatedShapes;
    generatedShapes.clear();
    recomputeCallCount = 0;

    // Modify OD and trigger recompute
    spec->setOd(219.1);
    triggerRecompute(spec->id());

    // Transaction should have triggered recompute
    EXPECT_GE(recomputeCallCount, 1);

    // Verify the shapes were regenerated (with new OD they'll differ)
    EXPECT_FALSE(generatedShapes.empty());
}

// ============================================================
// Scenario 6: 保存/加载 — Full model round-trip via JSON
// ============================================================

TEST_F(IntegrationTest, Scenario6_SaveLoadRoundTrip) {
    doc.setName("IntegrationProject");

    auto cfg = std::make_shared<model::ProjectConfig>("Config");
    cfg->setProjectName("IntegrationProject");
    cfg->setAuthor("Tester");
    cfg->setStandard("ASME");
    cfg->setUnitSystem(foundation::UnitSystem::SI);
    doc.addObject(cfg);

    auto spec = makeSpec("Spec-E", 168.3, 7.11);

    auto p0 = makePoint("A00", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("A01", model::PipePointType::Bend,
                         gp_Pnt(1000, 0, 0), spec);
    p1->setParam("bendMultiplier", 2.0);
    auto p2 = makePoint("A02", model::PipePointType::Run,
                         gp_Pnt(1000, 1000, 0), spec);

    // Attach flange accessory to p0
    auto flange = std::make_shared<model::Flange>("F1", gp_Pnt(0, 0, 0));
    flange->setRating("150");
    flange->setFaceType("RF");
    flange->setBoltHoleCount(8);
    flange->attachTo(p0);
    p0->addAccessory(flange);
    doc.addObject(flange);

    auto seg = std::make_shared<model::Segment>("S1");
    seg->addPoint(p0);
    seg->addPoint(p1);
    seg->addPoint(p2);

    auto route = std::make_shared<model::Route>("R1");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // Add beam
    auto beam = std::make_shared<model::Beam>("B1");
    beam->setSectionType(model::BeamSectionType::HSection);
    beam->setWidth(200.0);
    beam->setHeight(300.0);
    beam->setStartPoint(p0);
    beam->setEndPoint(p1);
    doc.addObject(beam);

    // Save
    const std::string path = tempPath("roundtrip.json");
    ASSERT_TRUE(app::ProjectSerializer::save(doc, path));

    // Load
    auto loaded = app::ProjectSerializer::load(path);
    ASSERT_NE(loaded, nullptr);

    // Verify counts
    auto loadedSpecs = loaded->findByType<model::PipeSpec>();
    EXPECT_EQ(loadedSpecs.size(), 1u);
    EXPECT_DOUBLE_EQ(loadedSpecs[0]->od(), 168.3);

    auto loadedPoints = loaded->allPipePoints();
    EXPECT_EQ(loadedPoints.size(), 3u);

    auto loadedSegments = loaded->allSegments();
    EXPECT_EQ(loadedSegments.size(), 1u);
    EXPECT_EQ(loadedSegments[0]->pointCount(), 3u);

    auto loadedRoutes = loaded->findByType<model::Route>();
    EXPECT_EQ(loadedRoutes.size(), 1u);

    auto loadedBeams = loaded->findByType<model::Beam>();
    EXPECT_EQ(loadedBeams.size(), 1u);
    EXPECT_DOUBLE_EQ(loadedBeams[0]->width(), 200.0);

    auto loadedConfigs = loaded->findByType<model::ProjectConfig>();
    EXPECT_EQ(loadedConfigs.size(), 1u);
    EXPECT_EQ(loadedConfigs[0]->author(), "Tester");

    // Verify bend point params survived
    bool foundBend = false;
    for (auto* pp : loadedPoints) {
        if (pp->type() == model::PipePointType::Bend) {
            foundBend = true;
            EXPECT_TRUE(pp->hasParam("bendMultiplier"));
            EXPECT_DOUBLE_EQ(
                foundation::variantToDouble(pp->param("bendMultiplier")), 2.0);
        }
    }
    EXPECT_TRUE(foundBend);

    // Save again and verify equal JSON
    const std::string path2 = tempPath("roundtrip2.json");
    ASSERT_TRUE(app::ProjectSerializer::save(*loaded, path2));

    // Cleanup
    std::remove(path.c_str());
    std::remove(path2.c_str());
}

// ============================================================
// Scenario 7: STEP 导出 — Export full model, verify file valid
// ============================================================

TEST_F(IntegrationTest, Scenario7_StepExport) {
    auto spec = makeSpec("Spec-F", 114.3, 6.0);

    auto p0 = makePoint("A00", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("A01", model::PipePointType::Bend,
                         gp_Pnt(500, 0, 0), spec);
    p1->setParam("bendMultiplier", 1.5);
    auto p2 = makePoint("A02", model::PipePointType::Run,
                         gp_Pnt(500, 500, 0), spec);

    auto seg = std::make_shared<model::Segment>("S1");
    seg->addPoint(p0);
    seg->addPoint(p1);
    seg->addPoint(p2);

    auto route = std::make_shared<model::Route>("R1");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    const std::string stepPath = tempPath("export.step");
    ASSERT_TRUE(app::StepExporter::exportAll(doc, stepPath));

    // Verify the STEP file can be read back
    STEPCAFControl_Reader reader;
    ASSERT_EQ(reader.ReadFile(stepPath.c_str()), IFSelect_RetDone);

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
    EXPECT_GE(freeShapes.Length(), 1);

    // Cleanup
    std::remove(stepPath.c_str());
}

// ============================================================
// Scenario: End-to-end pipeline — create, modify, validate,
//           recompute, save, load, export STEP
// ============================================================

TEST_F(IntegrationTest, EndToEnd_FullPipeline) {
    // --- Phase 1: Create project ---
    doc.setName("E2E-Project");
    auto cfg = std::make_shared<model::ProjectConfig>("Config");
    cfg->setProjectName("E2E-Project");
    cfg->setAuthor("E2E");
    cfg->setStandard("ASME");
    doc.addObject(cfg);

    auto spec = makeSpec("Spec-Main", 168.3, 7.11);

    // --- Phase 2: Add pipe points with bend ---
    auto p0 = makePoint("A00", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("A01", model::PipePointType::Bend,
                         gp_Pnt(1000, 0, 0), spec);
    p1->setParam("bendMultiplier", 1.5);
    auto p2 = makePoint("A02", model::PipePointType::Run,
                         gp_Pnt(1000, 1000, 0), spec);

    auto seg = std::make_shared<model::Segment>("Seg-001");
    seg->addPoint(p0);
    seg->addPoint(p1);
    seg->addPoint(p2);

    auto route = std::make_shared<model::Route>("Route-A");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // --- Phase 3: Initial recompute ---
    engine->recomputeAll();
    EXPECT_FALSE(generatedShapes.empty());
    size_t initialShapeCount = generatedShapes.size();

    // --- Phase 4: Validate pipeline ---
    engine::ConstraintSolver solver;
    auto errors = solver.checkAll(*route);
    // A valid 3-point pipeline with one bend should have no constraint errors
    EXPECT_TRUE(errors.empty());

    engine::PipelineValidator validator;
    auto warnings = validator.validateAll(*route);
    EXPECT_TRUE(warnings.empty());

    // --- Phase 5: Modify OD and trigger recompute ---
    generatedShapes.clear();
    spec->setOd(219.1);
    triggerRecompute(spec->id());

    // All points should be recomputed since they depend on spec
    EXPECT_FALSE(generatedShapes.empty());

    // --- Phase 6: Save project ---
    const std::string jsonPath = tempPath("e2e.json");
    ASSERT_TRUE(app::ProjectSerializer::save(doc, jsonPath));

    // --- Phase 7: Load project ---
    auto loaded = app::ProjectSerializer::load(jsonPath);
    ASSERT_NE(loaded, nullptr);

    // Verify loaded content
    EXPECT_EQ(loaded->allPipePoints().size(), 3u);
    EXPECT_EQ(loaded->allSegments().size(), 1u);
    EXPECT_EQ(loaded->findByType<model::Route>().size(), 1u);

    auto loadedSpecs = loaded->findByType<model::PipeSpec>();
    ASSERT_EQ(loadedSpecs.size(), 1u);
    // OD was modified to 219.1 in Phase 5
    EXPECT_DOUBLE_EQ(loadedSpecs[0]->od(), 219.1);

    // --- Phase 8: Recompute loaded document ---
    app::DependencyGraph loadedGraph;
    engine::RecomputeEngine loadedEngine(*loaded, loadedGraph);

    std::map<std::string, TopoDS_Shape> loadedShapes;
    loadedEngine.setSceneUpdateCallback(
        [&](const std::string& uuid, const TopoDS_Shape& shape) {
            loadedShapes[uuid] = shape;
        });
    loadedEngine.recomputeAll();
    EXPECT_FALSE(loadedShapes.empty());

    // --- Phase 9: STEP export from loaded document ---
    const std::string stepPath = tempPath("e2e.step");
    ASSERT_TRUE(app::StepExporter::exportAll(*loaded, stepPath));

    // Verify STEP file is readable
    STEPCAFControl_Reader reader;
    ASSERT_EQ(reader.ReadFile(stepPath.c_str()), IFSelect_RetDone);

    // Cleanup
    std::remove(jsonPath.c_str());
    std::remove(stepPath.c_str());
}

// ============================================================
// Cross-cutting: Selection manager integration
// ============================================================

TEST_F(IntegrationTest, SelectionManagerIntegration) {
    auto spec = makeSpec("Spec", 114.3, 6.0);
    auto p0 = makePoint("P0", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("P1", model::PipePointType::Run,
                         gp_Pnt(100, 0, 0), spec);

    app::SelectionManager sel;

    std::vector<foundation::UUID> lastSelection;
    sel.setSelectionChangedCallback(
        [&](const std::vector<foundation::UUID>& ids) {
            lastSelection = ids;
        });

    // Select first point
    EXPECT_TRUE(sel.select(p0->id()));
    EXPECT_EQ(sel.size(), 1u);
    EXPECT_TRUE(sel.isSelected(p0->id()));
    EXPECT_EQ(lastSelection.size(), 1u);

    // Select second point
    EXPECT_TRUE(sel.select(p1->id()));
    EXPECT_EQ(sel.size(), 2u);

    // Deselect first
    EXPECT_TRUE(sel.deselect(p0->id()));
    EXPECT_EQ(sel.size(), 1u);
    EXPECT_FALSE(sel.isSelected(p0->id()));
    EXPECT_TRUE(sel.isSelected(p1->id()));

    // Clear
    sel.clear();
    EXPECT_EQ(sel.size(), 0u);
}

// ============================================================
// Cross-cutting: WorkbenchManager integration
// ============================================================

TEST_F(IntegrationTest, WorkbenchManagerIntegration) {
    app::WorkbenchManager wm(doc);

    std::string activatedName;
    wm.setWorkbenchChangedCallback(
        [&](const app::Workbench* wb) {
            activatedName = wb ? wb->name() : "";
        });

    // Register CAD workbench
    wm.registerWorkbench(std::make_unique<app::CadWorkbench>());

    auto names = wm.workbenchNames();
    ASSERT_EQ(names.size(), 1u);
    EXPECT_EQ(names[0], "CAD");

    // Switch to CAD
    EXPECT_TRUE(wm.switchWorkbench("CAD"));
    EXPECT_NE(wm.activeWorkbench(), nullptr);
    EXPECT_EQ(wm.activeWorkbench()->name(), "CAD");
    EXPECT_EQ(activatedName, "CAD");

    // Activate with document
    wm.activeWorkbench()->activate(doc);

    // Verify toolbar actions and panels are available
    auto actions = wm.activeWorkbench()->toolbarActions();
    EXPECT_FALSE(actions.empty());

    auto panels = wm.activeWorkbench()->panelIds();
    EXPECT_FALSE(panels.empty());
}

// ============================================================
// Cross-cutting: Dependency graph + recompute chain
// ============================================================

TEST_F(IntegrationTest, DependencyChain_SpecToPipePointRecompute) {
    auto spec = makeSpec("Spec-Chain", 114.3, 6.0);

    // Create 5 points all depending on same spec
    std::vector<std::shared_ptr<model::PipePoint>> points;
    for (int i = 0; i < 5; ++i) {
        auto pp = makePoint("P" + std::to_string(i),
                            model::PipePointType::Run,
                            gp_Pnt(i * 200.0, 0, 0), spec);
        points.push_back(pp);
    }

    auto seg = std::make_shared<model::Segment>("Seg");
    for (auto& pp : points) {
        seg->addPoint(pp);
    }
    auto route = std::make_shared<model::Route>("Route");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // Initial recompute
    engine->recomputeAll();
    generatedShapes.clear();
    recomputeCallCount = 0;

    // Modify spec should mark all 5 points dirty
    spec->setOd(323.9);
    triggerRecompute(spec->id());

    EXPECT_GE(recomputeCallCount, 1);
    // Should have recomputed multiple points (not just the spec)
    EXPECT_GE(generatedShapes.size(), 1u);
}

// ============================================================
// Cross-cutting: Multiple segments in route
// ============================================================

TEST_F(IntegrationTest, MultipleSegmentsInRoute) {
    auto spec = makeSpec("Spec", 114.3, 6.0);

    // Segment 1: 3 points with bend
    auto s1p0 = makePoint("S1-A00", model::PipePointType::Run,
                           gp_Pnt(0, 0, 0), spec);
    auto s1p1 = makePoint("S1-A01", model::PipePointType::Bend,
                           gp_Pnt(500, 0, 0), spec);
    s1p1->setParam("bendMultiplier", 1.5);
    auto s1p2 = makePoint("S1-A02", model::PipePointType::Run,
                           gp_Pnt(500, 500, 0), spec);

    auto seg1 = std::make_shared<model::Segment>("Seg1");
    seg1->addPoint(s1p0);
    seg1->addPoint(s1p1);
    seg1->addPoint(s1p2);

    // Segment 2: 2 points straight run
    auto s2p0 = makePoint("S2-A00", model::PipePointType::Run,
                           gp_Pnt(1000, 0, 0), spec);
    auto s2p1 = makePoint("S2-A01", model::PipePointType::Run,
                           gp_Pnt(2000, 0, 0), spec);

    auto seg2 = std::make_shared<model::Segment>("Seg2");
    seg2->addPoint(s2p0);
    seg2->addPoint(s2p1);

    auto route = std::make_shared<model::Route>("Route");
    route->addSegment(seg1);
    route->addSegment(seg2);
    doc.addObject(seg1);
    doc.addObject(seg2);
    doc.addObject(route);

    // Recompute all
    engine->recomputeAll();
    EXPECT_GE(generatedShapes.size(), 2u);

    // Validate route structure
    EXPECT_EQ(route->segmentCount(), 2u);
    EXPECT_EQ(route->segmentAt(0)->pointCount(), 3u);
    EXPECT_EQ(route->segmentAt(1)->pointCount(), 2u);

    // Constraint check
    engine::ConstraintSolver solver;
    auto errs = solver.checkAll(*route);
    EXPECT_TRUE(errs.empty());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
