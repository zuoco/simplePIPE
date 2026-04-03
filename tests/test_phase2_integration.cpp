// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

/// T45 — 端到端集成测试 (Phase 2)
///
/// 验收场景:
///   1. Spec→Design→Analysis 全流程: PipeSpec → 管路设计(多管点+弯头) → 载荷 → 工况/组合
///   2. 工作台切换: Design ↔ Analysis 来回切换，数据和视口状态正确恢复
///   3. 序列化 round-trip: 保存→加载→所有对象(含载荷/工况)完整恢复
///   4. Undo/Redo: 跨工作台操作的事务回退正确
///   5. 渲染模式切换: Solid ↔ Beam 切换不崩溃
///   6. 编译零错误零警告

#include <gtest/gtest.h>

// Application layer
#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "app/TransactionManager.h"
#include "app/ProjectSerializer.h"
#include "app/SelectionManager.h"
#include "app/StepExporter.h"
#include "app/WorkbenchManager.h"
#include "app/CadWorkbench.h"
#include "app/DesignWorkbench.h"
#include "app/SpecWorkbench.h"
#include "app/AnalysisWorkbench.h"

// Engine layer
#include "engine/RecomputeEngine.h"
#include "engine/BendCalculator.h"
#include "engine/ConstraintSolver.h"
#include "engine/PipelineValidator.h"
#include "engine/ComponentCatalog.h"
#include "engine/ComponentTemplate.h"

// Model layer
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Segment.h"
#include "model/Route.h"
#include "model/ProjectConfig.h"
#include "model/Beam.h"
#include "model/Flange.h"
#include "model/Support.h"
#include "model/Load.h"
#include "model/DeadWeightLoad.h"
#include "model/ThermalLoad.h"
#include "model/PressureLoad.h"
#include "model/LoadCase.h"
#include "model/LoadCombination.h"

// Visualization (ViewManager)
#include "visualization/ViewManager.h"

// Foundation
#include "foundation/Types.h"

// OCCT
#include <gp_Pnt.hxx>
#include <TopoDS_Shape.hxx>

#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

// ============================================================
// Phase 2 integration test fixture
// ============================================================

class Phase2IntegrationTest : public ::testing::Test {
protected:
    app::Document        doc;
    app::DependencyGraph graph;
    std::unique_ptr<app::TransactionManager> txn;
    std::unique_ptr<engine::RecomputeEngine> engine;
    app::WorkbenchManager wm{doc};

    std::map<std::string, TopoDS_Shape> generatedShapes;
    int recomputeCallCount = 0;

    void SetUp() override {
        txn = std::make_unique<app::TransactionManager>(doc, graph);
        engine = std::make_unique<engine::RecomputeEngine>(doc, graph);

        txn->setRecomputeCallback([this](const std::vector<foundation::UUID>& dirtyIds) {
            engine->recompute(dirtyIds);
            ++recomputeCallCount;
        });

        engine->setSceneUpdateCallback(
            [this](const std::string& uuid, const TopoDS_Shape& shape) {
                generatedShapes[uuid] = shape;
            });
    }

    std::shared_ptr<model::PipeSpec> makeSpec(const std::string& name,
                                               double od, double wt) {
        auto spec = std::make_shared<model::PipeSpec>(name);
        spec->setOd(od);
        spec->setWallThickness(wt);
        spec->setMaterial("CS");
        doc.addObject(spec);
        return spec;
    }

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
        return std::string("/tmp/pipecad_phase2_") + suffix;
    }
};

// ============================================================
// 1. Spec→Design→Analysis 全流程
// ============================================================

