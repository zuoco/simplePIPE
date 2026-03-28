#include <gtest/gtest.h>

#include "model/Accessory.h"
#include "model/FixedPoint.h"
#include "model/Support.h"
#include "model/Flange.h"
#include "model/Gasket.h"
#include "model/SealRing.h"
#include "model/Beam.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"

#include <memory>

// ============================================================
// Accessory Base Tests
// ============================================================

TEST(Accessory, Create_DefaultValues) {
    model::Accessory acc("ACC-001");
    EXPECT_EQ(acc.name(), "ACC-001");
    EXPECT_NEAR(acc.offset().X(), 0.0, 1e-12);
    EXPECT_NEAR(acc.offset().Y(), 0.0, 1e-12);
    EXPECT_NEAR(acc.offset().Z(), 0.0, 1e-12);
    EXPECT_EQ(acc.pipePoint(), nullptr);
}

TEST(Accessory, AttachToPipePoint) {
    auto pt = std::make_shared<model::PipePoint>("A01", model::PipePointType::Run,
                                                  gp_Pnt(100, 200, 300));
    model::Accessory acc("ACC-001");
    acc.attachTo(pt);

    auto retrieved = acc.pipePoint();
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->name(), "A01");
    EXPECT_NEAR(retrieved->position().X(), 100.0, 1e-12);
}

TEST(Accessory, Detach) {
    auto pt = std::make_shared<model::PipePoint>("A01");
    model::Accessory acc("ACC-001");
    acc.attachTo(pt);
    EXPECT_NE(acc.pipePoint(), nullptr);

    acc.detach();
    EXPECT_EQ(acc.pipePoint(), nullptr);
}

TEST(Accessory, Offset_SetGet) {
    model::Accessory acc("ACC-001");
    acc.setOffset(gp_Vec(10.0, 20.0, 30.0));
    EXPECT_NEAR(acc.offset().X(), 10.0, 1e-12);
    EXPECT_NEAR(acc.offset().Y(), 20.0, 1e-12);
    EXPECT_NEAR(acc.offset().Z(), 30.0, 1e-12);
}

TEST(Accessory, Signal_OnAttach) {
    auto pt = std::make_shared<model::PipePoint>("A01");
    model::Accessory acc("ACC-001");
    int count = 0;
    acc.changed.connect([&]() { ++count; });

    acc.attachTo(pt);
    EXPECT_EQ(count, 1);
    acc.detach();
    EXPECT_EQ(count, 2);
}

TEST(Accessory, Signal_OnOffsetChange) {
    model::Accessory acc("ACC-001");
    int count = 0;
    acc.changed.connect([&]() { ++count; });

    acc.setOffset(gp_Vec(1, 2, 3));
    EXPECT_EQ(count, 1);
}

TEST(Accessory, WeakRef_Expires) {
    model::Accessory acc("ACC-001");
    {
        auto pt = std::make_shared<model::PipePoint>("A01");
        acc.attachTo(pt);
        EXPECT_NE(acc.pipePoint(), nullptr);
    }
    // PipePoint has been destroyed, weak_ptr should expire
    EXPECT_EQ(acc.pipePoint(), nullptr);
}

// ============================================================
// FixedPoint Tests
// ============================================================

TEST(FixedPoint, IsFixed) {
    model::FixedPoint fp("FP-001", gp_Pnt(0, 0, 0));
    EXPECT_TRUE(fp.isFixed());
}

TEST(FixedPoint, InheritsAccessory) {
    auto pt = std::make_shared<model::PipePoint>("A01");
    auto fp = model::FixedPoint("FP-001");
    fp.attachTo(pt);
    EXPECT_NE(fp.pipePoint(), nullptr);
}

// ============================================================
// Support Tests
// ============================================================

