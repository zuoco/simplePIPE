#include <gtest/gtest.h>

#include "engine/TopologyManager.h"
#include "engine/ConstraintSolver.h"
#include "engine/PipelineValidator.h"

#include "model/Route.h"
#include "model/Segment.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"

#include "foundation/Math.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_Ax2.hxx>

using namespace engine;
using namespace model;

// ============================================================
// Helpers
// ============================================================

static std::shared_ptr<PipePoint> makePoint(
    const std::string& name,
    PipePointType type,
    const gp_Pnt& pos = gp_Pnt(0, 0, 0))
{
    return std::make_shared<PipePoint>(name, type, pos);
}

static std::shared_ptr<PipeSpec> makeSpec(double od)
{
    auto spec = std::make_shared<PipeSpec>("spec");
    spec->setOd(od);
    return spec;
}

// ============================================================
// TopologyManager — 基本追加
// ============================================================

TEST(TopologyManager, AppendRunPoint_NoExtraSegment) {
    Route route("R");
    auto seg0 = std::make_shared<Segment>("S0");
    route.addSegment(seg0);

    TopologyManager tm;
    auto pt = makePoint("P1", PipePointType::Run);
    auto branch = tm.appendPoint(route, *seg0, pt);

    EXPECT_EQ(branch, nullptr);
    EXPECT_EQ(route.segmentCount(), 1u);
    EXPECT_EQ(seg0->pointCount(), 1u);
}

// ============================================================
// TopologyManager — 三通自动分叉
// ============================================================

TEST(TopologyManager, AppendTee_CreatesBranchSegment) {
    Route route("R");
    auto seg0 = std::make_shared<Segment>("Main");
    route.addSegment(seg0);

    TopologyManager tm;
    auto tee = makePoint("Tee1", PipePointType::Tee, gp_Pnt(5, 0, 0));
    auto branch = tm.appendPoint(route, *seg0, tee);

    ASSERT_NE(branch, nullptr);
    EXPECT_EQ(route.segmentCount(), 2u);           // main + branch
    EXPECT_EQ(seg0->pointCount(), 1u);             // tee in main segment
    EXPECT_FALSE(tm.branchSegmentId(tee->id()).empty());  // mapping recorded
}

TEST(TopologyManager, TwoBranchesFromTwoTees) {
    Route route("R");
    auto seg0 = std::make_shared<Segment>("Main");
    route.addSegment(seg0);

    TopologyManager tm;
    auto t1 = makePoint("T1", PipePointType::Tee, gp_Pnt(1, 0, 0));
    auto t2 = makePoint("T2", PipePointType::Tee, gp_Pnt(2, 0, 0));

    tm.appendPoint(route, *seg0, t1);
    tm.appendPoint(route, *seg0, t2);

    EXPECT_EQ(route.segmentCount(), 3u);  // main + 2 branches
}

// ============================================================
// TopologyManager — 插入点
// ============================================================

TEST(TopologyManager, InsertTee_AtIndex0) {
    Route route("R");
    auto seg0 = std::make_shared<Segment>("Main");
    route.addSegment(seg0);

    TopologyManager tm;
    auto run = makePoint("R1", PipePointType::Run);
    seg0->addPoint(run);

    auto tee = makePoint("T1", PipePointType::Tee, gp_Pnt(0, 5, 0));
    auto branch = tm.insertPoint(route, *seg0, 0, tee);

    ASSERT_NE(branch, nullptr);
    EXPECT_EQ(route.segmentCount(), 2u);
    EXPECT_EQ(seg0->pointCount(), 2u);
    EXPECT_EQ(seg0->pointAt(0), tee.get());
}

// ============================================================
// TopologyManager — 删除普通点
// ============================================================

