// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "geometry/OcctTypes.h"

#include <string>
#include <vector>

namespace geometry {

class StepIO {
public:
    /// 将多个 Shape 导出为 STEP 文件
    /// @return true 成功，false 失败
    static bool exportStep(const std::vector<TopoDS_Shape>& shapes,
                           const std::string& filePath);

    /// 从 STEP 文件导入 Shape 列表
    /// @return 导入的 Shape 列表（失败时为空）
    static std::vector<TopoDS_Shape> importStep(const std::string& filePath);
};

} // namespace geometry
