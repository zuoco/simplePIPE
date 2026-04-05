// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

// OCCT 核心类型别名与前置声明
// 统一管理 geometry 层使用的 OCCT 类型

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <gp_Trsf.hxx>
#include <Standard_Handle.hxx>

// 前置声明常用类型
class TopoDS_Wire;
class TopoDS_Edge;
class TopoDS_Face;
class Geom_Curve;
class Geom_Surface;

namespace geometry {

// 常用别名，简化代码
using OcctShape = TopoDS_Shape;

} // namespace geometry
