# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50–T76** 已完成。

**T76 完成摘要（2026-04-06）**：
- 固化 `src/apps/CMakeLists.txt`：引入 `PIPECAD_APP_LIST` 与 `pipecad_register_app()`，统一 app 注册并校验模板骨架（model/engine/workbench/ui/resources/main.cpp）
- 固化 `src/apps/pipecad/CMakeLists.txt`：引入 `PIPECAD_APP_ID` 与 `PIPECAD_QML_MAIN_FILE`，由 app id 派生 app 主目标与子域目标命名
- 补齐 app 模板骨架：`src/apps/pipecad/workbench/` 新增可编译占位库，`src/apps/pipecad/resources/` 新增资源位与接口目标
- 新增模板说明文档：`docs/tasks/phase4-lib-app-refactor/t76-pipecad-app-template.md`
- 验证通过：Debug 构建成功，测试 **46/46** 全部通过

**当前基线事实**：
- 测试基线 46/46 全部通过
- `src/lib` 与 `src/apps` 已成为唯一 CMake 构建入口目录
- app 模板骨架与 CMake 模板已固化，可按 `<name>_app` + `<name>` 规则扩展
- T77（更新文档与开发规范）前置依赖（T75）已完成，状态 `ready`

## 下一个任务

**T77 — 更新文档与开发规范**

工作目标：
将架构文档、开发流程说明与任务接力文档更新到 `src/lib + src/apps` 新结构，确保后续开发者仅依赖新结构即可开展工作。

建议聚焦：
1. 更新架构文档中目录结构、目标依赖图与命名规则到 T76 基线
2. 更新开发流程文档中的构建、测试、迁移规则（强调旧目录不再作为构建入口）
3. 更新任务接力说明，使新会话能以 `current.md -> status.md -> Phase4 任务卡` 无歧义接续
4. 执行构建与全量测试，确认文档改动不引入脚本/路径回归

参考任务卡：`docs/tasks/phase4-lib-app-refactor/m6-closure.md` 中 T77 章节。

推荐模型：**GPT-5.3 Codex**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/m6-closure.md`（T77 验收标准）
2. `docs/lib-app-refactor-plan.md`（M6 收口要求）
3. `docs/architecture.md`（架构正文与目录结构）
4. `README.md`（开发入口说明）
5. `AGENTS.md`（AI 工作流规范）
6. `CLAUDE.md`（开发代理指令同步）
7. `docs/tasks/status.md`
8. `docs/tasks/current.md`（本文件，更新后再自检）

