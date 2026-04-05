# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50、T51、T52、T53、T54、T55** 已全部完成。

**T55 完成摘要（2026-04-05）**：
- 将可执行目标 `pipecad_app`（EXE）改名为 `pipecad`（EXE）
- 在 `src/CMakeLists.txt` 新增 `pipecad_app`（INTERFACE）业务库占位，初期链接 `ui + Qt6::{Quick,Qml}`，逻辑上等价于 ui 层；选用 INTERFACE 因为 M1 阶段不移动源文件
- `pipecad`（EXE）改为链接 `pipecad_app`（INTERFACE），不再直接链接 `ui` 和 Qt6
- 更新 `scripts/build.sh` 中所有可执行路径引用（`pipecad_app` → `pipecad`）
- 41/41 测试全部通过

**当前基线事实**：
- CMake 目标拓扑：`pipecad_lib(INTERFACE) → app`；`pipecad_app(INTERFACE) → ui + Qt6`；`pipecad(EXE) → pipecad_app`
- 三大核心目标完整落地：`pipecad_lib`（架构聚合）、`pipecad_app`（业务库占位）、`pipecad`（可执行）
- 测试链接旧 target（app/engine/ui 等）保持不变，零破坏
- 可执行二进制路径：`build/debug/src/pipecad`

## 下一个任务

**T56 — 建立 src/lib 目录骨架**

工作目标：
1. 创建 `src/lib/` 目录，内含 `base/`、`platform/`、`runtime/`、`framework/` 四个子目录（含 placeholder CMakeLists.txt）
2. 在 `src/CMakeLists.txt` 中引入 `add_subdirectory(lib)`
3. 每个子目录创建空的 `CMakeLists.txt` 作为命名空间占位
4. 确保编译通过，测试 41/41 通过

T57（建立 src/apps/pipecad 目录骨架）也已 ready，可视情况同步执行。

推荐模型：**GPT-5.3 Codex**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/m2-directory-migration.md`（T56、T57 规格）
2. `docs/tasks/phase4-lib-app-refactor/t53-cmake-topology.md`（§3.4 最终目标关系图）
3. `src/CMakeLists.txt`（当前顶层 CMake，了解 add_subdirectory 位置）
4. `docs/tasks/status.md`（确认 T56/T57 状态均为 ready）

