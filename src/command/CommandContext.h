// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

// 前向声明：CommandContext 只持有指针，不依赖 app/engine 层的完整头文件
namespace app {
    class Document;
    class DependencyGraph;
}

namespace engine {
    class TopologyManager;
}

namespace command {

/// 命令执行上下文：命令通过此结构访问运行时依赖，而不直接持有引用
/// 由 Application::createCommandContext() 便利方法构建
/// 除拓扑结构命令外，topologyManager 通常为 nullptr
struct CommandContext {
    app::Document*           document          = nullptr; ///< 文档对象管理器
    app::DependencyGraph*    dependencyGraph   = nullptr; ///< 依赖图
    engine::TopologyManager* topologyManager   = nullptr; ///< 拓扑管理器（可选）
};

} // namespace command
