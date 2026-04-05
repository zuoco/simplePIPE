# 文档导航

本目录用于承载当前开发文档、任务状态与历史归档。

## 目录结构

- `architecture.md`
  - 当前架构设计与分层说明
- `tasks/`
  - 当前任务执行入口（`current.md` + `status.md`）
- `archive/`
  - 已完成阶段的任务规格与完成记录
  - 包含 `task-specs/phase4-lib-app-refactor-plan.md`（Phase 4 架构重构实施计划书，历史实施基线）
  - 包含 `task-specs/phase4-lib-app-refactor/`（Phase 4 任务卡目录，已归档）

## 推荐阅读顺序

1. `tasks/current.md`：获取当前上下文和下一个任务
2. `tasks/status.md`：确认任务状态、依赖与完成索引
3. `architecture.md`：理解当前代码结构与系统边界
4. `archive/`：仅在追溯历史决策时按需阅读

## 维护规则

- `docs/` 根目录只保留当前有效入口与基线文档
- 已完成阶段的大型设计稿、阶段日志统一进入 `archive/`
- 避免在根目录保留与 `archive/` 内容重复的历史副本
