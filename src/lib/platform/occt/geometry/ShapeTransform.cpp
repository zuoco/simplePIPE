// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "geometry/ShapeTransform.h"

#include <BRepBuilderAPI_Transform.hxx>

namespace geometry {

TopoDS_Shape ShapeTransform::translate(const TopoDS_Shape& shape, const gp_Vec& vec) {
    gp_Trsf t;
    t.SetTranslation(vec);
    return transform(shape, t);
}

TopoDS_Shape ShapeTransform::rotate(const TopoDS_Shape& shape, const gp_Ax1& axis, double angle) {
    gp_Trsf t;
    t.SetRotation(axis, angle);
    return transform(shape, t);
}

TopoDS_Shape ShapeTransform::transform(const TopoDS_Shape& shape, const gp_Trsf& trsf) {
    BRepBuilderAPI_Transform builder(shape, trsf, /*copy=*/true);
    return builder.Shape();
}

} // namespace geometry
