// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "model/Load.h"
#include "model/DeadWeightLoad.h"
#include "model/ThermalLoad.h"
#include "model/PressureLoad.h"
#include "model/WindLoad.h"
#include "model/SeismicLoad.h"
#include "model/DisplacementLoad.h"
#include "model/UserDefinedLoad.h"
#include "foundation/Math.h"

#include <memory>
#include <string>

using Vec3 = foundation::math::Vec3;

// ============================================================
// Load 基类 — affectedObjects 管理
// ============================================================

TEST(Load, AffectedObjects_AddAndGet) {
    model::DeadWeightLoad load("W");
    EXPECT_TRUE(load.affectedObjects().empty());

    auto id1 = foundation::UUID::generate();
    auto id2 = foundation::UUID::generate();
    load.addAffectedObject(id1);
    load.addAffectedObject(id2);

    auto objs = load.affectedObjects();
    ASSERT_EQ(objs.size(), 2u);
    EXPECT_EQ(objs[0], id1);
    EXPECT_EQ(objs[1], id2);
}

TEST(Load, AffectedObjects_NoDuplicates) {
    model::DeadWeightLoad load("W");
    auto id = foundation::UUID::generate();
    load.addAffectedObject(id);
    load.addAffectedObject(id); // 重复添加不应产生多条记录
    EXPECT_EQ(load.affectedObjects().size(), 1u);
}

TEST(Load, AffectedObjects_Remove) {
    model::DeadWeightLoad load("W");
    auto id = foundation::UUID::generate();
    load.addAffectedObject(id);
    EXPECT_TRUE(load.removeAffectedObject(id));
    EXPECT_TRUE(load.affectedObjects().empty());
}

TEST(Load, AffectedObjects_RemoveNonExistent) {
    model::DeadWeightLoad load("W");
    auto id = foundation::UUID::generate();
    EXPECT_FALSE(load.removeAffectedObject(id));
}

TEST(Load, Changed_Signal_OnAddAffectedObject) {
    model::DeadWeightLoad load("W");
    int count = 0;
    load.changed.connect([&]() { ++count; });
    auto id = foundation::UUID::generate();
    load.addAffectedObject(id);
    EXPECT_EQ(count, 1);
    load.addAffectedObject(id); // 重复，不触发信号
    EXPECT_EQ(count, 1);
}

TEST(Load, Changed_Signal_OnRemoveAffectedObject) {
    model::DeadWeightLoad load("W");
    auto id = foundation::UUID::generate();
    load.addAffectedObject(id);
    int count = 0;
    load.changed.connect([&]() { ++count; });
    load.removeAffectedObject(id);
    EXPECT_EQ(count, 1);
}

// ============================================================
// Load — UUID 和名称继承自 DocumentObject
// ============================================================

TEST(Load, InheritsDocumentObject_UUID) {
    model::DeadWeightLoad a("W1");
    model::DeadWeightLoad b("W2");
    EXPECT_NE(a.id(), b.id());
    EXPECT_FALSE(a.id().isNull());
}

TEST(Load, InheritsDocumentObject_Name) {
    model::DeadWeightLoad load("DeadLoad1");
    EXPECT_EQ(load.name(), "DeadLoad1");
    load.setName("DeadLoad2");
    EXPECT_EQ(load.name(), "DeadLoad2");
}

// ============================================================
// DeadWeightLoad
// ============================================================

TEST(DeadWeightLoad, LoadType) {
    model::DeadWeightLoad load;
    EXPECT_EQ(load.loadType(), "DeadWeight");
}

TEST(DeadWeightLoad, DefaultName) {
    model::DeadWeightLoad load;
    EXPECT_EQ(load.name(), "DeadWeight");
}

// ============================================================
// ThermalLoad
// ============================================================

TEST(ThermalLoad, LoadType) {
    model::ThermalLoad load;
    EXPECT_EQ(load.loadType(), "Thermal");
}

TEST(ThermalLoad, DefaultTemperatures) {
    model::ThermalLoad load;
    EXPECT_DOUBLE_EQ(load.installTemp(), 20.0);
    EXPECT_DOUBLE_EQ(load.operatingTemp(), 20.0);
}

TEST(ThermalLoad, SetGetInstallTemp) {
    model::ThermalLoad load;
    load.setInstallTemp(15.0);
    EXPECT_DOUBLE_EQ(load.installTemp(), 15.0);
}

TEST(ThermalLoad, SetGetOperatingTemp) {
    model::ThermalLoad load;
    load.setOperatingTemp(150.0);
    EXPECT_DOUBLE_EQ(load.operatingTemp(), 150.0);
}

