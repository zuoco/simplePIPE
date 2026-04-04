# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）和 Phase 3（T0-T10）已全部完成。

在现有基线之上，已新增 **Phase 4 — lib/apps 架构重构** 的实施计划与正式任务体系：
- 总体实施计划书已创建：`docs/lib-app-refactor-plan.md`
- 正式任务卡已拆分到：`docs/tasks/phase4-lib-app-refactor/`
- 新阶段状态已追加到：`docs/tasks/status.md`

**当前基线事实**：
- 代码仍处于旧目录结构：`src/foundation`、`src/geometry`、`src/model`、`src/engine`、`src/app`、`src/command`、`src/ui` 等
- `TransactionManager` 已移除，命令模式由 `CommandStack` 体系承接
- `app` 静态库仍吞并 `command` 与 `RecomputeEngine.cpp`，这是 Phase 4 的关键结构阻塞点之一
- 当前接力目标已切换到重构阶段，而不是继续旧 Phase 任务

## 下一个任务

**T50 — 冻结目录与目标命名规则**

工作目标：
1. 将 `src/lib` 与 `src/apps` 固化为唯一长期代码根目录
2. 冻结 `pipecad_lib`、`pipecad_app`、`pipecad` 等目标命名规则
3. 明确未来新增 app 的目录模板与命名约定

推荐模型：**GPT-5.4**

## 需要读取的文件

1. `docs/lib-app-refactor-plan.md`
2. `docs/tasks/status.md`
3. `docs/tasks/phase4-lib-app-refactor/README.md`
4. `docs/tasks/phase4-lib-app-refactor/m0-rule-freeze.md`
5. `src/CMakeLists.txt`
6. `src/app/CMakeLists.txt`
