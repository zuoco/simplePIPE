# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T21 |
| **任务名** | STEP 导出 |
| **推荐模型** | Sonnet |
| **前置依赖** | T08, T09, T04 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 21/22 个任务
- 当前阶段: Phase 6 — 数据交换

## 上一个完成的任务

T20 — JSON 序列化 (2026-03-28)
- 产出: `src/app/ProjectSerializer.h/.cpp`，`tests/test_project_serializer.cpp`，以及构建与模型基类更新
- 关键接口: `app::ProjectSerializer::save/load`；`DocumentObject::setIdForDeserialization()`
- 注意事项: load 后已自动执行全量 `RecomputeEngine::recomputeAll()`；JSON round-trip 稳定性已由测试覆盖

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T21** 章节
2. 读取 `docs/architecture.md` 中数据交换/STEP 导出相关章节
3. 读取前置代码: `src/app/Document.h`、`src/app/ProjectSerializer.h`、`src/engine/GeometryDeriver.h`、`src/engine/RunBuilder.h`、`src/engine/BendBuilder.h`、`src/engine/ReducerBuilder.h`、`src/engine/TeeBuilder.h`、`src/engine/ValveBuilder.h`、`src/engine/FlexJointBuilder.h`、`src/geometry/StepIO.h`
4. 如需库指南: 读取 `lib/occt/AGENTS.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
