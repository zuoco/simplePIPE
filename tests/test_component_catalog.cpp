#include <gtest/gtest.h>

#include "engine/ComponentCatalog.h"
#include "engine/ComponentTemplate.h"
#include "geometry/ShapeProperties.h"

#include <TopoDS_Shape.hxx>
#include <algorithm>
#include <vector>
#include <string>

using namespace engine;

class ComponentCatalogTest : public ::testing::Test {
protected:
    ComponentCatalog& catalog = ComponentCatalog::instance();
};

// ============================================================
// Catalog 基础功能测试
// ============================================================

TEST_F(ComponentCatalogTest, HasAtLeast8Templates) {
    EXPECT_GE(catalog.size(), 8u);
}

TEST_F(ComponentCatalogTest, AllTemplateIdsNonEmpty) {
    auto ids = catalog.allTemplateIds();
    EXPECT_GE(ids.size(), 8u);
    for (const auto& id : ids) {
        EXPECT_FALSE(id.empty());
    }
}

TEST_F(ComponentCatalogTest, GetTemplateReturnsNonNullForRegistered) {
    auto ids = catalog.allTemplateIds();
    for (const auto& id : ids) {
        EXPECT_NE(catalog.getTemplate(id), nullptr) << "Template not found: " << id;
    }
}

TEST_F(ComponentCatalogTest, GetTemplateReturnsNullForUnknown) {
    EXPECT_EQ(catalog.getTemplate("NonExistentTemplate"), nullptr);
}

TEST_F(ComponentCatalogTest, ContainsExpected8Templates) {
    std::vector<std::string> expected = {
        "Pipe", "Elbow", "Tee", "Reducer",
        "GateValve", "WeldNeckFlange", "RigidSupport", "SpringHanger"
    };
    for (const auto& id : expected) {
        EXPECT_NE(catalog.getTemplate(id), nullptr) << "Missing template: " << id;
    }
}

// ============================================================
// 每种模板的参数化验证 (OD=168.3 和 OD=323.8)
// ============================================================

struct TemplateTestParam {
    std::string templateId;
    double od;
    double wt;
};

class TemplateShapeTest : public ::testing::TestWithParam<TemplateTestParam> {};

TEST_P(TemplateShapeTest, BuildShapeReturnsNonNullWithPositiveVolume) {
    const auto& p = GetParam();
    auto* tpl = ComponentCatalog::instance().getTemplate(p.templateId);
    ASSERT_NE(tpl, nullptr) << "Template not found: " << p.templateId;
    EXPECT_EQ(tpl->templateId(), p.templateId);

    auto params = tpl->deriveParams(p.od, p.wt);
    EXPECT_DOUBLE_EQ(params.od, p.od);
    EXPECT_DOUBLE_EQ(params.wallThickness, p.wt);

    auto shape = tpl->buildShape(params);
    EXPECT_FALSE(shape.IsNull()) << p.templateId << " returned null shape for OD=" << p.od;

    double vol = geometry::ShapeProperties::volume(shape);
    EXPECT_GT(vol, 0.0) << p.templateId << " volume <= 0 for OD=" << p.od;
}

// 每种模板用两组管径测试 (OD=168.3mm/WT=7.11mm 和 OD=323.8mm/WT=9.53mm)
INSTANTIATE_TEST_SUITE_P(
    AllTemplates,
    TemplateShapeTest,
    ::testing::Values(
        // OD = 168.3 mm (6")
        TemplateTestParam{"Pipe",             168.3, 7.11},
        TemplateTestParam{"Elbow",            168.3, 7.11},
        TemplateTestParam{"Tee",              168.3, 7.11},
        TemplateTestParam{"Reducer",          168.3, 7.11},
        TemplateTestParam{"GateValve",        168.3, 7.11},
        TemplateTestParam{"WeldNeckFlange",   168.3, 7.11},
        TemplateTestParam{"RigidSupport",     168.3, 7.11},
        TemplateTestParam{"SpringHanger",     168.3, 7.11},
        // OD = 323.8 mm (12")
        TemplateTestParam{"Pipe",             323.8, 9.53},
        TemplateTestParam{"Elbow",            323.8, 9.53},
        TemplateTestParam{"Tee",              323.8, 9.53},
        TemplateTestParam{"Reducer",          323.8, 9.53},
        TemplateTestParam{"GateValve",        323.8, 9.53},
        TemplateTestParam{"WeldNeckFlange",   323.8, 9.53},
        TemplateTestParam{"RigidSupport",     323.8, 9.53},
        TemplateTestParam{"SpringHanger",     323.8, 9.53}
    )
);

