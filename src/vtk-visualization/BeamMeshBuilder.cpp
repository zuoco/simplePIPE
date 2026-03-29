#include "vtk-visualization/BeamMeshBuilder.h"

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>

namespace vtk_vis {

vtkSmartPointer<vtkPolyData> buildBeamMesh(const std::vector<gp_Pnt>& centerline) {
    if (centerline.size() < 2) {
        return nullptr;
    }

    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(static_cast<vtkIdType>(centerline.size()));

    for (vtkIdType i = 0; i < static_cast<vtkIdType>(centerline.size()); ++i) {
        const gp_Pnt& p = centerline[static_cast<size_t>(i)];
        points->SetPoint(i, p.X(), p.Y(), p.Z());
    }

    auto polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(static_cast<vtkIdType>(centerline.size()));
    for (vtkIdType i = 0; i < static_cast<vtkIdType>(centerline.size()); ++i) {
        polyLine->GetPointIds()->SetId(i, i);
    }

    auto lines = vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell(polyLine);

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    return polyData;
}

} // namespace vtk_vis
