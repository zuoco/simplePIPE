#include <gtest/gtest.h>

#include "vtk-visualization/VtkSceneManager.h"

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyLine.h>
#include <vtkRenderer.h>

namespace {

vtkSmartPointer<vtkActor> makeSolidActor() {
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(0.0, 0.0, 0.0);
    points->InsertNextPoint(1.0, 0.0, 0.0);
    points->InsertNextPoint(0.0, 1.0, 0.0);

    auto polys = vtkSmartPointer<vtkCellArray>::New();
    const vtkIdType tri[3] = {0, 1, 2};
    polys->InsertNextCell(3, tri);

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polys);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    return actor;
}

vtkSmartPointer<vtkActor> makeBeamActor() {
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(0.0, 0.0, 0.0);
    points->InsertNextPoint(1.0, 0.0, 0.0);
    points->InsertNextPoint(2.0, 0.5, 0.0);

    auto line = vtkSmartPointer<vtkPolyLine>::New();
    line->GetPointIds()->SetNumberOfIds(3);
    line->GetPointIds()->SetId(0, 0);
    line->GetPointIds()->SetId(1, 1);
    line->GetPointIds()->SetId(2, 2);

    auto lines = vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell(line);

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    return actor;
}

bool rendererContainsActor(vtkRenderer* renderer, vtkActor* actor) {
    auto actors = renderer->GetActors();
    actors->InitTraversal();
    while (vtkActor* current = actors->GetNextActor()) {
        if (current == actor) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST(VtkSceneManager, RendererIsValid) {
    vtk_vis::VtkSceneManager scene;
    EXPECT_NE(scene.renderer(), nullptr);
}

TEST(VtkSceneManager, AddAndRemoveActor) {
    vtk_vis::VtkSceneManager scene;
    auto solidActor = makeSolidActor();

    scene.addActor("solid-1", solidActor);
    EXPECT_TRUE(scene.hasActor("solid-1"));
    EXPECT_EQ(scene.actorCount(), 1u);
    EXPECT_TRUE(rendererContainsActor(scene.renderer(), solidActor));

    scene.removeActor("solid-1");
    EXPECT_FALSE(scene.hasActor("solid-1"));
    EXPECT_EQ(scene.actorCount(), 0u);
    EXPECT_FALSE(rendererContainsActor(scene.renderer(), solidActor));
}

TEST(VtkSceneManager, UpdateActorReplacesExistingActor) {
    vtk_vis::VtkSceneManager scene;
    auto oldActor = makeSolidActor();
    auto newActor = makeSolidActor();

    scene.addActor("solid-1", oldActor);
    scene.updateActor("solid-1", newActor);

    EXPECT_EQ(scene.actorCount(), 1u);
    EXPECT_TRUE(rendererContainsActor(scene.renderer(), newActor));
    EXPECT_FALSE(rendererContainsActor(scene.renderer(), oldActor));
}

TEST(VtkSceneManager, RenderModeTogglesSolidAndBeamVisibility) {
    vtk_vis::VtkSceneManager scene;
    auto solidActor = makeSolidActor();
    auto beamActor = makeBeamActor();

    scene.addActor("solid-1", solidActor);
    scene.addActor("beam-1", beamActor);

    EXPECT_EQ(solidActor->GetVisibility(), 1);
    EXPECT_EQ(beamActor->GetVisibility(), 0);

    scene.setRenderMode(vtk_vis::VtkSceneManager::RenderMode::Beam);
    EXPECT_EQ(solidActor->GetVisibility(), 0);
    EXPECT_EQ(beamActor->GetVisibility(), 1);

    scene.setRenderMode(vtk_vis::VtkSceneManager::RenderMode::Solid);
    EXPECT_EQ(solidActor->GetVisibility(), 1);
    EXPECT_EQ(beamActor->GetVisibility(), 0);
}