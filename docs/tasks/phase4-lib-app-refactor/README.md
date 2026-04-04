# Phase 4 任务卡索引

本目录用于承载 PipeCAD lib/apps 架构重构的正式任务卡。

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

## 使用规则

1. 先读 `docs/tasks/current.md` 获取当前上下文和下一个任务。
2. 再读 `docs/tasks/status.md` 确认任务状态与依赖。
3. 按任务 ID 到本目录对应文件中读取正式任务卡。
4. 每次只执行一个任务，完成后同步更新状态表、日志和 current 文件。