#include <gtest/gtest.h>

#include "model/DocumentObject.h"
#include "model/SpatialObject.h"
#include "model/PropertyObject.h"
#include "model/ContainerObject.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/ProjectConfig.h"
#include "model/Segment.h"
#include "model/Route.h"

#include <memory>
#include <string>

// ============================================================
// DocumentObject Tests
// ============================================================

TEST(DocumentObject, UUID_IsUnique) {
    model::PipeSpec a("spec1");
    model::PipeSpec b("spec2");
    EXPECT_NE(a.id(), b.id());
    EXPECT_FALSE(a.id().isNull());
}

TEST(DocumentObject, Name_SetGet) {
    model::PipeSpec obj("initial");
    EXPECT_EQ(obj.name(), "initial");
    obj.setName("changed");
    EXPECT_EQ(obj.name(), "changed");
}

TEST(DocumentObject, Changed_Signal_OnNameChange) {
    model::PipeSpec obj("test");
    int count = 0;
    obj.changed.connect([&]() { ++count; });
    obj.setName("new");
    EXPECT_EQ(count, 1);
    obj.setName("new"); // 相同值不触发
    EXPECT_EQ(count, 1);
}

// ============================================================
// SpatialObject Tests
// ============================================================

TEST(SpatialObject, Position_SetGet) {
    model::PipePoint pt("A01", model::PipePointType::Run, gp_Pnt(1.0, 2.0, 3.0));
    EXPECT_NEAR(pt.position().X(), 1.0, 1e-12);
    EXPECT_NEAR(pt.position().Y(), 2.0, 1e-12);
    EXPECT_NEAR(pt.position().Z(), 3.0, 1e-12);
    pt.setPosition(gp_Pnt(4.0, 5.0, 6.0));
    EXPECT_NEAR(pt.position().X(), 4.0, 1e-12);
}

TEST(SpatialObject, Changed_Signal_OnPositionChange) {
    model::PipePoint pt("A01");
    int count = 0;
    pt.changed.connect([&]() { ++count; });
    pt.setPosition(gp_Pnt(100, 200, 300));
    EXPECT_EQ(count, 1);
    pt.setPosition(gp_Pnt(100, 200, 300)); // 不触发
    EXPECT_EQ(count, 1);
}

// ============================================================
// PropertyObject / PipeSpec Tests
// ============================================================

TEST(PipeSpec, Fields_SetGet_Double) {
    model::PipeSpec spec("6inch-Sch40");
    spec.setOd(168.3);
    spec.setWallThickness(7.11);
    EXPECT_NEAR(spec.od(), 168.3, 1e-12);
    EXPECT_NEAR(spec.wallThickness(), 7.11, 1e-12);
}

TEST(PipeSpec, Fields_SetGet_String) {
    model::PipeSpec spec("spec");
    spec.setMaterial("Carbon Steel");
    EXPECT_EQ(spec.material(), "Carbon Steel");
}

TEST(PipeSpec, Fields_Extensible) {
    model::PipeSpec spec("spec");
    spec.setField("designPressure", 10.0);
    spec.setField("designTemp", 350.0);
    EXPECT_TRUE(spec.hasField("designPressure"));
    EXPECT_NEAR(foundation::variantToDouble(spec.field("designPressure")), 10.0, 1e-12);
}

TEST(PipeSpec, RemoveField) {
    model::PipeSpec spec("spec");
    spec.setField("OD", 100.0);
    EXPECT_TRUE(spec.hasField("OD"));
    spec.removeField("OD");
    EXPECT_FALSE(spec.hasField("OD"));
}

TEST(PipeSpec, Changed_Signal_OnFieldChange) {
    model::PipeSpec spec("spec");
    int count = 0;
    spec.changed.connect([&]() { ++count; });
    spec.setOd(168.3);
    EXPECT_EQ(count, 1);
    spec.setWallThickness(7.11);
    EXPECT_EQ(count, 2);
}

// ============================================================
// PipePoint Tests
// ============================================================

TEST(PipePoint, Type_SetGet) {
    model::PipePoint pt("A06", model::PipePointType::Bend);
    EXPECT_EQ(pt.type(), model::PipePointType::Bend);
    pt.setType(model::PipePointType::Tee);
    EXPECT_EQ(pt.type(), model::PipePointType::Tee);
}