TEST(TopologyManager, RemoveRunPoint_SegmentShrinks) {
    Route route("R");
    auto seg0 = std::make_shared<Segment>("Main");
    route.addSegment(seg0);

    TopologyManager tm;
    auto p1 = makePoint("P1", PipePointType::Run);
    auto p2 = makePoint("P2", PipePointType::Run);
    tm.appendPoint(route, *seg0, p1);
    tm.appendPoint(route, *seg0, p2);

    EXPECT_EQ(seg0->pointCount(), 2u);
    bool ok = tm.removePoint(route, p1->id());
    EXPECT_TRUE(ok);
    EXPECT_EQ(seg0->pointCount(), 1u);
    EXPECT_EQ(route.segmentCount(), 1u);  // segment kept (not empty)
}

TEST(TopologyManager, RemoveNonexistentPoint_ReturnsFalse) {
    Route route("R");
    auto seg0 = std::make_shared<Segment>("Main");
    route.addSegment(seg0);

    auto pt = makePoint("P1", PipePointType::Run);
    seg0->addPoint(pt);

    TopologyManager tm;
    auto fake = makePoint("Fake", PipePointType::Run);
    EXPECT_FALSE(tm.removePoint(route, fake->id()));
}

// ============================================================
// TopologyManager — 删除 Tee 同时删除分支
// ============================================================

TEST(TopologyManager, RemoveTee_AlsoRemovesBranchSegment) {
    Route route("R");
    auto seg0 = std::make_shared<Segment>("Main");
    route.addSegment(seg0);

    TopologyManager tm;
    auto tee = makePoint("T1", PipePointType::Tee, gp_Pnt(3, 0, 0));
    tm.appendPoint(route, *seg0, tee);
    EXPECT_EQ(route.segmentCount(), 2u);

    bool ok = tm.removePoint(route, tee->id());
    EXPECT_TRUE(ok);
    EXPECT_EQ(route.segmentCount(), 1u);  // branch removed; one empty seg kept
    EXPECT_TRUE(tm.branchSegmentId(tee->id()).empty());  // mapping cleared
}

// ============================================================
// TopologyManager — segmentsContaining
// ============================================================

TEST(TopologyManager, SegmentsContaining_FindsPoint) {
    Route route("R");
    auto seg0 = std::make_shared<Segment>("S0");
    auto seg1 = std::make_shared<Segment>("S1");
    route.addSegment(seg0);
    route.addSegment(seg1);

    auto pt = makePoint("P1", PipePointType::Run);
    seg0->addPoint(pt);

    TopologyManager tm;
    auto segs = tm.segmentsContaining(route, pt->id());
    ASSERT_EQ(segs.size(), 1u);
    EXPECT_EQ(segs[0], seg0.get());
}

// ============================================================
// ConstraintSolver — 口径匹配
// ============================================================

TEST(ConstraintSolver, DiameterMismatch_Reported) {
    Segment seg("S");
    auto p1 = makePoint("P1", PipePointType::Run, gp_Pnt(0, 0, 0));
    auto p2 = makePoint("P2", PipePointType::Run, gp_Pnt(1, 0, 0));
    p1->setPipeSpec(makeSpec(50.0));
    p2->setPipeSpec(makeSpec(80.0));  // mismatch
    seg.addPoint(p1);
    seg.addPoint(p2);

    ConstraintSolver cs;
    auto errs = cs.checkDiameterConsistency(seg);
    EXPECT_EQ(errs.size(), 1u);
    EXPECT_EQ(errs[0].pointId, p2->id().toString());
}

TEST(ConstraintSolver, DiameterMatch_NoError) {
    Segment seg("S");
    auto p1 = makePoint("P1", PipePointType::Run, gp_Pnt(0, 0, 0));
    auto p2 = makePoint("P2", PipePointType::Run, gp_Pnt(1, 0, 0));
    p1->setPipeSpec(makeSpec(50.0));
    p2->setPipeSpec(makeSpec(50.0));
    seg.addPoint(p1);
    seg.addPoint(p2);

    ConstraintSolver cs;
    EXPECT_TRUE(cs.checkDiameterConsistency(seg).empty());
}

