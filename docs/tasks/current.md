# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T25 |
| **任务名** | 集成测试与端到端验证 |
| **推荐模型** | Opus |
| **前置依赖** | 全部 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 21/22 个任务
- 当前阶段: Phase 6 — 集成验证

## 上一个完成的任务

T21 — STEP 导出 (2026-03-28)
- 产出: `src/app/StepExporter.h/.cpp`，`tests/test_step_exporter.cpp`，以及 app/tests 构建接入
- 关键接口: `app::StepExporter::exportAll(const Document&, const std::string&)`
- 注意事项: 导出层级为 Route→Segment→Component；当前仅覆盖 PipePoint 推导几何

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T25** 章节
2. 读取 `docs/architecture.md` 中 **§14 验证计划** 与 Phase 6 集成验证相关章节
3. 读取前置代码: `src/app/StepExporter.h`、`src/app/ProjectSerializer.h`、`src/engine/RecomputeEngine.h`、`src/ui/AppController.h`（若存在）
4. 如需库指南: 读取 `lib/occt/CLAUDE.md`、`lib/vsg/AGENTS.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
