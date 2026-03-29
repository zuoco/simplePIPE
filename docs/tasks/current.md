# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T44 |
| **任务名** | AnalysisWorkbench 工作台 |
| **推荐模型** | Opus |
| **前置依赖** | T33, T39, T42 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 43/45 个任务
- 当前阶段: Phase 12 — AnalysisWorkbench

## 上一个完成的任务

T43 — 序列化扩展 (Load/LoadCase) (2026-03-29)
- 产出: `ProjectSerializer.h/cpp`, `DependencyGraph.h/cpp`, `TransactionManager.cpp`, `test_load_serialization.cpp`
- 关键接口: `ProjectSerializer::load(path, DependencyGraph*)`；`DependencyGraph::rebuildLoadDependencyChain(const Document&)`
- 注意事项: 读档后可直接重建 `Load -> LoadCase -> LoadCombination` 依赖；entries/caseEntries 的事务回放仍需后续补齐。

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T44** 章节
2. 读取 `docs/architecture.md` **§6.6** 相关章节
3. 读取前置代码: `src/app/AnalysisWorkbench.h/cpp`, `src/ui/WorkbenchController.*`, `src/vtk-visualization/VtkViewport.*`
4. 如需领域知识或库指南：读取 `.github/skills/industrial-software-dev/SKILL.md` 与 `lib/vtk/AGENTS.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
