// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "geometry/ShapeMesher.h"
#include "geometry/StepIO.h"
#include "geometry/ShapeProperties.h"
#include "geometry/ShapeBuilder.h"

#include <filesystem>
#include <cmath>

// ============================================================
// ShapeMesher Tests
// ============================================================

TEST(ShapeMesher, Cylinder_HasVertices) {
    auto shape = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    auto mesh  = geometry::ShapeMesher::mesh(shape, 0.2);
    EXPECT_GT(mesh.vertices.size(), 0u);
}

TEST(ShapeMesher, Cylinder_HasIndices) {
    auto shape = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    auto mesh  = geometry::ShapeMesher::mesh(shape, 0.2);
    EXPECT_GT(mesh.indices.size(), 0u);
    EXPECT_EQ(mesh.indices.size() % 3, 0u); // 三角面
}

TEST(ShapeMesher, NormalsMatchVertexCount) {
    auto shape = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    auto mesh  = geometry::ShapeMesher::mesh(shape, 0.2);
    EXPECT_EQ(mesh.vertices.size(), mesh.normals.size());
}

TEST(ShapeMesher, NormalsAreUnitLength) {
    auto shape = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    auto mesh  = geometry::ShapeMesher::mesh(shape, 0.2);
    for (const auto& n : mesh.normals) {
        float len = std::sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
        // 仅检查非零法线
        if (len > 1e-6f) {
            EXPECT_NEAR(len, 1.0f, 0.01f);
        }
    }
}

TEST(ShapeMesher, IndicesInRange) {
    auto shape = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    auto mesh  = geometry::ShapeMesher::mesh(shape, 0.2);
    auto vSize = static_cast<uint32_t>(mesh.vertices.size());
    for (uint32_t idx : mesh.indices) {
        EXPECT_LT(idx, vSize);
    }
}

TEST(ShapeMesher, FinerDeflection_MoreTriangles) {
    auto shape   = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    auto coarse  = geometry::ShapeMesher::mesh(shape, 1.0);
    auto fine    = geometry::ShapeMesher::mesh(shape, 0.05);
    EXPECT_GT(fine.indices.size(), coarse.indices.size());
}

TEST(ShapeMesher, Torus_NotEmpty) {
    auto shape = geometry::ShapeBuilder::makeTorus(20.0, 5.0);
    auto mesh  = geometry::ShapeMesher::mesh(shape, 0.5);
    EXPECT_GT(mesh.vertices.size(), 0u);
    EXPECT_GT(mesh.indices.size(),  0u);
}

// ============================================================
// ShapeProperties Tests
// ============================================================

TEST(ShapeProperties, CylinderVolume) {
    const double r = 5.0, h = 20.0;
    auto shape = geometry::ShapeBuilder::makeCylinder(r, h);
    double vol = geometry::ShapeProperties::volume(shape);
    double expected = M_PI * r * r * h;
    EXPECT_NEAR(vol, expected, expected * 1e-6);
}

TEST(ShapeProperties, CylinderSurfaceArea) {
    const double r = 5.0, h = 20.0;
    auto shape = geometry::ShapeBuilder::makeCylinder(r, h);
    double area = geometry::ShapeProperties::surfaceArea(shape);
    // 圆柱侧面 + 2*底面 = 2*π*r*h + 2*π*r²
    double expected = 2.0 * M_PI * r * h + 2.0 * M_PI * r * r;
    EXPECT_NEAR(area, expected, expected * 1e-4);
}

// ============================================================
// StepIO Tests
// ============================================================

TEST(StepIO, ExportCreatesFile) {
    auto shape = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    std::string path = "/tmp/test_t04_cylinder.step";
    bool ok = geometry::StepIO::exportStep({shape}, path);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(std::filesystem::exists(path));
    std::filesystem::remove(path);
}

TEST(StepIO, ExportThenImport_ShapeNotNull) {
    auto shape = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    std::string path = "/tmp/test_t04_roundtrip.step";

    bool exported = geometry::StepIO::exportStep({shape}, path);
    ASSERT_TRUE(exported);

    auto imported = geometry::StepIO::importStep(path);
    EXPECT_FALSE(imported.empty());
    EXPECT_FALSE(imported[0].IsNull());

    std::filesystem::remove(path);
}

TEST(StepIO, ExportThenImport_VolumePreserved) {
    auto shape = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    std::string path = "/tmp/test_t04_volume.step";

    double originalVol = geometry::ShapeProperties::volume(shape);
    geometry::StepIO::exportStep({shape}, path);

    auto imported   = geometry::StepIO::importStep(path);
    ASSERT_FALSE(imported.empty());
    double importedVol = geometry::ShapeProperties::volume(imported[0]);

    // 体积误差 < 1%
    EXPECT_NEAR(importedVol, originalVol, originalVol * 0.01);

    std::filesystem::remove(path);
}

TEST(StepIO, ImportNonExistentFile_ReturnsEmpty) {
    auto result = geometry::StepIO::importStep("/tmp/nonexistent_xyz_abc.step");
    EXPECT_TRUE(result.empty());
}

TEST(StepIO, MultipleShapes_ExportImport) {
    auto c1 = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);
    auto c2 = geometry::ShapeBuilder::makeCone(10.0, 5.0, 15.0);
    std::string path = "/tmp/test_t04_multi.step";

    bool ok = geometry::StepIO::exportStep({c1, c2}, path);
    EXPECT_TRUE(ok);

    auto imported = geometry::StepIO::importStep(path);
    EXPECT_FALSE(imported.empty());

    std::filesystem::remove(path);
}
