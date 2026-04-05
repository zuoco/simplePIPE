# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50、T51、T52、T53、T54、T55、T56、T57** 已全部完成。

**T56/T57 完成摘要（2026-04-05）**：
- `src/lib/` 目录骨架建立：`base/`、`platform/{occt,vsg,vtk}/`、`runtime/`、`framework/` 七个子目录，各含占位 CMakeLists.txt
- `src/apps/` 目录骨架建立：`pipecad/` 目录模板含 `model/`、`engine/`、`workbench/`、`ui/` 四子域，各含占位 CMakeLists.txt
- `src/CMakeLists.txt` 新增 `add_subdirectory(lib)` + `add_subdirectory(apps)`（位于 add_subdirectory(ui) 之后）
- 41/41 测试全部通过，221/221 编译步骤成功

**当前基线事实**：
- CMake 目标拓扑（T55 已落地）：`pipecad_lib(INTERFACE) → app`；`pipecad_app(INTERFACE) → ui + Qt6`；`pipecad(EXE) → pipecad_app`
- `src/lib/` 与 `src/apps/pipecad/` 目录骨架均已存在，但无源文件（占位 CMakeLists.txt）
- T58（ready）、T59（ready）、T60（ready）、T61（ready）四个迁移任务可独立推进
- 可执行路径：`build/debug/src/pipecad`

## 下一个任务

**T58 — 迁移 foundation 到 src/lib/base**

工作目标：
1. 将 `src/foundation/` 下所有头文件（含 UUID、Variant、Math、Signal、Log 等）迁移/复制到 `src/lib/base/`
2. 在 `src/lib/base/CMakeLists.txt` 建立正式的 INTERFACE 或 STATIC 目标（`lib_base` 或 `lib::base`）
3. 在 `src/foundation/CMakeLists.txt` 建立向后兼容别名 `foundation → lib::base`，确保旧链接代码不破坏
4. 更新 `src/CMakeLists.txt` 中 `pipecad_lib` 的链接关系（按需）
5. 确保编译通过，41/41 测试全部通过

T59（迁移 geometry）、T60（拆分 visualization）、T61（迁移 app/command）亦均为 ready，可并行推进。

推荐模型：**GPT-5.4 Codex**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/m2-directory-migration.md`（T58 规格）
2. `docs/tasks/phase4-lib-app-refactor/t53-cmake-topology.md`（最终目标关系图 §3.4）
3. `src/foundation/CMakeLists.txt`（当前 foundation 构建文件）
4. `src/lib/base/CMakeLists.txt`（当前占位文件）
5. `src/CMakeLists.txt`（顶层 CMake，了解 pipecad_lib 链接关系）
6. `docs/tasks/status.md`（确认 T58 状态为 ready）

