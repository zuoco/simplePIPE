# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50–T74** 已全部完成。

**T74 完成摘要（2026-04-06）**：
- 新增 `tests/test_concurrent_recompute.cpp`（5 个并发回归测试）
- 覆盖：批量编辑 / 撤销重做版本丢弃 / 工作台切换 discard / 应用退出安全回收 / drainFresh 精确版本匹配
- 测试基线提升到 **46/46**（新增 ConcurrentRecompute 套件，耗时 0.11s）

**当前基线事实**：
- 测试基线 46/46 全部通过
- T75（清理旧目录兼容层）所有前置（T62/T64/T65/T66/T74）已 done，状态 `ready`

## 下一个任务

**T75 — 清理旧目录兼容层**

工作目标：
删除 `src/foundation/`、`src/geometry/`、`src/model/`、`src/engine/`、`src/visualization/`、`src/vtk-visualization/`、`src/app/`、`src/command/`、`src/ui/` 等旧目录中仅含 ALIAS CMakeLists，将所有旧目标别名迁移到顶层 `src/CMakeLists.txt` 或各 lib/apps 子目录，使项目目录结构与新架构保持一致。清理后需保证：
1. 全部构建目标仍然存在（ALIAS 别名不删除，只是搬移到合适位置）
2. 所有测试 46/46 继续通过
3. 旧目录下不再有 `CMakeLists.txt`

参考任务卡：`docs/tasks/phase4-lib-app-refactor/` 中 T75 章节。

推荐模型：**Claude Sonnet 4.6**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/` 目录下 T75 任务卡（如有）
2. `src/CMakeLists.txt`（顶层 CMake，了解当前 add_subdirectory 结构）
3. `src/foundation/CMakeLists.txt`、`src/geometry/CMakeLists.txt` 等旧目录（了解 ALIAS 内容）
4. `src/lib/base/CMakeLists.txt`、`src/apps/pipecad/CMakeLists.txt`（了解新结构）
5. `docs/tasks/status.md`（确认 T75 前置依赖）

