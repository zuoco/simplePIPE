#include "geometry/StepIO.h"

#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_StepModelType.hxx>
#include <STEPControl_Writer.hxx>

namespace geometry {

bool StepIO::exportStep(const std::vector<TopoDS_Shape>& shapes,
                        const std::string& filePath) {
    STEPControl_Writer writer;
    for (const auto& shape : shapes) {
        if (shape.IsNull()) continue;
        IFSelect_ReturnStatus status = writer.Transfer(shape, STEPControl_AsIs);
        if (status != IFSelect_RetDone) return false;
    }
    IFSelect_ReturnStatus status = writer.Write(filePath.c_str());
    return status == IFSelect_RetDone;
}

std::vector<TopoDS_Shape> StepIO::importStep(const std::string& filePath) {
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
    if (status != IFSelect_RetDone) return {};

    reader.TransferRoots();

    std::vector<TopoDS_Shape> result;
    result.reserve(static_cast<size_t>(reader.NbShapes()));
    for (int i = 1; i <= reader.NbShapes(); ++i) {
        TopoDS_Shape shape = reader.Shape(i);
        if (!shape.IsNull()) result.push_back(shape);
    }
    return result;
}

} // namespace geometry