TEST(Support, TypeAndDirection_Defaults) {
    model::Support sup("SUP-001");
    EXPECT_EQ(sup.supportType(), model::SupportType::Rigid);
    // Default load direction is Z-up (0, 0, 1)
    EXPECT_NEAR(sup.loadDirection().Z(), 1.0, 1e-12);
}

TEST(Support, SetType) {
    model::Support sup("SUP-001");
    sup.setSupportType(model::SupportType::Spring);
    EXPECT_EQ(sup.supportType(), model::SupportType::Spring);
}

TEST(Support, SetLoadDirection) {
    model::Support sup("SUP-001");
    sup.setLoadDirection(gp_Dir(0, 1, 0));
    EXPECT_NEAR(sup.loadDirection().Y(), 1.0, 1e-12);
    EXPECT_NEAR(sup.loadDirection().Z(), 0.0, 1e-12);
}

TEST(Support, Signal_OnTypeChange) {
    model::Support sup("SUP-001");
    int count = 0;
    sup.changed.connect([&]() { ++count; });

    sup.setSupportType(model::SupportType::Rod);
    EXPECT_EQ(count, 1);
    sup.setSupportType(model::SupportType::Rod); // same value, no signal
    EXPECT_EQ(count, 1);
}

TEST(Support, Signal_OnDirectionChange) {
    model::Support sup("SUP-001");
    int count = 0;
    sup.changed.connect([&]() { ++count; });

    sup.setLoadDirection(gp_Dir(1, 0, 0));
    EXPECT_EQ(count, 1);
    sup.setLoadDirection(gp_Dir(1, 0, 0)); // same value, no signal
    EXPECT_EQ(count, 1);
}

// ============================================================
// Flange Tests
// ============================================================

TEST(Flange, RatingAndFaceType) {
    model::Flange fl("FL-001");
    fl.setRating("300");
    fl.setFaceType("RF");
    EXPECT_EQ(fl.rating(), "300");
    EXPECT_EQ(fl.faceType(), "RF");
}

TEST(Flange, BoltHoles) {
    model::Flange fl("FL-001");
    EXPECT_EQ(fl.boltHoleCount(), 0);
    fl.setBoltHoleCount(12);
    EXPECT_EQ(fl.boltHoleCount(), 12);
}

TEST(Flange, Signal_OnChange) {
    model::Flange fl("FL-001");
    int count = 0;
    fl.changed.connect([&]() { ++count; });

    fl.setRating("150");
    EXPECT_EQ(count, 1);
    fl.setFaceType("RTJ");
    EXPECT_EQ(count, 2);
    fl.setBoltHoleCount(8);
    EXPECT_EQ(count, 3);

    // Same values should not trigger
    fl.setRating("150");
    fl.setFaceType("RTJ");
    fl.setBoltHoleCount(8);
    EXPECT_EQ(count, 3);
}

// ============================================================
// Gasket Tests
// ============================================================

TEST(Gasket, MaterialAndThickness) {
    model::Gasket g("GK-001");
    g.setGasketMaterial("Graphite");
    g.setThickness(3.0);
    EXPECT_EQ(g.gasketMaterial(), "Graphite");
    EXPECT_NEAR(g.thickness(), 3.0, 1e-12);
}

TEST(Gasket, Signal_OnChange) {
    model::Gasket g("GK-001");
    int count = 0;
    g.changed.connect([&]() { ++count; });

    g.setGasketMaterial("PTFE");
    EXPECT_EQ(count, 1);
    g.setThickness(1.5);
    EXPECT_EQ(count, 2);

    g.setGasketMaterial("PTFE"); // no change
    g.setThickness(1.5);        // no change
    EXPECT_EQ(count, 2);
}

// ============================================================
// SealRing Tests
// ============================================================

TEST(SealRing, MaterialAndCrossSection) {
    model::SealRing sr("SR-001");
    sr.setSealMaterial("Viton");
    sr.setCrossSectionDiameter(5.7);
    EXPECT_EQ(sr.sealMaterial(), "Viton");
    EXPECT_NEAR(sr.crossSectionDiameter(), 5.7, 1e-12);
}

