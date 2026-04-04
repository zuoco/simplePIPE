// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

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
#include "model/ThermalLoad.h"
#include "model/PressureLoad.h"
#include "model/WindLoad.h"
#include "model/SeismicLoad.h"
#include "model/DisplacementLoad.h"
#include "model/UserDefinedLoad.h"

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

// ============================================================
// T1: setProperty / getProperty 虚方法测试
// ============================================================

// DocumentObject 基类：处理 "name"
TEST(PropertyDispatch, DocumentObject_SetName) {
    model::PipeSpec obj("original");
    EXPECT_TRUE(obj.setProperty("name", std::string("updated")));
    EXPECT_EQ(obj.name(), "updated");
}

TEST(PropertyDispatch, DocumentObject_GetName) {
    model::PipeSpec obj("hello");
    foundation::Variant v = obj.getProperty("name");
    EXPECT_EQ(std::get<std::string>(v), "hello");
}

TEST(PropertyDispatch, DocumentObject_UnknownKey_SetReturnsFalse) {
    model::PipeSpec obj("spec");
    // PipeSpec (PropertyObject) 接受任意 key 存入 fields_，返回 true
    // 注意：对于需要测试 DocumentObject 基类返回 false 的情况，
    // 使用不继承 PropertyObject 的对象（如载荷类）
    model::ThermalLoad load;
    EXPECT_FALSE(load.setProperty("unknownXyz123", 1.0));
}

TEST(PropertyDispatch, DocumentObject_UnknownKey_GetThrows) {
    model::ThermalLoad load;
    EXPECT_THROW(load.getProperty("nonExistentKey"), std::out_of_range);
}

// SpatialObject：处理 "x"/"y"/"z"
TEST(PropertyDispatch, SpatialObject_XYZ_RoundTrip) {
    model::PipePoint pt("P01", model::PipePointType::Run, gp_Pnt(0, 0, 0));
    EXPECT_TRUE(pt.setProperty("x", 100.0));
    EXPECT_TRUE(pt.setProperty("y", 200.0));
    EXPECT_TRUE(pt.setProperty("z", 300.0));
    EXPECT_NEAR(std::get<double>(pt.getProperty("x")), 100.0, 1e-12);
    EXPECT_NEAR(std::get<double>(pt.getProperty("y")), 200.0, 1e-12);
    EXPECT_NEAR(std::get<double>(pt.getProperty("z")), 300.0, 1e-12);
    EXPECT_NEAR(pt.position().X(), 100.0, 1e-12);
    EXPECT_NEAR(pt.position().Y(), 200.0, 1e-12);
    EXPECT_NEAR(pt.position().Z(), 300.0, 1e-12);
}

// PropertyObject：任意 key 存入 fields_
TEST(PropertyDispatch, PropertyObject_ArbitraryField) {
    model::PipeSpec spec("6in-Sch40");
    EXPECT_TRUE(spec.setProperty("OD", 168.3));
    EXPECT_TRUE(spec.setProperty("wallThickness", 7.11));
    EXPECT_TRUE(spec.setProperty("material", std::string("Carbon Steel")));
    EXPECT_NEAR(std::get<double>(spec.getProperty("OD")), 168.3, 1e-12);
    EXPECT_NEAR(std::get<double>(spec.getProperty("wallThickness")), 7.11, 1e-12);
    EXPECT_EQ(std::get<std::string>(spec.getProperty("material")), "Carbon Steel");
}

TEST(PropertyDispatch, PropertyObject_NameFallthrough) {
    model::ProjectConfig cfg("proj");
    EXPECT_TRUE(cfg.setProperty("name", std::string("NewName")));
    EXPECT_EQ(cfg.name(), "NewName");
    EXPECT_EQ(std::get<std::string>(cfg.getProperty("name")), "NewName");
}

TEST(PropertyDispatch, PropertyObject_GetMissingFieldThrows) {
    model::PipeSpec spec("empty");
    EXPECT_THROW(spec.getProperty("nonExistentField"), std::out_of_range);
}

// PipePoint：type / x/y/z / typeParams
TEST(PropertyDispatch, PipePoint_Type_RoundTrip) {
    model::PipePoint pt("A06", model::PipePointType::Run);
    EXPECT_TRUE(pt.setProperty("type", static_cast<int>(model::PipePointType::Bend)));
    EXPECT_EQ(pt.type(), model::PipePointType::Bend);
    foundation::Variant v = pt.getProperty("type");
    EXPECT_EQ(std::get<int>(v), static_cast<int>(model::PipePointType::Bend));
}

TEST(PropertyDispatch, PipePoint_TypeParams_Fallback) {
    model::PipePoint pt("A06", model::PipePointType::Bend);
    EXPECT_TRUE(pt.setProperty("bendMultiplier", 1.5));
    EXPECT_NEAR(std::get<double>(pt.getProperty("bendMultiplier")), 1.5, 1e-12);
    EXPECT_NEAR(foundation::variantToDouble(pt.param("bendMultiplier")), 1.5, 1e-12);
}

TEST(PropertyDispatch, PipePoint_NamePropagates) {
    model::PipePoint pt("init");
    EXPECT_TRUE(pt.setProperty("name", std::string("A99")));
    EXPECT_EQ(pt.name(), "A99");
}

