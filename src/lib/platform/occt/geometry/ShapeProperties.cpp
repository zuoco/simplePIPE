// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "geometry/ShapeProperties.h"

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>

namespace geometry {

double ShapeProperties::volume(const TopoDS_Shape& shape) {
    GProp_GProps props;
    BRepGProp::VolumeProperties(shape, props);
    return props.Mass();
}

double ShapeProperties::surfaceArea(const TopoDS_Shape& shape) {
    GProp_GProps props;
    BRepGProp::SurfaceProperties(shape, props);
    return props.Mass();
}

} // namespace geometry