TEST(SealRing, Signal_OnChange) {
    model::SealRing sr("SR-001");
    int count = 0;
    sr.changed.connect([&]() { ++count; });

    sr.setSealMaterial("NBR");
    EXPECT_EQ(count, 1);
    sr.setCrossSectionDiameter(3.5);
    EXPECT_EQ(count, 2);

    sr.setSealMaterial("NBR");        // no change
    sr.setCrossSectionDiameter(3.5);  // no change
    EXPECT_EQ(count, 2);
}

// ============================================================
// Beam Tests
// ============================================================

TEST(Beam, DualEndReferences) {
    auto p1 = std::make_shared<model::PipePoint>("A01", model::PipePointType::Run,
                                                   gp_Pnt(0, 0, 0));
    auto p2 = std::make_shared<model::PipePoint>("A02", model::PipePointType::Run,
                                                   gp_Pnt(3000, 0, 0));
    model::Beam beam("BM-001");
    beam.setStartPoint(p1);
    beam.setEndPoint(p2);

    ASSERT_NE(beam.startPoint(), nullptr);
    ASSERT_NE(beam.endPoint(), nullptr);
    EXPECT_EQ(beam.startPoint()->name(), "A01");
    EXPECT_EQ(beam.endPoint()->name(), "A02");
}

TEST(Beam, Length_TwoEndDistance) {
    auto p1 = std::make_shared<model::PipePoint>("A01", model::PipePointType::Run,
                                                   gp_Pnt(0, 0, 0));
    auto p2 = std::make_shared<model::PipePoint>("A02", model::PipePointType::Run,
                                                   gp_Pnt(3000, 4000, 0));
    model::Beam beam("BM-001");
    beam.setStartPoint(p1);
    beam.setEndPoint(p2);

    EXPECT_NEAR(beam.length(), 5000.0, 1e-6);
}

TEST(Beam, Length_ZeroWhenNoPoints) {
    model::Beam beam("BM-001");
    EXPECT_NEAR(beam.length(), 0.0, 1e-12);
}

TEST(Beam, Length_ZeroWhenOnePoint) {
    auto p1 = std::make_shared<model::PipePoint>("A01", model::PipePointType::Run,
                                                   gp_Pnt(0, 0, 0));
    model::Beam beam("BM-001");
    beam.setStartPoint(p1);
    EXPECT_NEAR(beam.length(), 0.0, 1e-12);
}

TEST(Beam, SectionType_Default) {
    model::Beam beam("BM-001");
    EXPECT_EQ(beam.sectionType(), model::BeamSectionType::Rectangular);
}

TEST(Beam, SectionType_Set) {
    model::Beam beam("BM-001");
    beam.setSectionType(model::BeamSectionType::HSection);
    EXPECT_EQ(beam.sectionType(), model::BeamSectionType::HSection);
}

TEST(Beam, Dimensions) {
    model::Beam beam("BM-001");
    beam.setWidth(150.0);
    beam.setHeight(300.0);
    EXPECT_NEAR(beam.width(), 150.0, 1e-12);
    EXPECT_NEAR(beam.height(), 300.0, 1e-12);
}

TEST(Beam, Signal_OnChange) {
    model::Beam beam("BM-001");
    int count = 0;
    beam.changed.connect([&]() { ++count; });

    auto p1 = std::make_shared<model::PipePoint>("A01");
    beam.setStartPoint(p1);
    EXPECT_EQ(count, 1);

    beam.setSectionType(model::BeamSectionType::HSection);
    EXPECT_EQ(count, 2);

    beam.setWidth(250.0);
    EXPECT_EQ(count, 3);

    beam.setHeight(400.0);
    EXPECT_EQ(count, 4);

    // Same values should not trigger
    beam.setSectionType(model::BeamSectionType::HSection);
    beam.setWidth(250.0);
    beam.setHeight(400.0);
    EXPECT_EQ(count, 4);
}

