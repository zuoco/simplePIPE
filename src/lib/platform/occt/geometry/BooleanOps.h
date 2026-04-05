// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "geometry/OcctTypes.h"

namespace geometry {

// ============================================================
// BooleanOps — OCCT 布尔运算封装
// ============================================================

class BooleanOps {
public:
    // 差集: s1 - s2
    static TopoDS_Shape cut(const TopoDS_Shape& s1, const TopoDS_Shape& s2);

    // 并集: s1 ∪ s2
    static TopoDS_Shape fuse(const TopoDS_Shape& s1, const TopoDS_Shape& s2);
};

} // namespace geometry
