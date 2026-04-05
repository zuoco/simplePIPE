---
description: "启动一个 PipeCAD 开发任务（T 编号），引导 AI 读取状态文件、确认依赖、实现代码并完成收口。使用方式：/start-task T78"
---

# 启动任务 {{taskId}}

## Step 1 — 读取当前状态

读取 `docs/tasks/current.md`，获取：
- 上一个任务的上下文摘要
- 下一个任务 ID 与工作描述
- 推荐模型
- 需要读取的文件列表

## Step 2 — 确认前置依赖

读取 `docs/tasks/status.md`，找到任务 **{{taskId}}**：
- 确认前置依赖均为 `done`
- 若有未完成依赖，报告阻塞原因并停止

## Step 3 — 读取任务详情

根据任务编号选择正确的任务卡：
- Phase 4 (T50-T77+)：`docs/archive/task-specs/phase4-lib-app-refactor/` 目录
- Phase 3 (T0-T10)：`docs/archive/task-specs/command-pattern-design.md`
- Phase 1-2 (T01-T45)：`docs/archive/task-specs/development-plan.md`

获取：交付物列表、接口定义、验收标准。

## Step 4 — 读取前置上下文

按 `current.md` 中列出的文件逐一读取。优先读头文件，比读日志更准确。

## Step 5 — 实现代码

- 按交付物列表创建/修改文件
- C++17 标准，遵循项目代码风格
- 必须包含单元测试

## Step 6 — 编译验证

```bash
mkdir -p tmp_build_logs
pixi run build-debug > tmp_build_logs/build.log 2>&1
pixi run test > tmp_build_logs/test.log 2>&1
tail -50 tmp_build_logs/build.log
tail -50 tmp_build_logs/test.log
rm -rf tmp_build_logs
```

全部测试通过后才可继续。

## Step 7 — 收口

按顺序执行：
1. 更新 `docs/tasks/status.md`：将 {{taskId}} 标记为 `done`，后续任务状态从 `pending` → `ready`
2. 追加完成记录到对应日志文件（Phase 4 → `docs/archive/task-logs/phase4-refactor.md`）
3. Git 提交：`git add -A && git commit -m "feat: {{taskId}} — 功能描述"`
4. **清空并重写** `docs/tasks/current.md`，写入下一任务上下文

## Step 8 — 输出

报告：完成文件列表、测试是否通过、Git 提交信息、下一步指令（含推荐模型）。
