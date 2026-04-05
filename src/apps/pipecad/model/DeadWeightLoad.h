// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/Load.h"

namespace model {

/// 自重载荷：由管件材料密度 + 壁厚自动计算，无额外参数
class DeadWeightLoad : public Load {
public:
    explicit DeadWeightLoad(const std::string& name = "DeadWeight")
        : Load(name) {}

    std::string loadType() const override { return "DeadWeight"; }
};

} // namespace model