TEST(ConstraintSolver, DiameterMismatch_WithReducer_NoError) {
    Segment seg("S");
    auto p1  = makePoint("P1",  PipePointType::Run,     gp_Pnt(0, 0, 0));
    auto red = makePoint("Red", PipePointType::Reducer,  gp_Pnt(1, 0, 0));
    auto p2  = makePoint("P2",  PipePointType::Run,     gp_Pnt(2, 0, 0));
    p1->setPipeSpec(makeSpec(50.0));
    red->setPipeSpec(makeSpec(50.0));
    p2->setPipeSpec(makeSpec(80.0));
    seg.addPoint(p1);
    seg.addPoint(red);
    seg.addPoint(p2);

    ConstraintSolver cs;
    // P1→Reducer: Reducer present → skip; Reducer→P2: Reducer present → skip
    EXPECT_TRUE(cs.checkDiameterConsistency(seg).empty());
}

TEST(ConstraintSolver, NoSpecSet_DiameterCheckSkips) {
    Segment seg("S");
    auto p1 = makePoint("P1", PipePointType::Run);
    auto p2 = makePoint("P2", PipePointType::Run);
    // no PipeSpec assigned
    seg.addPoint(p1);
    seg.addPoint(p2);

    ConstraintSolver cs;
    EXPECT_TRUE(cs.checkDiameterConsistency(seg).empty());
}

// ============================================================
// ConstraintSolver — 弯头角度
// ============================================================

TEST(ConstraintSolver, ValidBend90_NoError) {
    // A(0,0,0) → B(1,0,0)[Bend] → C(1,1,0)  ← 90° angle at B
    Segment seg("S");
    auto prev = makePoint("Prev", PipePointType::Run,  gp_Pnt(0, 0, 0));
    auto bend = makePoint("Bend", PipePointType::Bend, gp_Pnt(1, 0, 0));
    auto next = makePoint("Next", PipePointType::Run,  gp_Pnt(1, 1, 0));
    seg.addPoint(prev);
    seg.addPoint(bend);
    seg.addPoint(next);

    ConstraintSolver cs;
    EXPECT_TRUE(cs.checkBendAngles(seg).empty());
}

TEST(ConstraintSolver, Collinear_BendReported) {
    // All points on the X-axis: no effective bend
    Segment seg("S");
    auto prev = makePoint("Prev", PipePointType::Run,  gp_Pnt(0, 0, 0));
    auto bend = makePoint("Bend", PipePointType::Bend, gp_Pnt(1, 0, 0));
    auto next = makePoint("Next", PipePointType::Run,  gp_Pnt(2, 0, 0));
    seg.addPoint(prev);
    seg.addPoint(bend);
    seg.addPoint(next);

    ConstraintSolver cs;
    auto errs = cs.checkBendAngles(seg);
    EXPECT_EQ(errs.size(), 1u);
    EXPECT_EQ(errs[0].pointId, bend->id().toString());
}

TEST(ConstraintSolver, BendAtSegmentEnd_Reported) {
    // Bend is the last point — no successor
    Segment seg("S");
    auto prev = makePoint("Prev", PipePointType::Run,  gp_Pnt(0, 0, 0));
    auto bend = makePoint("Bend", PipePointType::Bend, gp_Pnt(1, 0, 0));
    seg.addPoint(prev);
    seg.addPoint(bend);

    ConstraintSolver cs;
    auto errs = cs.checkBendAngles(seg);
    EXPECT_EQ(errs.size(), 1u);
}

TEST(ConstraintSolver, CheckAll_MultipleSegments) {
    Route route("R");
    auto seg0 = std::make_shared<Segment>("S0");
    auto seg1 = std::make_shared<Segment>("S1");
    route.addSegment(seg0);
    route.addSegment(seg1);

    // seg0: OD mismatch
    auto p1 = makePoint("P1", PipePointType::Run);
    auto p2 = makePoint("P2", PipePointType::Run);
    p1->setPipeSpec(makeSpec(50.0));
    p2->setPipeSpec(makeSpec(80.0));
    seg0->addPoint(p1);
    seg0->addPoint(p2);

    // seg1: valid bend
    auto prev = makePoint("Prev", PipePointType::Run,  gp_Pnt(0, 0, 0));
    auto bend = makePoint("Bend", PipePointType::Bend, gp_Pnt(1, 0, 0));
    auto next = makePoint("Next", PipePointType::Run,  gp_Pnt(1, 1, 0));
    seg1->addPoint(prev);
    seg1->addPoint(bend);
    seg1->addPoint(next);

    ConstraintSolver cs;
    auto errs = cs.checkAll(route);
    EXPECT_EQ(errs.size(), 1u);  // only the OD mismatch from seg0
}

