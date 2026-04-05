# Phase 4 任务卡索引

本目录用于承载 PipeCAD lib/apps 架构重构的已归档任务卡。

## 任务编号范围

- T50-T52：M0 规则冻结
- T53-T55：M1 构建重编组
- T56-T62：M2 目录迁移
- T63-T66：M3 lib 模块化
- T67-T70：M4 并发基础设施
- T71-T74：M5 异步重算落地
- T75-T77：M6 收口与模板化

## 任务卡文件

- `m0-rule-freeze.md`
- `m1-build-reorg.md`
- `m2-directory-migration.md`
- `m3-lib-modules.md`
- `m4-concurrency-foundation.md`
- `m5-async-recompute.md`
- `m6-closure.md`
- `t50-directory-target-freeze.md`
- `t51-include-import-freeze.md`
- `t52-thread-boundary-freeze.md`
- `t53-cmake-topology.md`
- `t70-sync-policy.md`
- `t76-pipecad-app-template.md`

说明：
- 里程碑卡（`m*.md`）覆盖每个阶段的完整任务范围。
- 专项任务卡（`t*.md`）用于沉淀关键规则或收口任务的详细设计。

## 使用规则

1. 本目录仅用于回溯 Phase 4 设计与实施细节，不再作为当前任务入口。
2. 当前会话仍应先读 `docs/tasks/current.md` 和 `docs/tasks/status.md`。
3. 仅在需要追溯 T50-T77 的正式任务卡时，再按任务 ID 到本目录读取。