TEST(PipePoint, PipeSpec_Reference) {
    auto spec = std::make_shared<model::PipeSpec>("6inch-Sch40");
    spec->setOd(168.3);
    spec->setWallThickness(7.11);

    model::PipePoint pt("A01");
    pt.setPipeSpec(spec);

    ASSERT_NE(pt.pipeSpec(), nullptr);
    EXPECT_NEAR(pt.pipeSpec()->od(), 168.3, 1e-12);
    EXPECT_NEAR(pt.pipeSpec()->wallThickness(), 7.11, 1e-12);
}

TEST(PipePoint, TypeParams_Bend) {
    model::PipePoint pt("A06", model::PipePointType::Bend);
    pt.setParam("bendMultiplier", 1.5);
    EXPECT_TRUE(pt.hasParam("bendMultiplier"));
    EXPECT_NEAR(foundation::variantToDouble(pt.param("bendMultiplier")), 1.5, 1e-12);
}

TEST(PipePoint, TypeParams_Valve) {
    model::PipePoint pt("V01", model::PipePointType::Valve);
    pt.setParam("valveType", std::string("Gate"));
    EXPECT_EQ(foundation::variantToString(pt.param("valveType")), "Gate");
}

// ============================================================
// ProjectConfig Tests
// ============================================================

TEST(ProjectConfig, BasicFields) {
    model::ProjectConfig cfg("My Project");
    cfg.setProjectName("PipeCAD Test");
    cfg.setAuthor("Test Engineer");
    cfg.setStandard("ASME B31.3");
    EXPECT_EQ(cfg.projectName(), "PipeCAD Test");
    EXPECT_EQ(cfg.author(), "Test Engineer");
    EXPECT_EQ(cfg.standard(), "ASME B31.3");
}

TEST(ProjectConfig, UnitSystem) {
    model::ProjectConfig cfg;
    EXPECT_EQ(cfg.unitSystem(), foundation::UnitSystem::SI);
    cfg.setUnitSystem(foundation::UnitSystem::Imperial);
    EXPECT_EQ(cfg.unitSystem(), foundation::UnitSystem::Imperial);
}

// ============================================================
// Segment Tests
// ============================================================

TEST(Segment, AddPoints_Ordered) {
    auto seg = model::Segment("SEG-001");
    auto p1 = std::make_shared<model::PipePoint>("A01", model::PipePointType::Run, gp_Pnt(0, 0, 0));
    auto p2 = std::make_shared<model::PipePoint>("A02", model::PipePointType::Run, gp_Pnt(1000, 0, 0));
    auto p3 = std::make_shared<model::PipePoint>("A03", model::PipePointType::Run, gp_Pnt(2000, 0, 0));

    seg.addPoint(p1);
    seg.addPoint(p2);
    seg.addPoint(p3);

    EXPECT_EQ(seg.pointCount(), 3u);
    EXPECT_EQ(seg.pointAt(0)->name(), "A01");
    EXPECT_EQ(seg.pointAt(1)->name(), "A02");
    EXPECT_EQ(seg.pointAt(2)->name(), "A03");
}

TEST(Segment, InsertPoint) {
    auto seg = model::Segment("SEG-001");
    auto p1 = std::make_shared<model::PipePoint>("A01");
    auto p3 = std::make_shared<model::PipePoint>("A03");
    seg.addPoint(p1);
    seg.addPoint(p3);

    auto p2 = std::make_shared<model::PipePoint>("A02");
    seg.insertPoint(1, p2);

    EXPECT_EQ(seg.pointCount(), 3u);
    EXPECT_EQ(seg.pointAt(0)->name(), "A01");
    EXPECT_EQ(seg.pointAt(1)->name(), "A02");
    EXPECT_EQ(seg.pointAt(2)->name(), "A03");
}

TEST(Segment, RemovePoint) {
    auto seg = model::Segment("SEG-001");
    auto p1 = std::make_shared<model::PipePoint>("A01");
    auto p2 = std::make_shared<model::PipePoint>("A02");
    seg.addPoint(p1);
    seg.addPoint(p2);

    EXPECT_TRUE(seg.removePoint(p1->id()));
    EXPECT_EQ(seg.pointCount(), 1u);
    EXPECT_EQ(seg.pointAt(0)->name(), "A02");
}

TEST(Segment, PointAt_OutOfRange) {
    auto seg = model::Segment("SEG-001");
    EXPECT_EQ(seg.pointAt(0), nullptr);
    EXPECT_EQ(seg.pointAt(100), nullptr);
}

// ============================================================
// Route Tests
// ============================================================