// ============================================================
// PipelineValidator — 未连接端口
// ============================================================

TEST(PipelineValidator, SinglePointSegment_Warning) {
    Route route("R");
    auto seg = std::make_shared<Segment>("S1");
    auto pt  = makePoint("P1", PipePointType::Run);
    seg->addPoint(pt);
    route.addSegment(seg);

    PipelineValidator pv;
    auto warns = pv.checkUnconnectedPorts(route);
    EXPECT_EQ(warns.size(), 1u);
    EXPECT_EQ(warns[0].severity, ValidationWarning::Severity::Warning);
    EXPECT_EQ(warns[0].objectId, seg->id().toString());
}

TEST(PipelineValidator, TwoPointSegment_NoWarning) {
    Route route("R");
    auto seg = std::make_shared<Segment>("S1");
    seg->addPoint(makePoint("P1", PipePointType::Run, gp_Pnt(0, 0, 0)));
    seg->addPoint(makePoint("P2", PipePointType::Run, gp_Pnt(1, 0, 0)));
    route.addSegment(seg);

    PipelineValidator pv;
    EXPECT_TRUE(pv.checkUnconnectedPorts(route).empty());
}

TEST(PipelineValidator, EmptySegment_Warning) {
    Route route("R");
    auto seg = std::make_shared<Segment>("Empty");
    route.addSegment(seg);

    PipelineValidator pv;
    auto warns = pv.checkUnconnectedPorts(route);
    EXPECT_EQ(warns.size(), 1u);
}

// ============================================================
// PipelineValidator — 干涉检测
// ============================================================

TEST(PipelineValidator, NoInterference_TwoDistantCylinders) {
    // Two cylinders far apart — no interference
    gp_Ax2 ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    gp_Ax2 ax2(gp_Pnt(100, 0, 0), gp_Dir(0, 0, 1));
    TopoDS_Shape c1 = BRepPrimAPI_MakeCylinder(ax1, 5.0, 20.0).Shape();
    TopoDS_Shape c2 = BRepPrimAPI_MakeCylinder(ax2, 5.0, 20.0).Shape();

    PipelineValidator pv;
    auto warns = pv.checkInterference({c1, c2}, {"obj1", "obj2"});
    EXPECT_TRUE(warns.empty());
}

TEST(PipelineValidator, Interference_OverlappingCylinders) {
    // Two coincident cylinders — definite interference
    gp_Ax2 ax(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    TopoDS_Shape c1 = BRepPrimAPI_MakeCylinder(ax, 5.0, 20.0).Shape();
    TopoDS_Shape c2 = BRepPrimAPI_MakeCylinder(ax, 5.0, 20.0).Shape();

    PipelineValidator pv;
    auto warns = pv.checkInterference({c1, c2}, {"obj1", "obj2"});
    EXPECT_GE(warns.size(), 1u);
    EXPECT_EQ(warns[0].severity, ValidationWarning::Severity::Error);
}

TEST(PipelineValidator, ValidateAll_OnlyStructuralChecks) {
    Route route("R");
    auto seg = std::make_shared<Segment>("S");
    seg->addPoint(makePoint("P1", PipePointType::Run, gp_Pnt(0, 0, 0)));
    seg->addPoint(makePoint("P2", PipePointType::Run, gp_Pnt(1, 0, 0)));
    route.addSegment(seg);

    PipelineValidator pv;
    EXPECT_TRUE(pv.validateAll(route).empty());
}
