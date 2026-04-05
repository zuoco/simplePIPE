# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50–T75** 已完成。

**T75 完成摘要（2026-04-06）**：
- 将旧兼容别名与 `vtk_visualization` 目标定义统一迁移到 `src/CMakeLists.txt`
- 删除旧目录兼容层中的 `CMakeLists.txt`：`src/foundation/`、`src/geometry/`、`src/visualization/`、`src/vtk-visualization/`、`src/app/`、`src/command/`、`src/model/`、`src/engine/`、`src/ui/`
- 验证通过：Debug 构建成功，测试 **46/46** 全部通过

**当前基线事实**：
- 测试基线 46/46 全部通过
- `src/lib` 与 `src/apps` 已成为唯一 CMake 构建入口目录
- T76（固化 pipecad app 模板）前置依赖（T62/T75）均已完成，状态 `ready`

## 下一个任务

**T76 — 固化 pipecad app 模板**

工作目标：
将当前 `src/apps/pipecad/` 的目录与 CMake 组织提炼为可复用模板，明确未来新增 app 时的最小复制路径与必须改动点，确保新增 app 不需要重新设计 lib 边界。

建议聚焦：
1. 固化 `src/apps/CMakeLists.txt` 与 `src/apps/pipecad/CMakeLists.txt` 的模板化结构
2. 明确 app 子目录（model/engine/workbench/ui/main）的职责边界与最小骨架
3. 补充模板化说明并保持现有构建与测试基线稳定

参考任务卡：`docs/tasks/phase4-lib-app-refactor/m6-closure.md` 中 T76 章节。

推荐模型：**GPT-5.3 Codex**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/m6-closure.md`（T76 验收标准）
2. `docs/tasks/phase4-lib-app-refactor/t50-directory-target-freeze.md`（app 模板命名规则）
3. `src/apps/CMakeLists.txt`
4. `src/apps/pipecad/CMakeLists.txt`
5. `src/apps/pipecad/main.cpp`
6. `docs/lib-app-refactor-plan.md`（M6 收口要求）
7. `docs/tasks/status.md`