TEST(Beam, WeakRef_Expires) {
    model::Beam beam("BM-001");
    {
        auto p1 = std::make_shared<model::PipePoint>("A01", model::PipePointType::Run,
                                                       gp_Pnt(0, 0, 0));
        auto p2 = std::make_shared<model::PipePoint>("A02", model::PipePointType::Run,
                                                       gp_Pnt(1000, 0, 0));
        beam.setStartPoint(p1);
        beam.setEndPoint(p2);
        EXPECT_NEAR(beam.length(), 1000.0, 1e-6);
    }
    // Points destroyed
    EXPECT_EQ(beam.startPoint(), nullptr);
    EXPECT_EQ(beam.endPoint(), nullptr);
    EXPECT_NEAR(beam.length(), 0.0, 1e-12);
}

// ============================================================
// PipePoint Accessory List Tests
// ============================================================

TEST(PipePoint, AccessoryList_Empty) {
    model::PipePoint pt("A01");
    EXPECT_EQ(pt.accessoryCount(), 0u);
    EXPECT_TRUE(pt.accessories().empty());
}

TEST(PipePoint, AccessoryList_AddAndQuery) {
    auto pt = std::make_shared<model::PipePoint>("A01", model::PipePointType::Run,
                                                   gp_Pnt(100, 0, 0));
    auto fl = std::make_shared<model::Flange>("FL-001");
    fl->setRating("300");
    fl->attachTo(pt);
    pt->addAccessory(fl);

    EXPECT_EQ(pt->accessoryCount(), 1u);
    EXPECT_EQ(pt->accessories()[0]->name(), "FL-001");

    // Back-reference from accessory to pipe point
    ASSERT_NE(fl->pipePoint(), nullptr);
    EXPECT_EQ(fl->pipePoint()->name(), "A01");
}

TEST(PipePoint, AccessoryList_Multiple) {
    auto pt = std::make_shared<model::PipePoint>("A01");
    auto fl = std::make_shared<model::Flange>("FL-001");
    auto gk = std::make_shared<model::Gasket>("GK-001");
    auto sr = std::make_shared<model::SealRing>("SR-001");

    fl->attachTo(pt);
    gk->attachTo(pt);
    sr->attachTo(pt);

    pt->addAccessory(fl);
    pt->addAccessory(gk);
    pt->addAccessory(sr);

    EXPECT_EQ(pt->accessoryCount(), 3u);
}

TEST(PipePoint, AccessoryList_Remove) {
    auto pt = std::make_shared<model::PipePoint>("A01");
    auto fl = std::make_shared<model::Flange>("FL-001");
    auto gk = std::make_shared<model::Gasket>("GK-001");

    pt->addAccessory(fl);
    pt->addAccessory(gk);
    EXPECT_EQ(pt->accessoryCount(), 2u);

    bool removed = pt->removeAccessory(fl->id());
    EXPECT_TRUE(removed);
    EXPECT_EQ(pt->accessoryCount(), 1u);
    EXPECT_EQ(pt->accessories()[0]->name(), "GK-001");
}

TEST(PipePoint, AccessoryList_RemoveNonExistent) {
    auto pt = std::make_shared<model::PipePoint>("A01");
    foundation::UUID fakeId = foundation::UUID::generate();
    EXPECT_FALSE(pt->removeAccessory(fakeId));
}

TEST(PipePoint, AccessoryList_Signal) {
    auto pt = std::make_shared<model::PipePoint>("A01");
    int count = 0;
    pt->changed.connect([&]() { ++count; });

    auto fl = std::make_shared<model::Flange>("FL-001");
    pt->addAccessory(fl);
    EXPECT_EQ(count, 1);

    pt->removeAccessory(fl->id());
    EXPECT_EQ(count, 2);
}
