// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/DependencyGraph.h"
#include "app/Document.h"

#include <memory>
#include <string>

namespace app {

/// 工程 JSON 序列化/反序列化。
class ProjectSerializer {
public:
    static bool save(const Document& document, const std::string& filePath);

    /// 读取 JSON 工程并重建对象关系，失败返回 nullptr。
    /// 可选传入 dependencyGraph，用于在加载后重建 Load->LoadCase->LoadCombination 依赖链。
    static std::unique_ptr<Document> load(const std::string& filePath,
                                          DependencyGraph* dependencyGraph = nullptr);
};

} // namespace app