TEST_F(Phase2IntegrationTest, FullPipeline_SpecToDesignToAnalysis) {
    // --- Phase: Create project config ---
    doc.setName("Phase2-E2E");
    auto cfg = std::make_shared<model::ProjectConfig>("Config");
    cfg->setProjectName("Phase2-E2E");
    cfg->setAuthor("Phase2Test");
    cfg->setStandard("ASME B31.3");
    cfg->setUnitSystem(foundation::UnitSystem::SI);
    doc.addObject(cfg);

    // --- Phase: Spec definition ---
    auto spec = makeSpec("Spec-10inch", 273.0, 9.27);

    // --- Phase: Design pipe route with multiple points and bends ---
    auto p0 = makePoint("A00", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("A01", model::PipePointType::Bend,
                         gp_Pnt(2000, 0, 0), spec);
    p1->setParam("bendMultiplier", 1.5);
    auto p2 = makePoint("A02", model::PipePointType::Run,
                         gp_Pnt(2000, 1500, 0), spec);
    auto p3 = makePoint("A03", model::PipePointType::Bend,
                         gp_Pnt(2000, 3000, 0), spec);
    p3->setParam("bendMultiplier", 1.5);
    auto p4 = makePoint("A04", model::PipePointType::Run,
                         gp_Pnt(4000, 3000, 0), spec);

    auto seg = std::make_shared<model::Segment>("Seg-001");
    seg->addPoint(p0);
    seg->addPoint(p1);
    seg->addPoint(p2);
    seg->addPoint(p3);
    seg->addPoint(p4);

    auto route = std::make_shared<model::Route>("Route-Main");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // Recompute geometry for all route points
    engine->recomputeAll();
    EXPECT_FALSE(generatedShapes.empty());

    // Verify bend calculations
    auto bend1 = engine::BendCalculator::calculateBend(
        p0->position(), p1->position(), p2->position(),
        spec->od(), 1.5);
    ASSERT_TRUE(bend1.has_value());
    EXPECT_NEAR(bend1->bendAngle, M_PI / 2.0, 1e-6);

    auto bend2 = engine::BendCalculator::calculateBend(
        p2->position(), p3->position(), p4->position(),
        spec->od(), 1.5);
    ASSERT_TRUE(bend2.has_value());
    EXPECT_NEAR(bend2->bendAngle, M_PI / 2.0, 1e-6);

    // --- Phase: Add loads ---
    auto deadWeight = std::make_shared<model::DeadWeightLoad>("W1");
    deadWeight->addAffectedObject(p0->id());
    deadWeight->addAffectedObject(p1->id());
    deadWeight->addAffectedObject(p2->id());
    deadWeight->addAffectedObject(p3->id());
    deadWeight->addAffectedObject(p4->id());
    doc.addObject(deadWeight);

    auto thermal = std::make_shared<model::ThermalLoad>("T1");
    thermal->setInstallTemp(20.0);
    thermal->setOperatingTemp(250.0);
    thermal->addAffectedObject(seg->id());
    doc.addObject(thermal);

    auto pressure = std::make_shared<model::PressureLoad>("P1");
    pressure->setPressure(4.5);  // 4.5 MPa internal
    pressure->setIsExternal(false);
    pressure->addAffectedObject(seg->id());
    doc.addObject(pressure);

    // --- Phase: Create load cases ---
    auto lcWeight = std::make_shared<model::LoadCase>("W");
    lcWeight->addEntry({deadWeight->id(), 1.0});
    doc.addObject(lcWeight);

    auto lcThermal = std::make_shared<model::LoadCase>("T1");
    lcThermal->addEntry({thermal->id(), 1.0});
    doc.addObject(lcThermal);

    auto lcPressure = std::make_shared<model::LoadCase>("P1");
    lcPressure->addEntry({pressure->id(), 1.0});
    doc.addObject(lcPressure);

    // --- Phase: Create load combinations (B31.3 style) ---
    auto combSUS = std::make_shared<model::LoadCombination>(
        "SUS", model::StressCategory::Sustained, model::CombineMethod::Algebraic);
    combSUS->addCaseEntry({lcWeight->id(), 1.0});
    combSUS->addCaseEntry({lcPressure->id(), 1.0});
    doc.addObject(combSUS);

    auto combEXP = std::make_shared<model::LoadCombination>(
        "EXP", model::StressCategory::Expansion, model::CombineMethod::Algebraic);
    combEXP->addCaseEntry({lcThermal->id(), 1.0});
    doc.addObject(combEXP);

    auto combOPE = std::make_shared<model::LoadCombination>(
        "OPE", model::StressCategory::Operating, model::CombineMethod::Algebraic);
    combOPE->addCaseEntry({lcWeight->id(), 1.0});
    combOPE->addCaseEntry({lcThermal->id(), 1.0});
    combOPE->addCaseEntry({lcPressure->id(), 1.0});
    doc.addObject(combOPE);

    // Verify total object counts
    auto allLoads = doc.findByType<model::Load>();
    EXPECT_EQ(allLoads.size(), 3u);

    auto allCases = doc.findByType<model::LoadCase>();
    EXPECT_EQ(allCases.size(), 3u);

    auto allCombinations = doc.findByType<model::LoadCombination>();
    EXPECT_EQ(allCombinations.size(), 3u);

    // Verify combination references
    EXPECT_EQ(combSUS->caseEntries().size(), 2u);
    EXPECT_EQ(combEXP->caseEntries().size(), 1u);
    EXPECT_EQ(combOPE->caseEntries().size(), 3u);
}

// ============================================================
// 2. 工作台切换 — Design ↔ Analysis 来回切换
// ============================================================

TEST_F(Phase2IntegrationTest, WorkbenchSwitch_DesignAnalysis) {
    // Register all three workbenches
    wm.registerWorkbench(std::make_unique<app::DesignWorkbench>());
    wm.registerWorkbench(std::make_unique<app::SpecWorkbench>());
    wm.registerWorkbench(std::make_unique<app::AnalysisWorkbench>());

    auto names = wm.workbenchNames();
    EXPECT_EQ(names.size(), 3u);

    std::string lastActivated;
    wm.setWorkbenchChangedCallback([&](const app::Workbench* wb) {
        lastActivated = wb ? wb->name() : "";
    });

    // Switch to Design
    EXPECT_TRUE(wm.switchWorkbench("Design"));
    EXPECT_EQ(lastActivated, "Design");
    ASSERT_NE(wm.activeWorkbench(), nullptr);
    EXPECT_EQ(wm.activeWorkbench()->viewportType(), app::ViewportType::Vsg);

    // Verify Design toolbar and panels
    auto designActions = wm.activeWorkbench()->toolbarActions();
    EXPECT_FALSE(designActions.empty());
    auto designPanels = wm.activeWorkbench()->panelIds();
    EXPECT_FALSE(designPanels.empty());

    // Switch to Analysis
    EXPECT_TRUE(wm.switchWorkbench("Analysis"));
    EXPECT_EQ(lastActivated, "Analysis");
    ASSERT_NE(wm.activeWorkbench(), nullptr);
    EXPECT_EQ(wm.activeWorkbench()->viewportType(), app::ViewportType::Vtk);

    // Verify Analysis toolbar and panels
    auto analysisActions = wm.activeWorkbench()->toolbarActions();
    EXPECT_FALSE(analysisActions.empty());
    auto analysisPanels = wm.activeWorkbench()->panelIds();
    EXPECT_FALSE(analysisPanels.empty());

    // Switch back to Design
    EXPECT_TRUE(wm.switchWorkbench("Design"));
    EXPECT_EQ(lastActivated, "Design");
    EXPECT_EQ(wm.activeWorkbench()->viewportType(), app::ViewportType::Vsg);

    // Switch to Specification
    EXPECT_TRUE(wm.switchWorkbench("Specification"));
    EXPECT_EQ(lastActivated, "Specification");
    EXPECT_EQ(wm.activeWorkbench()->viewportType(), app::ViewportType::Vsg);

    // Switch back to Analysis
    EXPECT_TRUE(wm.switchWorkbench("Analysis"));
    EXPECT_EQ(lastActivated, "Analysis");

    // Invalid workbench name
    EXPECT_FALSE(wm.switchWorkbench("NonExistent"));
}

// ============================================================
// 2b. 工作台切换 + ViewManager 视口状态保存/恢复
// ============================================================

TEST_F(Phase2IntegrationTest, WorkbenchSwitch_ViewManagerState) {
    visualization::ViewManager vm;

    // ViewManager state management without actual VSG components
    vm.setRenderMode(visualization::ViewManager::RenderMode::Solid);
    EXPECT_EQ(vm.renderMode(), visualization::ViewManager::RenderMode::Solid);

    // Save Design viewport state
    vm.saveViewState("Design");

    // Switch render mode for Analysis
    vm.setRenderMode(visualization::ViewManager::RenderMode::Beam);
    EXPECT_EQ(vm.renderMode(), visualization::ViewManager::RenderMode::Beam);
    vm.saveViewState("Analysis");

    // Restore Design state
    vm.restoreViewState("Design");
    // ViewManager restores camera state; render mode changes are separate

    // Restore Analysis state
    vm.restoreViewState("Analysis");

    // Category visibility
    vm.setCategoryVisible(visualization::ViewManager::Category::LoadArrows, true);
    EXPECT_TRUE(vm.isCategoryVisible(visualization::ViewManager::Category::LoadArrows));
    vm.setCategoryVisible(visualization::ViewManager::Category::LoadArrows, false);
    EXPECT_FALSE(vm.isCategoryVisible(visualization::ViewManager::Category::LoadArrows));
}

// ============================================================
// 3. 序列化 round-trip — 含载荷/工况的完整保存+加载
// ============================================================

TEST_F(Phase2IntegrationTest, SerializationRoundTrip_WithLoads) {
    // Build a full model
    doc.setName("RoundTrip-Phase2");

    auto cfg = std::make_shared<model::ProjectConfig>("Config");
    cfg->setProjectName("RoundTrip-Phase2");
    cfg->setAuthor("RoundTripper");
    cfg->setStandard("ASME B31.3");
    cfg->setUnitSystem(foundation::UnitSystem::SI);
    doc.addObject(cfg);

    auto spec = makeSpec("Spec-8inch", 219.1, 8.18);

    // Create route: 3 points, 1 bend
    auto p0 = makePoint("A00", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("A01", model::PipePointType::Bend,
                         gp_Pnt(1500, 0, 0), spec);
    p1->setParam("bendMultiplier", 2.0);
    auto p2 = makePoint("A02", model::PipePointType::Run,
                         gp_Pnt(1500, 2000, 0), spec);

    // Attach flange to p0
    auto flange = std::make_shared<model::Flange>("F1", gp_Pnt(0, 0, 0));
    flange->setRating("300");
    flange->setFaceType("RF");
    flange->setBoltHoleCount(12);
    flange->attachTo(p0);
    p0->addAccessory(flange);
    doc.addObject(flange);

    auto seg = std::make_shared<model::Segment>("Seg-001");
    seg->addPoint(p0);
    seg->addPoint(p1);
    seg->addPoint(p2);

    auto route = std::make_shared<model::Route>("Route-Main");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // Add beam
    auto beam = std::make_shared<model::Beam>("Beam-01");
    beam->setSectionType(model::BeamSectionType::HSection);
    beam->setWidth(150.0);
    beam->setHeight(250.0);
    beam->setStartPoint(p0);
    beam->setEndPoint(p1);
    doc.addObject(beam);

    // Add loads
    auto dw = std::make_shared<model::DeadWeightLoad>("W1");
    dw->addAffectedObject(p0->id());
    dw->addAffectedObject(p1->id());
    doc.addObject(dw);

    auto th = std::make_shared<model::ThermalLoad>("T1");
    th->setInstallTemp(25.0);
    th->setOperatingTemp(300.0);
    th->addAffectedObject(seg->id());
    doc.addObject(th);

    auto pr = std::make_shared<model::PressureLoad>("P1");
    pr->setPressure(6.0);
    pr->setIsExternal(false);
    pr->addAffectedObject(seg->id());
    doc.addObject(pr);

    // Add load cases
    auto lcW = std::make_shared<model::LoadCase>("W");
    lcW->addEntry({dw->id(), 1.0});
    doc.addObject(lcW);

    auto lcT = std::make_shared<model::LoadCase>("T1-Case");
    lcT->addEntry({th->id(), 1.0});
    doc.addObject(lcT);

    auto lcP = std::make_shared<model::LoadCase>("P1-Case");
    lcP->addEntry({pr->id(), 1.0});
    doc.addObject(lcP);

    // Add load combinations
    auto combSUS = std::make_shared<model::LoadCombination>(
        "SUS", model::StressCategory::Sustained, model::CombineMethod::Algebraic);
    combSUS->addCaseEntry({lcW->id(), 1.0});
    combSUS->addCaseEntry({lcP->id(), 1.0});
    doc.addObject(combSUS);

    auto combEXP = std::make_shared<model::LoadCombination>(
        "EXP", model::StressCategory::Expansion, model::CombineMethod::Algebraic);
    combEXP->addCaseEntry({lcT->id(), 1.0});
    doc.addObject(combEXP);

    // --- Save ---
    const std::string path = tempPath("roundtrip_loads.json");
    ASSERT_TRUE(app::ProjectSerializer::save(doc, path));

    // --- Load ---
    app::DependencyGraph loadedGraph;
    auto loaded = app::ProjectSerializer::load(path, &loadedGraph);
    ASSERT_NE(loaded, nullptr);

    // Verify project config
    auto configs = loaded->findByType<model::ProjectConfig>();
    ASSERT_EQ(configs.size(), 1u);
    EXPECT_EQ(configs[0]->projectName(), "RoundTrip-Phase2");
    EXPECT_EQ(configs[0]->author(), "RoundTripper");

    // Verify pipe spec
    auto specs = loaded->findByType<model::PipeSpec>();
    ASSERT_EQ(specs.size(), 1u);
    EXPECT_DOUBLE_EQ(specs[0]->od(), 219.1);
    EXPECT_DOUBLE_EQ(specs[0]->wallThickness(), 8.18);

    // Verify pipe points
    auto points = loaded->allPipePoints();
    EXPECT_EQ(points.size(), 3u);

    // Verify bend params survived
    bool foundBend = false;
    for (auto* pp : points) {
        if (pp->type() == model::PipePointType::Bend) {
            foundBend = true;
            EXPECT_TRUE(pp->hasParam("bendMultiplier"));
            EXPECT_DOUBLE_EQ(
                foundation::variantToDouble(pp->param("bendMultiplier")), 2.0);
        }
    }
    EXPECT_TRUE(foundBend);

    // Verify segments and routes
    auto segments = loaded->allSegments();
    EXPECT_EQ(segments.size(), 1u);
    EXPECT_EQ(segments[0]->pointCount(), 3u);

    auto routes = loaded->findByType<model::Route>();
    EXPECT_EQ(routes.size(), 1u);

    // Verify beam
    auto beams = loaded->findByType<model::Beam>();
    EXPECT_EQ(beams.size(), 1u);
    EXPECT_DOUBLE_EQ(beams[0]->width(), 150.0);
    EXPECT_DOUBLE_EQ(beams[0]->height(), 250.0);

    // Verify loads
    auto loads = loaded->findByType<model::Load>();
    EXPECT_EQ(loads.size(), 3u);

    // Verify load types
    bool hasDW = false, hasThermal = false, hasPressure = false;
    for (auto* l : loads) {
        if (l->loadType() == "DeadWeight") hasDW = true;
        if (l->loadType() == "Thermal") hasThermal = true;
        if (l->loadType() == "Pressure") hasPressure = true;
    }
    EXPECT_TRUE(hasDW);
    EXPECT_TRUE(hasThermal);
    EXPECT_TRUE(hasPressure);

    // Verify thermal load values
    for (auto* l : loads) {
        if (l->loadType() == "Thermal") {
            auto* tl = dynamic_cast<model::ThermalLoad*>(l);
            ASSERT_NE(tl, nullptr);
            EXPECT_DOUBLE_EQ(tl->installTemp(), 25.0);
            EXPECT_DOUBLE_EQ(tl->operatingTemp(), 300.0);
        }
        if (l->loadType() == "Pressure") {
            auto* pl = dynamic_cast<model::PressureLoad*>(l);
            ASSERT_NE(pl, nullptr);
            EXPECT_DOUBLE_EQ(pl->pressure(), 6.0);
            EXPECT_FALSE(pl->isExternal());
        }
    }

    // Verify load cases
    auto cases = loaded->findByType<model::LoadCase>();
    EXPECT_EQ(cases.size(), 3u);

    // Verify load case entries (each has 1 entry)
    for (auto* lc : cases) {
        EXPECT_GE(lc->entries().size(), 1u);
    }

    // Verify load combinations
    auto combos = loaded->findByType<model::LoadCombination>();
    EXPECT_EQ(combos.size(), 2u);

    // Verify SUS combination has 2 entries and EXP has 1
    bool foundSUS = false, foundEXP = false;
    for (auto* c : combos) {
        if (c->name() == "SUS") {
            foundSUS = true;
            EXPECT_EQ(c->caseEntries().size(), 2u);
            EXPECT_EQ(c->category(), model::StressCategory::Sustained);
            EXPECT_EQ(c->method(), model::CombineMethod::Algebraic);
        }
        if (c->name() == "EXP") {
            foundEXP = true;
            EXPECT_EQ(c->caseEntries().size(), 1u);
            EXPECT_EQ(c->category(), model::StressCategory::Expansion);
        }
    }
    EXPECT_TRUE(foundSUS);
    EXPECT_TRUE(foundEXP);

    // --- Verify recompute on loaded document ---
    engine::RecomputeEngine loadedEngine(*loaded, loadedGraph);
    std::map<std::string, TopoDS_Shape> loadedShapes;
    loadedEngine.setSceneUpdateCallback(
        [&](const std::string& uuid, const TopoDS_Shape& shape) {
            loadedShapes[uuid] = shape;
        });
    loadedEngine.recomputeAll();
    EXPECT_FALSE(loadedShapes.empty());

    // Cleanup
    std::remove(path.c_str());
}

// ============================================================
// 3b. 序列化 round-trip — 第二次保存与首次一致 (idempotent)
// ============================================================

TEST_F(Phase2IntegrationTest, SerializationRoundTrip_Idempotent) {
    doc.setName("Idempotent-Test");

    auto spec = makeSpec("Spec", 114.3, 6.0);
    auto p0 = makePoint("X0", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("X1", model::PipePointType::Run,
                         gp_Pnt(500, 0, 0), spec);

    auto seg = std::make_shared<model::Segment>("S");
    seg->addPoint(p0);
    seg->addPoint(p1);
    auto route = std::make_shared<model::Route>("R");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // Add a load and case
    auto dw = std::make_shared<model::DeadWeightLoad>("DW");
    doc.addObject(dw);
    auto lc = std::make_shared<model::LoadCase>("LC1");
    lc->addEntry({dw->id(), 1.0});
    doc.addObject(lc);

    // First save
    const std::string path1 = tempPath("idem1.json");
    ASSERT_TRUE(app::ProjectSerializer::save(doc, path1));

    // Load
    auto loaded = app::ProjectSerializer::load(path1);
    ASSERT_NE(loaded, nullptr);

    // Second save from loaded doc
    const std::string path2 = tempPath("idem2.json");
    ASSERT_TRUE(app::ProjectSerializer::save(*loaded, path2));

    // Load the second save
    auto loaded2 = app::ProjectSerializer::load(path2);
    ASSERT_NE(loaded2, nullptr);

    // Verify counts match original
    EXPECT_EQ(loaded2->allPipePoints().size(), 2u);
    EXPECT_EQ(loaded2->findByType<model::PipeSpec>().size(), 1u);
    EXPECT_EQ(loaded2->findByType<model::LoadCase>().size(), 1u);
    EXPECT_EQ(loaded2->findByType<model::Load>().size(), 1u);

    std::remove(path1.c_str());
    std::remove(path2.c_str());
}

// ============================================================
// 4. Undo/Redo — 跨工作台操作的事务回退
// ============================================================

TEST_F(Phase2IntegrationTest, UndoRedo_CrossWorkbench) {
    // Register workbenches
    wm.registerWorkbench(std::make_unique<app::DesignWorkbench>());
    wm.registerWorkbench(std::make_unique<app::AnalysisWorkbench>());

    auto spec = makeSpec("Spec-Undo", 168.3, 7.11);

    auto p0 = makePoint("U0", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("U1", model::PipePointType::Run,
                         gp_Pnt(1000, 0, 0), spec);

    auto seg = std::make_shared<model::Segment>("S-Undo");
    seg->addPoint(p0);
    seg->addPoint(p1);
    auto route = std::make_shared<model::Route>("R-Undo");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // Initial recompute
    engine->recomputeAll();

    // --- In Design workbench: modify OD ---
    wm.switchWorkbench("Design");

    txn->open("Design: 修改OD为219.1");
    double oldOd = spec->od();
    spec->setOd(219.1);
    txn->recordChange(spec->id(), "OD", oldOd, 219.1);
    txn->commit();
    EXPECT_DOUBLE_EQ(spec->od(), 219.1);

    // --- Switch to Analysis workbench: modify wall thickness ---
    wm.switchWorkbench("Analysis");

    txn->open("Analysis: 修改壁厚为12.0");
    double oldWT = spec->wallThickness();
    spec->setWallThickness(12.0);
    txn->recordChange(spec->id(), "wallThickness", oldWT, 12.0);
    txn->commit();
    EXPECT_DOUBLE_EQ(spec->wallThickness(), 12.0);

    // --- Undo Analysis change ---
    txn->undo();
    EXPECT_DOUBLE_EQ(spec->wallThickness(), 7.11);
    EXPECT_DOUBLE_EQ(spec->od(), 219.1);  // Design change unaffected

    // --- Undo Design change ---
    txn->undo();
    EXPECT_DOUBLE_EQ(spec->od(), 168.3);
    EXPECT_DOUBLE_EQ(spec->wallThickness(), 7.11);

    // --- Redo both ---
    txn->redo();
    EXPECT_DOUBLE_EQ(spec->od(), 219.1);

    txn->redo();
    EXPECT_DOUBLE_EQ(spec->wallThickness(), 12.0);

    // --- New change after undo should clear redo stack ---
    txn->undo();
    txn->undo();
    EXPECT_FALSE(txn->canUndo());
    EXPECT_TRUE(txn->canRedo());

    txn->open("New: 设置OD为273.0");
    spec->setOd(273.0);
    txn->recordChange(spec->id(), "OD", 168.3, 273.0);
    txn->commit();
    EXPECT_FALSE(txn->canRedo());
    EXPECT_DOUBLE_EQ(spec->od(), 273.0);
}

// ============================================================
// 4b. Undo/Redo — 载荷参数事务回退
// ============================================================

TEST_F(Phase2IntegrationTest, UndoRedo_LoadParameters) {
    auto th = std::make_shared<model::ThermalLoad>("Thermal-Undo");
    th->setOperatingTemp(200.0);
    doc.addObject(th);

    // Modify operating temp via transaction
    txn->open("修改操作温度");
    double oldTemp = th->operatingTemp();
    th->setOperatingTemp(350.0);
    txn->recordChange(th->id(), "operatingTemp", oldTemp, 350.0);
    txn->commit();
    EXPECT_DOUBLE_EQ(th->operatingTemp(), 350.0);

    // Undo
    txn->undo();
    EXPECT_DOUBLE_EQ(th->operatingTemp(), 200.0);

    // Redo
    txn->redo();
    EXPECT_DOUBLE_EQ(th->operatingTemp(), 350.0);
}

// ============================================================
// 5. 渲染模式切换 — AnalysisWorkbench Solid ↔ Beam
// ============================================================

TEST_F(Phase2IntegrationTest, RenderModeSwitch_SolidBeam) {
    wm.registerWorkbench(std::make_unique<app::AnalysisWorkbench>());
    EXPECT_TRUE(wm.switchWorkbench("Analysis"));

    auto* analysis = dynamic_cast<app::AnalysisWorkbench*>(wm.activeWorkbench());
    ASSERT_NE(analysis, nullptr);

    // Default should be Solid
    EXPECT_EQ(analysis->renderMode(), app::RenderMode::Solid);

    // Switch to Beam
    analysis->setRenderMode(app::RenderMode::Beam);
    EXPECT_EQ(analysis->renderMode(), app::RenderMode::Beam);

    // Switch back to Solid
    analysis->setRenderMode(app::RenderMode::Solid);
    EXPECT_EQ(analysis->renderMode(), app::RenderMode::Solid);

    // Deactivate and reactivate — mode should persist
    analysis->deactivate(doc);
    analysis->setRenderMode(app::RenderMode::Beam);
    analysis->activate(doc);
    EXPECT_EQ(analysis->renderMode(), app::RenderMode::Beam);
}

// ============================================================
// 5b. 渲染模式与 ViewManager 联动
// ============================================================

TEST_F(Phase2IntegrationTest, RenderMode_ViewManagerIntegration) {
    visualization::ViewManager vm;

    // Solid mode
    vm.setRenderMode(visualization::ViewManager::RenderMode::Solid);
    EXPECT_EQ(vm.renderMode(), visualization::ViewManager::RenderMode::Solid);

    // Switch to Beam
    vm.setRenderMode(visualization::ViewManager::RenderMode::Beam);
    EXPECT_EQ(vm.renderMode(), visualization::ViewManager::RenderMode::Beam);

    // Switch to Wireframe
    vm.setRenderMode(visualization::ViewManager::RenderMode::Wireframe);
    EXPECT_EQ(vm.renderMode(), visualization::ViewManager::RenderMode::Wireframe);

    // SolidWithEdges
    vm.setRenderMode(visualization::ViewManager::RenderMode::SolidWithEdges);
    EXPECT_EQ(vm.renderMode(), visualization::ViewManager::RenderMode::SolidWithEdges);
}

// ============================================================
// 6. ComponentCatalog 集成 — 全流程中使用模板库
// ============================================================

TEST_F(Phase2IntegrationTest, ComponentCatalog_Integration) {
    auto& catalog = engine::ComponentCatalog::instance();

    // Built-in templates should be registered
    EXPECT_GE(catalog.size(), 8u);

    // Verify key template IDs exist
    auto ids = catalog.allTemplateIds();
    bool hasPipe = false, hasElbow = false, hasGateValve = false;
    for (const auto& id : ids) {
        if (id == "Pipe") hasPipe = true;
        if (id == "Elbow") hasElbow = true;
        if (id == "GateValve") hasGateValve = true;
    }
    EXPECT_TRUE(hasPipe);
    EXPECT_TRUE(hasElbow);
    EXPECT_TRUE(hasGateValve);

    // Use Pipe template with standard params
    auto* pipeTpl = catalog.getTemplate("Pipe");
    ASSERT_NE(pipeTpl, nullptr);
    auto params = pipeTpl->deriveParams(168.3, 7.11);
    EXPECT_GT(params.bodyLength, 0.0);
    auto pipeShape = pipeTpl->buildShape(params);
    EXPECT_FALSE(pipeShape.IsNull());

    // Use Elbow template
    auto* elbowTpl = catalog.getTemplate("Elbow");
    ASSERT_NE(elbowTpl, nullptr);
    auto elbowParams = elbowTpl->deriveParams(168.3, 7.11);
    auto elbowShape = elbowTpl->buildShape(elbowParams);
    EXPECT_FALSE(elbowShape.IsNull());
}

// ============================================================
// 7. STEP 导出 — Phase 2 全模型导出
// ============================================================

TEST_F(Phase2IntegrationTest, StepExport_FullModel) {
    auto spec = makeSpec("Export-Spec", 114.3, 6.0);

    auto p0 = makePoint("E0", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("E1", model::PipePointType::Bend,
                         gp_Pnt(800, 0, 0), spec);
    p1->setParam("bendMultiplier", 1.5);
    auto p2 = makePoint("E2", model::PipePointType::Run,
                         gp_Pnt(800, 600, 0), spec);

    auto seg = std::make_shared<model::Segment>("ExportSeg");
    seg->addPoint(p0);
    seg->addPoint(p1);
    seg->addPoint(p2);

    auto route = std::make_shared<model::Route>("ExportRoute");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    const std::string stepPath = tempPath("phase2_export.step");
    ASSERT_TRUE(app::StepExporter::exportAll(doc, stepPath));

    // Verify the file exists and is reasonably sized
    std::ifstream file(stepPath, std::ios::binary | std::ios::ate);
    ASSERT_TRUE(file.is_open());
    auto fileSize = file.tellg();
    EXPECT_GT(fileSize, 100);  // Non-trivial STEP file

    std::remove(stepPath.c_str());
}

// ============================================================
// 8. 验证 + 约束检查集成
// ============================================================

TEST_F(Phase2IntegrationTest, Validation_ConstraintCheck) {
    auto spec = makeSpec("Val-Spec", 168.3, 7.11);

    auto p0 = makePoint("V0", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("V1", model::PipePointType::Bend,
                         gp_Pnt(1000, 0, 0), spec);
    p1->setParam("bendMultiplier", 1.5);
    auto p2 = makePoint("V2", model::PipePointType::Run,
                         gp_Pnt(1000, 800, 0), spec);

    auto seg = std::make_shared<model::Segment>("ValSeg");
    seg->addPoint(p0);
    seg->addPoint(p1);
    seg->addPoint(p2);

    auto route = std::make_shared<model::Route>("ValRoute");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // Constraint checks should pass for valid pipeline
    engine::ConstraintSolver solver;
    auto errors = solver.checkAll(*route);
    EXPECT_TRUE(errors.empty());

    // Pipeline validation should pass
    engine::PipelineValidator validator;
    auto warnings = validator.validateAll(*route);
    EXPECT_TRUE(warnings.empty());
}

// ============================================================
// 9. Selection 跨工作台 — 选择状态管理
// ============================================================

TEST_F(Phase2IntegrationTest, Selection_CrossWorkbench) {
    wm.registerWorkbench(std::make_unique<app::DesignWorkbench>());
    wm.registerWorkbench(std::make_unique<app::AnalysisWorkbench>());

    auto spec = makeSpec("Sel-Spec", 114.3, 6.0);
    auto p0 = makePoint("S0", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("S1", model::PipePointType::Run,
                         gp_Pnt(500, 0, 0), spec);

    app::SelectionManager sel;

    // Select in Design
    wm.switchWorkbench("Design");
    sel.select(p0->id());
    EXPECT_EQ(sel.size(), 1u);
    EXPECT_TRUE(sel.isSelected(p0->id()));

    // Switch to Analysis — selection persists (SelectionManager is global)
    wm.switchWorkbench("Analysis");
    EXPECT_EQ(sel.size(), 1u);
    EXPECT_TRUE(sel.isSelected(p0->id()));

    // Add selection in Analysis
    sel.select(p1->id());
    EXPECT_EQ(sel.size(), 2u);

    // Switch back to Design — both selected
    wm.switchWorkbench("Design");
    EXPECT_EQ(sel.size(), 2u);

    // Clear selection
    sel.clear();
    EXPECT_EQ(sel.size(), 0u);
}

// ============================================================
// 10. 依赖图 + 载荷链 — rebuildLoadDependencyChain
// ============================================================

TEST_F(Phase2IntegrationTest, DependencyGraph_LoadChain) {
    auto dw = std::make_shared<model::DeadWeightLoad>("DW");
    doc.addObject(dw);

    auto lc = std::make_shared<model::LoadCase>("LC");
    lc->addEntry({dw->id(), 1.0});
    doc.addObject(lc);

    auto combo = std::make_shared<model::LoadCombination>(
        "SUS", model::StressCategory::Sustained, model::CombineMethod::Algebraic);
    combo->addCaseEntry({lc->id(), 1.0});
    doc.addObject(combo);

    // Rebuild load dependency chain
    graph.rebuildLoadDependencyChain(doc);

    // Mark the load dirty — should propagate to case and combination
    graph.markDirty(dw->id());
    EXPECT_TRUE(graph.isDirty(dw->id()));
    EXPECT_TRUE(graph.isDirty(lc->id()));
    EXPECT_TRUE(graph.isDirty(combo->id()));

    auto dirtyIds = graph.collectDirty();
    EXPECT_GE(dirtyIds.size(), 3u);

    graph.clearDirty();
    EXPECT_EQ(graph.dirtyCount(), 0u);
}

// ============================================================
// 11. 端到端全流程: Spec→Design→Analysis→Save→Load→STEP
// ============================================================

TEST_F(Phase2IntegrationTest, EndToEnd_Phase2FullPipeline) {
    // --- Setup workbenches ---
    wm.registerWorkbench(std::make_unique<app::DesignWorkbench>());
    wm.registerWorkbench(std::make_unique<app::SpecWorkbench>());
    wm.registerWorkbench(std::make_unique<app::AnalysisWorkbench>());

    // --- Phase 1: Project setup (Specification workbench) ---
    wm.switchWorkbench("Specification");

    doc.setName("E2E-Phase2-Full");
    auto cfg = std::make_shared<model::ProjectConfig>("Config");
    cfg->setProjectName("E2E-Phase2-Full");
    cfg->setAuthor("FullTest");
    cfg->setStandard("ASME B31.3");
    cfg->setUnitSystem(foundation::UnitSystem::SI);
    doc.addObject(cfg);

    auto spec = makeSpec("Spec-Main", 219.1, 8.18);

    // --- Phase 2: Design route (Design workbench) ---
    wm.switchWorkbench("Design");

    auto p0 = makePoint("P00", model::PipePointType::Run,
                         gp_Pnt(0, 0, 0), spec);
    auto p1 = makePoint("P01", model::PipePointType::Bend,
                         gp_Pnt(2000, 0, 0), spec);
    p1->setParam("bendMultiplier", 1.5);
    auto p2 = makePoint("P02", model::PipePointType::Run,
                         gp_Pnt(2000, 1500, 0), spec);
    auto p3 = makePoint("P03", model::PipePointType::Bend,
                         gp_Pnt(2000, 3000, 0), spec);
    p3->setParam("bendMultiplier", 2.0);
    auto p4 = makePoint("P04", model::PipePointType::Run,
                         gp_Pnt(4000, 3000, 0), spec);

    auto seg = std::make_shared<model::Segment>("MainSeg");
    seg->addPoint(p0);
    seg->addPoint(p1);
    seg->addPoint(p2);
    seg->addPoint(p3);
    seg->addPoint(p4);

    auto route = std::make_shared<model::Route>("MainRoute");
    route->addSegment(seg);
    doc.addObject(seg);
    doc.addObject(route);

    // Recompute geometry
    engine->recomputeAll();
    size_t shapeCount = generatedShapes.size();
    EXPECT_GE(shapeCount, 2u);

    // Validate design
    engine::ConstraintSolver solver;
    auto errors = solver.checkAll(*route);
    EXPECT_TRUE(errors.empty());

    // --- Phase 3: Modify OD via transaction ---
    generatedShapes.clear();
    txn->open("修改OD");
    double oldOd = spec->od();
    spec->setOd(273.0);
    txn->recordChange(spec->id(), "OD", oldOd, 273.0);
    txn->commit();
    EXPECT_DOUBLE_EQ(spec->od(), 273.0);
    EXPECT_FALSE(generatedShapes.empty());

    // --- Phase 4: Switch to Analysis, add loads ---
    wm.switchWorkbench("Analysis");

    auto* analysisWB = dynamic_cast<app::AnalysisWorkbench*>(wm.activeWorkbench());
    ASSERT_NE(analysisWB, nullptr);
    EXPECT_EQ(analysisWB->renderMode(), app::RenderMode::Solid);

    // Add loads
    auto dw = std::make_shared<model::DeadWeightLoad>("DW");
    for (size_t i = 0; i < seg->pointCount(); ++i)
        dw->addAffectedObject(seg->pointAt(i)->id());
    doc.addObject(dw);

    auto th = std::make_shared<model::ThermalLoad>("T1");
    th->setInstallTemp(20.0);
    th->setOperatingTemp(350.0);
    doc.addObject(th);

    auto pr = std::make_shared<model::PressureLoad>("P1");
    pr->setPressure(8.0);
    doc.addObject(pr);

    // Load cases
    auto lcW = std::make_shared<model::LoadCase>("W");
    lcW->addEntry({dw->id(), 1.0});
    doc.addObject(lcW);

    auto lcT = std::make_shared<model::LoadCase>("T1");
    lcT->addEntry({th->id(), 1.0});
    doc.addObject(lcT);

    auto lcP = std::make_shared<model::LoadCase>("P1");
    lcP->addEntry({pr->id(), 1.0});
    doc.addObject(lcP);

    // Load combinations
    auto combSUS = std::make_shared<model::LoadCombination>(
        "SUS", model::StressCategory::Sustained, model::CombineMethod::Algebraic);
    combSUS->addCaseEntry({lcW->id(), 1.0});
    combSUS->addCaseEntry({lcP->id(), 1.0});
    doc.addObject(combSUS);

    auto combEXP = std::make_shared<model::LoadCombination>(
        "EXP", model::StressCategory::Expansion, model::CombineMethod::Algebraic);
    combEXP->addCaseEntry({lcT->id(), 1.0});
    doc.addObject(combEXP);

    // Switch render mode
    analysisWB->setRenderMode(app::RenderMode::Beam);
    EXPECT_EQ(analysisWB->renderMode(), app::RenderMode::Beam);
    analysisWB->setRenderMode(app::RenderMode::Solid);
    EXPECT_EQ(analysisWB->renderMode(), app::RenderMode::Solid);

    // --- Phase 5: Undo OD change ---
    txn->undo();
    EXPECT_DOUBLE_EQ(spec->od(), 219.1);

    // Redo
    txn->redo();
    EXPECT_DOUBLE_EQ(spec->od(), 273.0);

    // --- Phase 6: Save ---
    const std::string jsonPath = tempPath("e2e_full.json");
    ASSERT_TRUE(app::ProjectSerializer::save(doc, jsonPath));

    // --- Phase 7: Load ---
    app::DependencyGraph loadedGraph;
    auto loaded = app::ProjectSerializer::load(jsonPath, &loadedGraph);
    ASSERT_NE(loaded, nullptr);

    // Verify loaded document
    EXPECT_EQ(loaded->allPipePoints().size(), 5u);
    EXPECT_EQ(loaded->allSegments().size(), 1u);
    EXPECT_EQ(loaded->findByType<model::Route>().size(), 1u);
    EXPECT_EQ(loaded->findByType<model::Load>().size(), 3u);
    EXPECT_EQ(loaded->findByType<model::LoadCase>().size(), 3u);
    EXPECT_EQ(loaded->findByType<model::LoadCombination>().size(), 2u);

    auto loadedSpecs = loaded->findByType<model::PipeSpec>();
    ASSERT_EQ(loadedSpecs.size(), 1u);
    EXPECT_DOUBLE_EQ(loadedSpecs[0]->od(), 273.0);

    // Recompute on loaded document
    engine::RecomputeEngine loadedEngine(*loaded, loadedGraph);
    std::map<std::string, TopoDS_Shape> loadedShapes;
    loadedEngine.setSceneUpdateCallback(
        [&](const std::string& uuid, const TopoDS_Shape& shape) {
            loadedShapes[uuid] = shape;
        });
    loadedEngine.recomputeAll();
    EXPECT_FALSE(loadedShapes.empty());

    // --- Phase 8: STEP export from loaded document ---
    const std::string stepPath = tempPath("e2e_full.step");
    ASSERT_TRUE(app::StepExporter::exportAll(*loaded, stepPath));

    std::ifstream file(stepPath, std::ios::binary | std::ios::ate);
    ASSERT_TRUE(file.is_open());
    EXPECT_GT(file.tellg(), 100);

    // Cleanup
    std::remove(jsonPath.c_str());
    std::remove(stepPath.c_str());
}
