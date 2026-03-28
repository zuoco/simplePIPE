# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T31 |
| **任务名** | ComponentCatalog 参数化构件模板库 |
| **推荐模型** | **Opus** (opus4.6) |
| **前置依赖** | T08, T09 — 已完成 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 23/38 个任务 (Phase 1: 22/22, Phase 2: 1/16)
- 当前阶段: Phase 2 — 二期开发 (参数化构件模板、三工作台、载荷分析、VTK)

## 上一个完成的任务

T30 — ViewManager 视图管理器 (2026-03-29)
- 产出: `src/visualization/ViewManager.h/cpp`, `tests/test_view_manager.cpp`
- 关键接口: `ViewManager` — 统一门面，管理 VSG/VTK 双视口路由、渲染模式、可见性、相机状态缓存、LOD
- 注意事项: VTK 分支预留未实现，captureImage 为空实现

## 当前 ready 任务

| ID | 任务名 | 推荐模型 |
|----|--------|---------|
| T31 | ComponentCatalog 参数化构件模板库 | **Opus** (opus4.6) |
| T32 | Load 载荷数据模型 | Sonnet (sonnet4.6) |

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T31** 章节
2. 读取 `docs/architecture.md` **§3.7** (ComponentCatalog/ComponentTemplate)
3. 读取前置代码: `src/engine/GeometryDeriver.h`, `src/geometry/PipeGeometryBuilder.h`, `src/geometry/ElbowBuilder.h`, `src/geometry/TeeBuilder.h`, `src/geometry/ReducerBuilder.h`, `src/engine/ValveBuilder.h`, `src/engine/AccessoryBuilder.h`, `src/engine/BeamBuilder.h`
4. 完成后运行 `pixi run build-debug && pixi run test`
5. 验证通过后更新 `docs/tasks/status.md` 和本文件
