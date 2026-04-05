# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50、T51、T52、T53、T54** 已全部完成。

**T54 完成摘要（2026-04-05）**：
- 从 `src/app/CMakeLists.txt` 移除了 `${CMAKE_SOURCE_DIR}/src/engine/RecomputeEngine.cpp` 跨层源文件拼接
- 将 `RecomputeEngine.cpp` 加入 `src/engine/CMakeLists.txt` 的正式源文件列表
- `engine` 不需要在 CMake 中声明对 `app` 的链接依赖（避免循环）：静态库允许未解析外部符号，在最终链接时由 `app` 库解析
- 41/41 测试全部通过

**当前基线事实**：
- `engine` 静态库现已包含完整的 `RecomputeEngine.cpp`，对外正常暴露 `engine::RecomputeEngine` 接口
- `app` 通过正常 `target_link_libraries(app PUBLIC engine)` 链接使用 `RecomputeEngine`（无跨层源文件拼接）
- `pipecad_lib` INTERFACE target 已建立，位于 `src/CMakeLists.txt`（T53 完成）
- `src/CMakeLists.txt` 中 `pipecad_app`（EXE）仍沿用旧名，T55 改名为 `pipecad`
- 代码仍处于旧目录结构：`src/foundation`、`src/geometry`、`src/model`、`src/engine`、`src/app`、`src/command`、`src/ui`

## 下一个任务

**T55 — 设计过渡兼容层**

工作目标：
1. 将可执行目标 `pipecad_app`（EXE）改名为 `pipecad`（EXE）
2. 在 `src/CMakeLists.txt` 中引入新的 `pipecad_app`（STATIC）作为业务库占位（初期 = ui 层）
3. 确保测试链接不受影响（测试不直接链接 `pipecad_app` EXE）
4. 更新 `src/CMakeLists.txt`、`src/ui/CMakeLists.txt`（如需）
5. 编译通过，测试 41/41 通过

推荐模型：**GPT-5.3 Codex**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/t53-cmake-topology.md`（§3.3 T55 后目标关系图）
2. `src/CMakeLists.txt`（当前可执行目标定义）
3. `src/ui/CMakeLists.txt`（ui 静态库定义）
4. `docs/tasks/status.md`（confirm T55 is ready）

