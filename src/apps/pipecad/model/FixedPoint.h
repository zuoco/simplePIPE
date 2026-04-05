// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/Accessory.h"

namespace model {

/// 固定点：固定约束标记
class FixedPoint : public Accessory {
public:
    explicit FixedPoint(const std::string& name = "",
                        const gp_Pnt& position = gp_Pnt(0, 0, 0))
        : Accessory(name, position) {}

    /// 固定点始终标记为固定约束
    bool isFixed() const { return true; }
};

} // namespace model
