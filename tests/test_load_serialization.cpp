// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "app/DependencyGraph.h"
#include "app/Document.h"
#include "app/ProjectSerializer.h"
#include "model/DeadWeightLoad.h"
#include "model/DisplacementLoad.h"
#include "model/LoadCase.h"
#include "model/LoadCombination.h"
#include "model/PressureLoad.h"
#include "model/ProjectConfig.h"
#include "model/SeismicLoad.h"
#include "model/ThermalLoad.h"
#include "model/UserDefinedLoad.h"
#include "model/WindLoad.h"

#include <nlohmann/json.hpp>

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace {

std::string tempPath(const std::string& suffix) {
    return std::string("/tmp/pipecad_load_serialization_") + suffix;
}

bool containsId(const std::vector<foundation::UUID>& ids, const foundation::UUID& id) {
    for (const auto& item : ids) {
        if (item == id) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST(LoadSerialization, SaveLoadSaveRoundTripForAllLoadTypes) {
    app::Document doc;
    doc.setName("LoadSerializationProject");

    auto projectConfig = std::make_shared<model::ProjectConfig>("ProjectConfig");
    projectConfig->setProjectName("LoadSerializationProject");
    projectConfig->setAuthor("QA");
    projectConfig->setStandard("ASME B31.3");
    projectConfig->setUnitSystem(foundation::UnitSystem::SI);

    auto deadWeight = std::make_shared<model::DeadWeightLoad>("W");
    deadWeight->addAffectedObject(foundation::UUID::generate());

    auto thermal = std::make_shared<model::ThermalLoad>("T1");
    thermal->setInstallTemp(20.0);
    thermal->setOperatingTemp(180.0);

    auto pressure = std::make_shared<model::PressureLoad>("P1");
    pressure->setPressure(2.5);
    pressure->setIsExternal(true);

    auto wind = std::make_shared<model::WindLoad>("WN");
    wind->setSpeed(30.0);
    wind->setDirection({1.0, 0.0, 0.0});

    auto seismic = std::make_shared<model::SeismicLoad>("EQ");
    seismic->setAcceleration(0.3);
    seismic->setDirection({0.0, 0.0, 1.0});

    auto displacement = std::make_shared<model::DisplacementLoad>("DISP");
    displacement->setTranslation({1.0, 2.0, 3.0});
    displacement->setRotation({10.0, 20.0, 30.0});

    auto userDefined = std::make_shared<model::UserDefinedLoad>("UDL");
    userDefined->setForce({100.0, 200.0, 300.0});
    userDefined->setMoment({1000.0, 2000.0, 3000.0});

    auto loadCaseW = std::make_shared<model::LoadCase>("W");
    loadCaseW->addEntry({deadWeight->id(), 1.0});
    loadCaseW->addEntry({pressure->id(), 1.0});

    auto loadCaseT = std::make_shared<model::LoadCase>("T1");
    loadCaseT->addEntry({thermal->id(), 1.0});

    auto combo = std::make_shared<model::LoadCombination>(
        "OPE", model::StressCategory::Operating, model::CombineMethod::Algebraic);
    combo->addCaseEntry({loadCaseW->id(), 1.0});
    combo->addCaseEntry({loadCaseT->id(), 1.0});

    doc.addObject(projectConfig);
    doc.addObject(deadWeight);
    doc.addObject(thermal);
    doc.addObject(pressure);
    doc.addObject(wind);
    doc.addObject(seismic);
    doc.addObject(displacement);
    doc.addObject(userDefined);
    doc.addObject(loadCaseW);
    doc.addObject(loadCaseT);
    doc.addObject(combo);

    const std::string path1 = tempPath("roundtrip_a.json");
    const std::string path2 = tempPath("roundtrip_b.json");

    ASSERT_TRUE(app::ProjectSerializer::save(doc, path1));

    auto loaded = app::ProjectSerializer::load(path1);
    ASSERT_NE(loaded, nullptr);

    auto loads = loaded->findByType<model::Load>();
    auto loadCases = loaded->findByType<model::LoadCase>();
    auto combinations = loaded->findByType<model::LoadCombination>();

    EXPECT_EQ(loads.size(), 7u);
    EXPECT_EQ(loadCases.size(), 2u);
    EXPECT_EQ(combinations.size(), 1u);

    int thermalCount = 0;
    int pressureCount = 0;
    int windCount = 0;
    int seismicCount = 0;
    int displacementCount = 0;
    int userDefinedCount = 0;
    int deadWeightCount = 0;

    for (const auto* load : loads) {
        const std::string type = load->loadType();
        if (type == "Thermal") {
            ++thermalCount;
            const auto* t = dynamic_cast<const model::ThermalLoad*>(load);
            ASSERT_NE(t, nullptr);
            EXPECT_DOUBLE_EQ(t->installTemp(), 20.0);
            EXPECT_DOUBLE_EQ(t->operatingTemp(), 180.0);
        } else if (type == "Pressure") {
            ++pressureCount;
            const auto* p = dynamic_cast<const model::PressureLoad*>(load);
            ASSERT_NE(p, nullptr);
            EXPECT_DOUBLE_EQ(p->pressure(), 2.5);
            EXPECT_TRUE(p->isExternal());
        } else if (type == "Wind") {
            ++windCount;
            const auto* w = dynamic_cast<const model::WindLoad*>(load);
            ASSERT_NE(w, nullptr);
            EXPECT_DOUBLE_EQ(w->speed(), 30.0);
            EXPECT_DOUBLE_EQ(w->direction().x, 1.0);
            EXPECT_DOUBLE_EQ(w->direction().y, 0.0);
            EXPECT_DOUBLE_EQ(w->direction().z, 0.0);
        } else if (type == "Seismic") {
            ++seismicCount;
            const auto* s = dynamic_cast<const model::SeismicLoad*>(load);
            ASSERT_NE(s, nullptr);
            EXPECT_DOUBLE_EQ(s->acceleration(), 0.3);
            EXPECT_DOUBLE_EQ(s->direction().x, 0.0);
            EXPECT_DOUBLE_EQ(s->direction().y, 0.0);
            EXPECT_DOUBLE_EQ(s->direction().z, 1.0);
        } else if (type == "Displacement") {
            ++displacementCount;
            const auto* d = dynamic_cast<const model::DisplacementLoad*>(load);
            ASSERT_NE(d, nullptr);
            EXPECT_DOUBLE_EQ(d->translation().x, 1.0);
            EXPECT_DOUBLE_EQ(d->translation().y, 2.0);
            EXPECT_DOUBLE_EQ(d->translation().z, 3.0);
            EXPECT_DOUBLE_EQ(d->rotation().x, 10.0);
            EXPECT_DOUBLE_EQ(d->rotation().y, 20.0);
            EXPECT_DOUBLE_EQ(d->rotation().z, 30.0);
        } else if (type == "UserDefined") {
            ++userDefinedCount;
            const auto* u = dynamic_cast<const model::UserDefinedLoad*>(load);
            ASSERT_NE(u, nullptr);
            EXPECT_DOUBLE_EQ(u->force().x, 100.0);
            EXPECT_DOUBLE_EQ(u->force().y, 200.0);
            EXPECT_DOUBLE_EQ(u->force().z, 300.0);
            EXPECT_DOUBLE_EQ(u->moment().x, 1000.0);
            EXPECT_DOUBLE_EQ(u->moment().y, 2000.0);
            EXPECT_DOUBLE_EQ(u->moment().z, 3000.0);
        } else if (type == "DeadWeight") {
            ++deadWeightCount;
        }
    }

    EXPECT_EQ(thermalCount, 1);
    EXPECT_EQ(pressureCount, 1);
    EXPECT_EQ(windCount, 1);
    EXPECT_EQ(seismicCount, 1);
    EXPECT_EQ(displacementCount, 1);
    EXPECT_EQ(userDefinedCount, 1);
    EXPECT_EQ(deadWeightCount, 1);

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

TEST(LoadSerialization, LoadBuildsDependencyChainInGraph) {
    app::Document doc;

    auto thermal = std::make_shared<model::ThermalLoad>("T1");
    auto loadCase = std::make_shared<model::LoadCase>("LC");
    loadCase->addEntry({thermal->id(), 1.0});

    auto combination = std::make_shared<model::LoadCombination>(
        "COMBO", model::StressCategory::Operating, model::CombineMethod::Algebraic);
    combination->addCaseEntry({loadCase->id(), 1.0});

    doc.addObject(thermal);
    doc.addObject(loadCase);
    doc.addObject(combination);

    const std::string path = tempPath("dependency.json");
    ASSERT_TRUE(app::ProjectSerializer::save(doc, path));

    app::DependencyGraph graph;
    auto loaded = app::ProjectSerializer::load(path, &graph);
    ASSERT_NE(loaded, nullptr);

    auto loadedLoads = loaded->findByType<model::Load>();
    auto loadedCases = loaded->findByType<model::LoadCase>();
    auto loadedCombos = loaded->findByType<model::LoadCombination>();

    ASSERT_EQ(loadedLoads.size(), 1u);
    ASSERT_EQ(loadedCases.size(), 1u);
    ASSERT_EQ(loadedCombos.size(), 1u);

    const foundation::UUID loadId = loadedLoads.front()->id();
    const foundation::UUID caseId = loadedCases.front()->id();
    const foundation::UUID comboId = loadedCombos.front()->id();

    graph.markDirty(loadId);

    EXPECT_TRUE(graph.isDirty(loadId));
    EXPECT_TRUE(graph.isDirty(caseId));
    EXPECT_TRUE(graph.isDirty(comboId));

    std::remove(path.c_str());
}

TEST(LoadSerialization, LoadDependencyChainPropagation) {
    app::Document doc;
    app::DependencyGraph graph;

    auto thermal = std::make_shared<model::ThermalLoad>("T1");
    auto loadCase = std::make_shared<model::LoadCase>("LC");
    loadCase->addEntry({thermal->id(), 1.0});

    auto combination = std::make_shared<model::LoadCombination>(
        "COMBO", model::StressCategory::Sustained, model::CombineMethod::Algebraic);
    combination->addCaseEntry({loadCase->id(), 1.0});

    doc.addObject(thermal);
    doc.addObject(loadCase);
    doc.addObject(combination);

    graph.rebuildLoadDependencyChain(doc);

    // 修改载荷属性后 markDirty，验证依赖链传播
    thermal->setOperatingTemp(150.0);
    graph.markDirty(thermal->id());
    auto dirtyIds = graph.collectDirty();
    EXPECT_DOUBLE_EQ(thermal->operatingTemp(), 150.0);
    EXPECT_TRUE(containsId(dirtyIds, thermal->id()));
    EXPECT_TRUE(containsId(dirtyIds, loadCase->id()));
    EXPECT_TRUE(containsId(dirtyIds, combination->id()));
    graph.clearDirty();
}
