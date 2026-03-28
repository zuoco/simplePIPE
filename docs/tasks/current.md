# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务（推荐）

| 属性 | 值 |
|------|---|
| **任务 ID** | T11 / T16 / T20 / T21 |
| **任务名** | OCCT→VSG 网格转换 / 应用层核心 / JSON 序列化 / STEP 导出 |
| **推荐模型** | Sonnet(T11,T20,T21) / **Opus**(T16) |
| **前置依赖** | 全部满足 ✅ |
| **前置状态** | ✅ 4 个任务均可立即开始 |

## 项目进度

- 已完成: 11/22 个任务（T01-T10）
- 当前阶段: Phase 3 完成 → 可进入 Phase 4/5/6

## 上一个完成的任务

**T10 — 拓扑管理与约束 (2026-03-28)**
- 产出:
  - `src/engine/TopologyManager.h/cpp` — Tee 自动分叉、管点增删一致性维护
  - `src/engine/ConstraintSolver.h/cpp` — 口径匹配检查 + Bend 角度约束
  - `src/engine/PipelineValidator.h/cpp` — 未连接端口警告 + OCCT 干涉检测
  - `tests/test_topology.cpp` — 22 个测试全部通过
- 关键接口:
  - `TopologyManager::appendPoint(route, seg, point)` → 若 Tee，返回 branch Segment
  - `TopologyManager::removePoint(route, id)` → 删 Tee 同时删分支
  - `ConstraintSolver::checkAll(route)` → 返回 `vector<ConstraintError>`
  - `PipelineValidator::checkInterference(shapes, ids, tol)` → `vector<ValidationWarning>`
- 注意事项:
  - engine CMakeLists 现在 PUBLIC 暴露 `${OpenCASCADE_INCLUDE_DIR}`（下游无需重复添加）
  - engine PRIVATE 链接 `${OpenCASCADE_ModelingAlgorithms_LIBRARIES}`（BRepExtrema 所在库）

## 给 AI 的指令

**推荐 T11（OCCT→VSG 网格转换）** — Sonnet:

1. 读取 `docs/development-plan.md` 中 **T11** 章节
2. 读取 `lib/vsg/AGENTS.md`
3. 前置代码: `src/geometry/ShapeMesher.h`（MeshData 结构）
4. 交付物: `src/visualization/OcctToVsg.h/cpp`
5. 完成后运行: `cd build/debug && cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$CONDA_PREFIX && ninja -j4 && ctest -R Visualization`
6. 更新 `docs/tasks/status.md` 和本文件

**其他可选任务**:

- **T16 (应用层核心)** — **Opus**: 读取 T16，前置: `src/model/*.h`, `src/engine/GeometryDeriver.h`, `src/engine/TopologyManager.h`（T10 新增）
- **T20 (JSON 序列化)** — Sonnet: 读取 T20，前置: `src/model/*.h`（含 T06 Accessory/Beam 类）
- **T21 (STEP 导出)** — Sonnet: 读取 T21，前置: `src/geometry/StepIO.h`, `src/engine/GeometryDeriver.h`