// ThermalLoad
TEST(PropertyDispatch, ThermalLoad_RoundTrip) {
    model::ThermalLoad load;
    EXPECT_TRUE(load.setProperty("installTemp", 10.0));
    EXPECT_TRUE(load.setProperty("operatingTemp", 120.0));
    EXPECT_NEAR(std::get<double>(load.getProperty("installTemp")), 10.0, 1e-12);
    EXPECT_NEAR(std::get<double>(load.getProperty("operatingTemp")), 120.0, 1e-12);
    EXPECT_NEAR(load.installTemp(), 10.0, 1e-12);
    EXPECT_NEAR(load.operatingTemp(), 120.0, 1e-12);
}

// PressureLoad（含 bool 类型）
TEST(PropertyDispatch, PressureLoad_RoundTrip) {
    model::PressureLoad load;
    EXPECT_TRUE(load.setProperty("pressure", 10.5));
    EXPECT_TRUE(load.setProperty("isExternal", true));
    EXPECT_NEAR(std::get<double>(load.getProperty("pressure")), 10.5, 1e-12);
    EXPECT_TRUE(std::get<bool>(load.getProperty("isExternal")));
    EXPECT_NEAR(load.pressure(), 10.5, 1e-12);
    EXPECT_TRUE(load.isExternal());
}

// WindLoad（含 Vec3 类型）
TEST(PropertyDispatch, WindLoad_RoundTrip) {
    model::WindLoad load;
    EXPECT_TRUE(load.setProperty("speed", 30.0));
    foundation::math::Vec3 dir{0.0, 1.0, 0.0};
    EXPECT_TRUE(load.setProperty("direction", dir));
    EXPECT_NEAR(std::get<double>(load.getProperty("speed")), 30.0, 1e-12);
    const auto& gotDir = std::get<foundation::math::Vec3>(load.getProperty("direction"));
    EXPECT_NEAR(gotDir.y, 1.0, 1e-12);
    EXPECT_NEAR(load.speed(), 30.0, 1e-12);
    EXPECT_NEAR(load.direction().y, 1.0, 1e-12);
}

// SeismicLoad（含 Vec3 类型）
TEST(PropertyDispatch, SeismicLoad_RoundTrip) {
    model::SeismicLoad load;
    EXPECT_TRUE(load.setProperty("acceleration", 0.3));
    foundation::math::Vec3 dir{1.0, 0.0, 0.0};
    EXPECT_TRUE(load.setProperty("direction", dir));
    EXPECT_NEAR(std::get<double>(load.getProperty("acceleration")), 0.3, 1e-12);
    const auto& gotDir = std::get<foundation::math::Vec3>(load.getProperty("direction"));
    EXPECT_NEAR(gotDir.x, 1.0, 1e-12);
    EXPECT_NEAR(load.acceleration(), 0.3, 1e-12);
}

// DisplacementLoad（Vec3 类型）
TEST(PropertyDispatch, DisplacementLoad_RoundTrip) {
    model::DisplacementLoad load;
    foundation::math::Vec3 t{10.0, 20.0, 30.0};
    foundation::math::Vec3 r{1.0, 2.0, 3.0};
    EXPECT_TRUE(load.setProperty("translation", t));
    EXPECT_TRUE(load.setProperty("rotation", r));
    const auto& gotT = std::get<foundation::math::Vec3>(load.getProperty("translation"));
    const auto& gotR = std::get<foundation::math::Vec3>(load.getProperty("rotation"));
    EXPECT_NEAR(gotT.x, 10.0, 1e-12);
    EXPECT_NEAR(gotR.z, 3.0, 1e-12);
    EXPECT_NEAR(load.translation().y, 20.0, 1e-12);
}

// UserDefinedLoad（Vec3 类型）
TEST(PropertyDispatch, UserDefinedLoad_RoundTrip) {
    model::UserDefinedLoad load;
    foundation::math::Vec3 f{100.0, 0.0, 0.0};
    foundation::math::Vec3 m{0.0, 500.0, 0.0};
    EXPECT_TRUE(load.setProperty("force", f));
    EXPECT_TRUE(load.setProperty("moment", m));
    const auto& gotF = std::get<foundation::math::Vec3>(load.getProperty("force"));
    const auto& gotM = std::get<foundation::math::Vec3>(load.getProperty("moment"));
    EXPECT_NEAR(gotF.x, 100.0, 1e-12);
    EXPECT_NEAR(gotM.y, 500.0, 1e-12);
}

// 多态分派：通过 DocumentObject* 调用
TEST(PropertyDispatch, Polymorphic_Via_BasePointer) {
    std::unique_ptr<model::DocumentObject> obj =
        std::make_unique<model::PipePoint>("P01", model::PipePointType::Run, gp_Pnt(0, 0, 0));
    EXPECT_TRUE(obj->setProperty("x", 999.0));
    EXPECT_NEAR(std::get<double>(obj->getProperty("x")), 999.0, 1e-12);

    auto* pp = dynamic_cast<model::PipePoint*>(obj.get());
    ASSERT_NE(pp, nullptr);
    EXPECT_NEAR(pp->position().X(), 999.0, 1e-12);
}

