#include "vtk-visualization/OcctToVtk.h"

#include "geometry/ShapeMesher.h"

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

namespace vtk_vis {

vtkSmartPointer<vtkPolyData> toVtkPolyData(const TopoDS_Shape& shape,
                                           double deflection) {
    if (shape.IsNull()) {
        return nullptr;
    }

    geometry::MeshData mesh = geometry::ShapeMesher::mesh(shape, deflection);
    if (mesh.vertices.empty() || mesh.indices.empty()) {
        return nullptr;
    }

    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToFloat();
    points->SetNumberOfPoints(static_cast<vtkIdType>(mesh.vertices.size()));

    auto normals = vtkSmartPointer<vtkFloatArray>::New();
    normals->SetName("Normals");
    normals->SetNumberOfComponents(3);
    normals->SetNumberOfTuples(static_cast<vtkIdType>(mesh.normals.size()));

    for (vtkIdType i = 0; i < static_cast<vtkIdType>(mesh.vertices.size()); ++i) {
        const auto& v = mesh.vertices[static_cast<size_t>(i)];
        points->SetPoint(i, v[0], v[1], v[2]);
    }

    for (vtkIdType i = 0; i < static_cast<vtkIdType>(mesh.normals.size()); ++i) {
        const auto& n = mesh.normals[static_cast<size_t>(i)];
        normals->SetTuple3(i, n[0], n[1], n[2]);
    }

    auto polys = vtkSmartPointer<vtkCellArray>::New();
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        const vtkIdType tri[3] = {
            static_cast<vtkIdType>(mesh.indices[i]),
            static_cast<vtkIdType>(mesh.indices[i + 1]),
            static_cast<vtkIdType>(mesh.indices[i + 2]),
        };
        polys->InsertNextCell(3, tri);
    }

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polys);
    polyData->GetPointData()->SetNormals(normals);

    return polyData;
}

} // namespace vtk_vis