TEST(ThermalLoad, Changed_Signal_OnTempChange) {
    model::ThermalLoad load;
    int count = 0;
    load.changed.connect([&]() { ++count; });
    load.setInstallTemp(10.0);
    EXPECT_EQ(count, 1);
    load.setOperatingTemp(200.0);
    EXPECT_EQ(count, 2);
    load.setOperatingTemp(200.0); // 相同值不触发
    EXPECT_EQ(count, 2);
}

// ============================================================
// PressureLoad
// ============================================================

TEST(PressureLoad, LoadType) {
    model::PressureLoad load;
    EXPECT_EQ(load.loadType(), "Pressure");
}

TEST(PressureLoad, Defaults) {
    model::PressureLoad load;
    EXPECT_DOUBLE_EQ(load.pressure(), 0.0);
    EXPECT_FALSE(load.isExternal());
}

TEST(PressureLoad, SetGetPressure) {
    model::PressureLoad load;
    load.setPressure(2.5);
    EXPECT_DOUBLE_EQ(load.pressure(), 2.5);
}

TEST(PressureLoad, SetGetIsExternal) {
    model::PressureLoad load;
    load.setIsExternal(true);
    EXPECT_TRUE(load.isExternal());
}

TEST(PressureLoad, Changed_Signal) {
    model::PressureLoad load;
    int count = 0;
    load.changed.connect([&]() { ++count; });
    load.setPressure(1.0);
    EXPECT_EQ(count, 1);
    load.setIsExternal(true);
    EXPECT_EQ(count, 2);
}

// ============================================================
// WindLoad
// ============================================================

TEST(WindLoad, LoadType) {
    model::WindLoad load;
    EXPECT_EQ(load.loadType(), "Wind");
}

TEST(WindLoad, Defaults) {
    model::WindLoad load;
    EXPECT_DOUBLE_EQ(load.speed(), 0.0);
    EXPECT_DOUBLE_EQ(load.direction().x, 1.0);
    EXPECT_DOUBLE_EQ(load.direction().y, 0.0);
    EXPECT_DOUBLE_EQ(load.direction().z, 0.0);
}

TEST(WindLoad, SetGetSpeed) {
    model::WindLoad load;
    load.setSpeed(30.0);
    EXPECT_DOUBLE_EQ(load.speed(), 30.0);
}

TEST(WindLoad, SetGetDirection) {
    model::WindLoad load;
    load.setDirection(Vec3{0.0, 1.0, 0.0});
    EXPECT_DOUBLE_EQ(load.direction().x, 0.0);
    EXPECT_DOUBLE_EQ(load.direction().y, 1.0);
    EXPECT_DOUBLE_EQ(load.direction().z, 0.0);
}

TEST(WindLoad, Changed_Signal) {
    model::WindLoad load;
    int count = 0;
    load.changed.connect([&]() { ++count; });
    load.setSpeed(20.0);
    EXPECT_EQ(count, 1);
    load.setDirection(Vec3{0.0, 0.0, 1.0});
    EXPECT_EQ(count, 2);
}

// ============================================================
// SeismicLoad
// ============================================================

TEST(SeismicLoad, LoadType) {
    model::SeismicLoad load;
    EXPECT_EQ(load.loadType(), "Seismic");
}

TEST(SeismicLoad, Defaults) {
    model::SeismicLoad load;
    EXPECT_DOUBLE_EQ(load.acceleration(), 0.0);
    EXPECT_DOUBLE_EQ(load.direction().z, 1.0);
}

TEST(SeismicLoad, SetGetAcceleration) {
    model::SeismicLoad load;
    load.setAcceleration(0.3);
    EXPECT_DOUBLE_EQ(load.acceleration(), 0.3);
}

TEST(SeismicLoad, SetGetDirection) {
    model::SeismicLoad load;
    load.setDirection(Vec3{1.0, 0.0, 0.0});
    EXPECT_DOUBLE_EQ(load.direction().x, 1.0);
}

TEST(SeismicLoad, Changed_Signal) {
    model::SeismicLoad load;
    int count = 0;
    load.changed.connect([&]() { ++count; });
    load.setAcceleration(0.2);
    EXPECT_EQ(count, 1);
    load.setAcceleration(0.2); // 相同值不触发
    EXPECT_EQ(count, 1);
}

// ============================================================
// DisplacementLoad
// ============================================================

TEST(DisplacementLoad, LoadType) {
    model::DisplacementLoad load;
    EXPECT_EQ(load.loadType(), "Displacement");
}