TEST(Route, AddSegments) {
    auto route = model::Route("RT-001");
    auto seg1 = std::make_shared<model::Segment>("SEG-001");
    auto seg2 = std::make_shared<model::Segment>("SEG-002");

    route.addSegment(seg1);
    route.addSegment(seg2);

    EXPECT_EQ(route.segmentCount(), 2u);
    EXPECT_EQ(route.segmentAt(0)->name(), "SEG-001");
    EXPECT_EQ(route.segmentAt(1)->name(), "SEG-002");
}

TEST(Route, RemoveSegment) {
    auto route = model::Route("RT-001");
    auto seg1 = std::make_shared<model::Segment>("SEG-001");
    auto seg2 = std::make_shared<model::Segment>("SEG-002");
    route.addSegment(seg1);
    route.addSegment(seg2);

    EXPECT_TRUE(route.removeSegment(seg1->id()));
    EXPECT_EQ(route.segmentCount(), 1u);
    EXPECT_EQ(route.segmentAt(0)->name(), "SEG-002");
}

TEST(Route, SegmentContainsPipePoints) {
    auto route = model::Route("RT-001");
    auto seg = std::make_shared<model::Segment>("SEG-001");
    auto pt = std::make_shared<model::PipePoint>("A01", model::PipePointType::Run, gp_Pnt(0, 0, 0));
    seg->addPoint(pt);
    route.addSegment(seg);

    EXPECT_EQ(route.segmentAt(0)->pointAt(0)->name(), "A01");
}

// ============================================================
// Inheritance / dynamic_cast Tests
// ============================================================

TEST(Inheritance, DynamicCast_PipePoint) {
    std::shared_ptr<model::DocumentObject> obj =
        std::make_shared<model::PipePoint>("A01", model::PipePointType::Run);

    auto* spatial = dynamic_cast<model::SpatialObject*>(obj.get());
    ASSERT_NE(spatial, nullptr);

    auto* pp = dynamic_cast<model::PipePoint*>(obj.get());
    ASSERT_NE(pp, nullptr);
    EXPECT_EQ(pp->type(), model::PipePointType::Run);

    auto* prop = dynamic_cast<model::PropertyObject*>(obj.get());
    EXPECT_EQ(prop, nullptr); // PipePoint 不是 PropertyObject
}

TEST(Inheritance, DynamicCast_PipeSpec) {
    std::shared_ptr<model::DocumentObject> obj =
        std::make_shared<model::PipeSpec>("spec");

    auto* prop = dynamic_cast<model::PropertyObject*>(obj.get());
    ASSERT_NE(prop, nullptr);

    auto* spatial = dynamic_cast<model::SpatialObject*>(obj.get());
    EXPECT_EQ(spatial, nullptr); // PipeSpec 不是 SpatialObject
}

TEST(Inheritance, DynamicCast_Segment) {
    std::shared_ptr<model::DocumentObject> obj =
        std::make_shared<model::Segment>("SEG");

    auto* container = dynamic_cast<model::ContainerObject*>(obj.get());
    ASSERT_NE(container, nullptr);

    auto* seg = dynamic_cast<model::Segment*>(obj.get());
    ASSERT_NE(seg, nullptr);
}

TEST(Inheritance, DynamicCast_Route) {
    std::shared_ptr<model::DocumentObject> obj =
        std::make_shared<model::Route>("RT");

    auto* container = dynamic_cast<model::ContainerObject*>(obj.get());
    ASSERT_NE(container, nullptr);

    auto* route = dynamic_cast<model::Route*>(obj.get());
    ASSERT_NE(route, nullptr);
}

// ============================================================
// ContainerObject Tests
// ============================================================

TEST(ContainerObject, FindChild) {
    auto container = model::ContainerObject("test-container");
    auto child = std::make_shared<model::PipeSpec>("findme");
    auto childId = child->id();
    container.addChild(child);

    EXPECT_NE(container.findChild(childId), nullptr);
    EXPECT_EQ(container.findChild(childId)->name(), "findme");
}

TEST(ContainerObject, RemoveChild_NotFound) {
    auto container = model::ContainerObject("empty");
    auto fakeId = foundation::UUID::generate();
    EXPECT_FALSE(container.removeChild(fakeId));
}

TEST(ContainerObject, Changed_Signal_OnAdd) {
    auto container = model::ContainerObject("test");
    int count = 0;
    container.changed.connect([&]() { ++count; });
    container.addChild(std::make_shared<model::PipeSpec>("child"));
    EXPECT_EQ(count, 1);
}
