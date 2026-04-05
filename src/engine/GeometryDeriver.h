// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0
//
// T62 兼容 shim：物理迁移后此文件与 src/apps/pipecad/engine/GeometryDeriver.h 保持同步。

#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <vector>
#include <memory>

namespace model { class PipePoint; class PipeSpec; }

// 快照类型前向声明
namespace app {
    struct PipePointSnapshot;
    struct PipeSpecSnapshot;
}

namespace engine {

/// 统一几何推导入口
/// 根据管点类型分发到对应的 Builder 生成 BRep 几何
class GeometryDeriver {
public:
    /// 为给定管点生成几何体（同步路径）
    static TopoDS_Shape deriveGeometry(
        const gp_Pnt& prevPoint,
        const std::shared_ptr<model::PipePoint>& current,
        const gp_Pnt& nextPoint);

    /// 为给定管点快照生成几何体（异步路径，后台线程安全）
    static TopoDS_Shape deriveFromSnapshot(
        const gp_Pnt& prevPoint,
        const app::PipePointSnapshot& current,
        const app::PipeSpecSnapshot* spec,
        const gp_Pnt& nextPoint);
};

} // namespace engine