// ============================================================
// 模板 deriveParams 参数合理性检查
// ============================================================

TEST_F(ComponentCatalogTest, PipeParamsScaleWithOD) {
    auto* tpl = catalog.getTemplate("Pipe");
    ASSERT_NE(tpl, nullptr);
    auto p1 = tpl->deriveParams(168.3, 7.11);
    auto p2 = tpl->deriveParams(323.8, 9.53);
    EXPECT_GT(p2.bodyLength, p1.bodyLength);
}

TEST_F(ComponentCatalogTest, GateValveParamsScaleWithOD) {
    auto* tpl = catalog.getTemplate("GateValve");
    ASSERT_NE(tpl, nullptr);
    auto p1 = tpl->deriveParams(168.3, 7.11);
    auto p2 = tpl->deriveParams(323.8, 9.53);
    EXPECT_GT(p2.bodyWidth, p1.bodyWidth);
    EXPECT_GT(p2.bodyHeight, p1.bodyHeight);
    EXPECT_GT(p2.get("handwheelDia"), p1.get("handwheelDia"));
}

TEST_F(ComponentCatalogTest, ElbowParamsHaveBendRadius) {
    auto* tpl = catalog.getTemplate("Elbow");
    ASSERT_NE(tpl, nullptr);
    auto p = tpl->deriveParams(168.3, 7.11);
    EXPECT_GT(p.get("bendRadius"), 0.0);
    EXPECT_GT(p.get("bendAngle"), 0.0);
}

TEST_F(ComponentCatalogTest, ReducerParamsHaveEndOD) {
    auto* tpl = catalog.getTemplate("Reducer");
    ASSERT_NE(tpl, nullptr);
    auto p = tpl->deriveParams(168.3, 7.11);
    double endOD = p.get("endOD");
    EXPECT_GT(endOD, 0.0);
    EXPECT_LT(endOD, 168.3);  // 小端 < 大端
}

TEST_F(ComponentCatalogTest, WeldNeckFlangeBodyWidthGtOD) {
    auto* tpl = catalog.getTemplate("WeldNeckFlange");
    ASSERT_NE(tpl, nullptr);
    auto p = tpl->deriveParams(168.3, 7.11);
    EXPECT_GT(p.bodyWidth, p.od); // 法兰外径 > 管外径
}

TEST_F(ComponentCatalogTest, RigidSupportHasBasePlateAndColumn) {
    auto* tpl = catalog.getTemplate("RigidSupport");
    ASSERT_NE(tpl, nullptr);
    auto p = tpl->deriveParams(168.3, 7.11);
    EXPECT_GT(p.get("basePlateEdge"), 0.0);
    EXPECT_GT(p.get("columnHeight"), 0.0);
}

TEST_F(ComponentCatalogTest, SpringHangerHasSpringAndRod) {
    auto* tpl = catalog.getTemplate("SpringHanger");
    ASSERT_NE(tpl, nullptr);
    auto p = tpl->deriveParams(168.3, 7.11);
    EXPECT_GT(p.get("springDia"), 0.0);
    EXPECT_GT(p.get("rodDia"), 0.0);
    EXPECT_GT(p.get("rodHeight"), 0.0);
}

// ============================================================
// 体积随管径增大单调增
// ============================================================

TEST_F(ComponentCatalogTest, VolumeIncreasesWithOD) {
    std::vector<std::string> ids = {
        "Pipe", "Elbow", "GateValve", "WeldNeckFlange",
        "RigidSupport", "SpringHanger"
    };
    for (const auto& id : ids) {
        auto* tpl = catalog.getTemplate(id);
        ASSERT_NE(tpl, nullptr) << id;
        auto p1 = tpl->deriveParams(168.3, 7.11);
        auto p2 = tpl->deriveParams(323.8, 9.53);
        auto s1 = tpl->buildShape(p1);
        auto s2 = tpl->buildShape(p2);
        ASSERT_FALSE(s1.IsNull()) << id << " OD=168.3";
        ASSERT_FALSE(s2.IsNull()) << id << " OD=323.8";
        double v1 = geometry::ShapeProperties::volume(s1);
        double v2 = geometry::ShapeProperties::volume(s2);
        EXPECT_GT(v2, v1) << id << " volume should increase with OD";
    }
}
