// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "app/Document.h"
#include "app/ProjectSerializer.h"
#include "model/Beam.h"
#include "model/Flange.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/ProjectConfig.h"
#include "model/Route.h"
#include "model/Segment.h"

#include <nlohmann/json.hpp>

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

namespace {

std::string tempPath(const std::string& suffix) {
    return std::string("/tmp/pipecad_serializer_") + suffix;
}

} // namespace

TEST(ProjectSerializer, SaveLoadSaveRoundTripStable) {
    app::Document doc;
    doc.setName("DemoProject");

    auto cfg = std::make_shared<model::ProjectConfig>("ProjectConfig");
    cfg->setProjectName("DemoProject");
    cfg->setAuthor("Alice");
    cfg->setStandard("ASME");
    cfg->setUnitSystem(foundation::UnitSystem::SI);

    auto spec = std::make_shared<model::PipeSpec>("Spec-A");
    spec->setOd(168.3);
    spec->setWallThickness(7.11);
    spec->setMaterial("CS");
    spec->setField("schedule", std::string("40"));

    auto p1 = std::make_shared<model::PipePoint>("P1", model::PipePointType::Run, gp_Pnt(0, 0, 0));
    auto p2 = std::make_shared<model::PipePoint>("P2", model::PipePointType::Bend, gp_Pnt(1000, 0, 0));
    p1->setPipeSpec(spec);
    p2->setPipeSpec(spec);
    p2->setParam("bendMultiplier", 2.0);

    auto seg = std::make_shared<model::Segment>("S1");
    seg->addPoint(p1);
    seg->addPoint(p2);

    auto route = std::make_shared<model::Route>("R1");
    route->addSegment(seg);

    auto flange = std::make_shared<model::Flange>("F1", gp_Pnt(0, 0, 0));
    flange->setRating("150");
    flange->setFaceType("RF");
    flange->setBoltHoleCount(8);
    flange->attachTo(p1);
    flange->setOffset(gp_Vec(1, 2, 3));
    p1->addAccessory(flange);

    auto beam = std::make_shared<model::Beam>("B1");
    beam->setSectionType(model::BeamSectionType::HSection);
    beam->setWidth(200.0);
    beam->setHeight(300.0);
    beam->setStartPoint(p1);
    beam->setEndPoint(p2);

    doc.addObject(cfg);
    doc.addObject(spec);
    doc.addObject(route);
    doc.addObject(seg);
    doc.addObject(p1);
    doc.addObject(p2);
    doc.addObject(flange);
    doc.addObject(beam);

    const std::string path1 = tempPath("a.json");
    const std::string path2 = tempPath("b.json");

    ASSERT_TRUE(app::ProjectSerializer::save(doc, path1));
    auto loaded = app::ProjectSerializer::load(path1);
    ASSERT_NE(loaded, nullptr);
    ASSERT_TRUE(app::ProjectSerializer::save(*loaded, path2));

    nlohmann::json j1;
    nlohmann::json j2;
    {
        std::ifstream f1(path1);
        std::ifstream f2(path2);
        ASSERT_TRUE(f1.is_open());
        ASSERT_TRUE(f2.is_open());
        f1 >> j1;
        f2 >> j2;
    }

    EXPECT_EQ(j1, j2);

    std::remove(path1.c_str());
    std::remove(path2.c_str());
}

TEST(ProjectSerializer, RestoresObjectReferences) {
    app::Document doc;

    auto spec = std::make_shared<model::PipeSpec>("Spec-B");
    spec->setOd(114.3);
    spec->setWallThickness(6.0);

    auto p1 = std::make_shared<model::PipePoint>("Start", model::PipePointType::Run, gp_Pnt(0, 0, 0));
    auto p2 = std::make_shared<model::PipePoint>("End", model::PipePointType::Run, gp_Pnt(500, 0, 0));
    p1->setPipeSpec(spec);
    p2->setPipeSpec(spec);

    auto seg = std::make_shared<model::Segment>("Seg");
    seg->addPoint(p1);
    seg->addPoint(p2);

    auto route = std::make_shared<model::Route>("Route");
    route->addSegment(seg);

    auto flange = std::make_shared<model::Flange>("Flange-A");
    flange->attachTo(p1);
    p1->addAccessory(flange);

    auto beam = std::make_shared<model::Beam>("Beam-A");
    beam->setStartPoint(p1);
    beam->setEndPoint(p2);

    doc.addObject(spec);
    doc.addObject(route);
    doc.addObject(seg);
    doc.addObject(p1);
    doc.addObject(p2);
    doc.addObject(flange);
    doc.addObject(beam);

    const std::string path = tempPath("refs.json");
    ASSERT_TRUE(app::ProjectSerializer::save(doc, path));

    auto loaded = app::ProjectSerializer::load(path);
    ASSERT_NE(loaded, nullptr);

    auto loadedPoints = loaded->findByType<model::PipePoint>();
    ASSERT_EQ(loadedPoints.size(), 2u);

    model::PipePoint* loadedStart = nullptr;
    model::PipePoint* loadedEnd = nullptr;
    for (auto* pp : loadedPoints) {
        if (pp->name() == "Start") loadedStart = pp;
        if (pp->name() == "End") loadedEnd = pp;
    }

    ASSERT_NE(loadedStart, nullptr);
    ASSERT_NE(loadedEnd, nullptr);
    ASSERT_NE(loadedStart->pipeSpec(), nullptr);
    ASSERT_NE(loadedEnd->pipeSpec(), nullptr);
    EXPECT_EQ(loadedStart->pipeSpec()->id(), loadedEnd->pipeSpec()->id());
    EXPECT_EQ(loadedStart->accessoryCount(), 1u);

    auto loadedBeams = loaded->findByType<model::Beam>();
    ASSERT_EQ(loadedBeams.size(), 1u);
    ASSERT_NE(loadedBeams[0]->startPoint(), nullptr);
    ASSERT_NE(loadedBeams[0]->endPoint(), nullptr);
    EXPECT_EQ(loadedBeams[0]->startPoint()->name(), "Start");
    EXPECT_EQ(loadedBeams[0]->endPoint()->name(), "End");

    std::remove(path.c_str());
}