TEST(DisplacementLoad, Defaults) {
    model::DisplacementLoad load;
    EXPECT_DOUBLE_EQ(load.translation().x, 0.0);
    EXPECT_DOUBLE_EQ(load.translation().y, 0.0);
    EXPECT_DOUBLE_EQ(load.translation().z, 0.0);
    EXPECT_DOUBLE_EQ(load.rotation().x, 0.0);
}

TEST(DisplacementLoad, SetGetTranslation) {
    model::DisplacementLoad load;
    load.setTranslation(Vec3{5.0, 10.0, 0.0});
    EXPECT_DOUBLE_EQ(load.translation().x, 5.0);
    EXPECT_DOUBLE_EQ(load.translation().y, 10.0);
}

TEST(DisplacementLoad, SetGetRotation) {
    model::DisplacementLoad load;
    load.setRotation(Vec3{0.0, 0.0, 45.0});
    EXPECT_DOUBLE_EQ(load.rotation().z, 45.0);
}

TEST(DisplacementLoad, Changed_Signal) {
    model::DisplacementLoad load;
    int count = 0;
    load.changed.connect([&]() { ++count; });
    load.setTranslation(Vec3{1.0, 0.0, 0.0});
    EXPECT_EQ(count, 1);
    load.setRotation(Vec3{0.0, 1.0, 0.0});
    EXPECT_EQ(count, 2);
}

// ============================================================
// UserDefinedLoad
// ============================================================

TEST(UserDefinedLoad, LoadType) {
    model::UserDefinedLoad load;
    EXPECT_EQ(load.loadType(), "UserDefined");
}

TEST(UserDefinedLoad, Defaults) {
    model::UserDefinedLoad load;
    EXPECT_DOUBLE_EQ(load.force().x, 0.0);
    EXPECT_DOUBLE_EQ(load.force().y, 0.0);
    EXPECT_DOUBLE_EQ(load.force().z, 0.0);
    EXPECT_DOUBLE_EQ(load.moment().x, 0.0);
}

TEST(UserDefinedLoad, SetGetForce) {
    model::UserDefinedLoad load;
    load.setForce(Vec3{100.0, 200.0, 300.0});
    EXPECT_DOUBLE_EQ(load.force().x, 100.0);
    EXPECT_DOUBLE_EQ(load.force().y, 200.0);
    EXPECT_DOUBLE_EQ(load.force().z, 300.0);
}

TEST(UserDefinedLoad, SetGetMoment) {
    model::UserDefinedLoad load;
    load.setMoment(Vec3{0.0, 0.0, 500.0});
    EXPECT_DOUBLE_EQ(load.moment().z, 500.0);
}

TEST(UserDefinedLoad, Changed_Signal) {
    model::UserDefinedLoad load;
    int count = 0;
    load.changed.connect([&]() { ++count; });
    load.setForce(Vec3{1.0, 0.0, 0.0});
    EXPECT_EQ(count, 1);
    load.setMoment(Vec3{0.0, 0.0, 100.0});
    EXPECT_EQ(count, 2);
}

// ============================================================
// 多态性 — 通过 Load* 使用子类
// ============================================================

TEST(Load, Polymorphism_LoadType) {
    std::vector<std::unique_ptr<model::Load>> loads;
    loads.push_back(std::make_unique<model::DeadWeightLoad>());
    loads.push_back(std::make_unique<model::ThermalLoad>());
    loads.push_back(std::make_unique<model::PressureLoad>());
    loads.push_back(std::make_unique<model::WindLoad>());
    loads.push_back(std::make_unique<model::SeismicLoad>());
    loads.push_back(std::make_unique<model::DisplacementLoad>());
    loads.push_back(std::make_unique<model::UserDefinedLoad>());

    std::vector<std::string> expected = {
        "DeadWeight", "Thermal", "Pressure", "Wind",
        "Seismic", "Displacement", "UserDefined"
    };
    ASSERT_EQ(loads.size(), expected.size());
    for (std::size_t i = 0; i < loads.size(); ++i) {
        EXPECT_EQ(loads[i]->loadType(), expected[i]);
    }
}

TEST(Load, Polymorphism_AffectedObjects) {
    std::unique_ptr<model::Load> load = std::make_unique<model::ThermalLoad>("T1");
    auto id = foundation::UUID::generate();
    load->addAffectedObject(id);
    EXPECT_EQ(load->affectedObjects().size(), 1u);
    EXPECT_EQ(load->affectedObjects()[0], id);
}
