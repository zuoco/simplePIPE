#include "geometry/BooleanOps.h"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>

namespace geometry {

TopoDS_Shape BooleanOps::cut(const TopoDS_Shape& s1, const TopoDS_Shape& s2) {
    BRepAlgoAPI_Cut cutter(s1, s2);
    cutter.Build();
    if (!cutter.IsDone()) return TopoDS_Shape();
    return cutter.Shape();
}

TopoDS_Shape BooleanOps::fuse(const TopoDS_Shape& s1, const TopoDS_Shape& s2) {
    BRepAlgoAPI_Fuse fuser(s1, s2);
    fuser.Build();
    if (!fuser.IsDone()) return TopoDS_Shape();
    return fuser.Shape();
}

} // namespace geometry
