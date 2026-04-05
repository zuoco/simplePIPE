// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

export module pipecad.platform.occt;

export namespace geometry {
class BooleanOps;
class ShapeBuilder;
class ShapeMesher;
class ShapeProperties;
class ShapeTransform;
class StepIO;
}

export namespace pipecad::platform::occt {
using BooleanOps = ::geometry::BooleanOps;
using ShapeBuilder = ::geometry::ShapeBuilder;
using ShapeMesher = ::geometry::ShapeMesher;
using ShapeProperties = ::geometry::ShapeProperties;
using ShapeTransform = ::geometry::ShapeTransform;
using StepIO = ::geometry::StepIO;
}