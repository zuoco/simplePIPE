// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "vtk-visualization/BeamMeshBuilder.h"
#include "vtk-visualization/OcctToVtk.h"

#include "geometry/ShapeBuilder.h"

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

#include <vector>

TEST(OcctToVtk, CylinderToPolyData) {
    TopoDS_Shape cylinder = geometry::ShapeBuilder::makeCylinder(5.0, 20.0);

    vtkSmartPointer<vtkPolyData> polyData = vtk_vis::toVtkPolyData(cylinder, 0.1);

    ASSERT_NE(polyData, nullptr);
    EXPECT_GT(polyData->GetNumberOfPoints(), 0);
    EXPECT_GT(polyData->GetNumberOfPolys(), 0);

    vtkDataArray* normals = polyData->GetPointData()->GetNormals();
    ASSERT_NE(normals, nullptr);
    EXPECT_EQ(normals->GetNumberOfTuples(), polyData->GetNumberOfPoints());
}

TEST(OcctToVtk, BendLikeShapeToPolyData) {
    TopoDS_Shape bend = geometry::ShapeBuilder::makeTorus(30.0, 5.0, 1.57079632679);

    vtkSmartPointer<vtkPolyData> polyData = vtk_vis::toVtkPolyData(bend, 0.2);

    ASSERT_NE(polyData, nullptr);
    EXPECT_GT(polyData->GetNumberOfPoints(), 0);
    EXPECT_GT(polyData->GetNumberOfPolys(), 0);
}

TEST(OcctToVtk, EmptyShapeReturnsNull) {
    TopoDS_Shape empty;

    vtkSmartPointer<vtkPolyData> polyData = vtk_vis::toVtkPolyData(empty);

    EXPECT_EQ(polyData, nullptr);
}

TEST(BeamMeshBuilder, BuildsSinglePolylineCell) {
    std::vector<gp_Pnt> centerline = {
        gp_Pnt(0.0, 0.0, 0.0),
        gp_Pnt(100.0, 0.0, 0.0),
        gp_Pnt(150.0, 50.0, 0.0),
    };

    vtkSmartPointer<vtkPolyData> polyData = vtk_vis::buildBeamMesh(centerline);

    ASSERT_NE(polyData, nullptr);
    EXPECT_EQ(polyData->GetNumberOfPoints(), 3);
    EXPECT_EQ(polyData->GetNumberOfLines(), 1);
}

TEST(BeamMeshBuilder, TooFewPointsReturnsNull) {
    std::vector<gp_Pnt> centerline = {gp_Pnt(0.0, 0.0, 0.0)};

    vtkSmartPointer<vtkPolyData> polyData = vtk_vis::buildBeamMesh(centerline);

    EXPECT_EQ(polyData, nullptr);
}
