# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50–T63** 已全部完成。

**T59/T60/T61/T63 完成摘要（2026-04-05）**：
- **T59**: 将 `src/geometry/` 所有文件复制至 `src/lib/platform/occt/geometry/`，建立 `lib_platform_occt` STATIC + `lib::platform::occt` ALIAS；`src/geometry/CMakeLists.txt` 改为 `ALIAS lib_platform_occt`
- **T60**: 将 `src/visualization/` 复制至 `src/lib/platform/vsg/visualization/`，建立 `lib_platform_vsg`；将 `vtk-visualization` 算法层（OcctToVtk/BeamMeshBuilder/VtkSceneManager）复制至 `src/lib/platform/vtk/vtk-visualization/`，建立 `lib_platform_vtk`（无Qt）；`vtk_visualization` 保留 VtkViewport 并链接 `lib_platform_vtk`；`src/visualization/CMakeLists.txt` 改为 `ALIAS lib_platform_vsg`
- **T61**: 将 `src/command/` 全部文件复制至 `src/lib/runtime/command/`；Document/DependencyGraph/SelectionManager 复制至 `src/lib/runtime/app/`，建立 `lib_runtime` STATIC；Application/Workbench 系列复制至 `src/lib/framework/app/`，建立 `lib_framework` STATIC；`src/app/CMakeLists.txt` 改为 `ALIAS lib_framework`
- **T63**: 在 `src/lib/base/baseMod/` 创建三个 C++20 模块接口单元（`pipecad.base.math.cppm`、`pipecad.base.types.cppm`、`pipecad.base.cppm`），建立 `lib_base_modules` STATIC 目标（FILE_SET CXX_MODULES, C++20）
- 258/258 编译通过，41/41 测试全部通过

**当前基线事实**：
- `lib_platform_occt` STATIC 已存在，`geometry` 为其 ALIAS
- `lib_platform_vsg` STATIC 已存在，`visualization` 为其 ALIAS
- `lib_platform_vtk` STATIC 已存在（纯算法，无Qt）；`vtk_visualization` 仅保留 VtkViewport
- `lib_runtime` STATIC 已存在（Document/DependencyGraph/SelectionManager + command 全部文件）
- `lib_framework` STATIC 已存在（Application/Workbench/ProjectSerializer/StepExporter），`app` 为其 ALIAS
- `lib_base_modules` STATIC 已存在（C++20 模块接口单元）
- ready 任务：T62（model/engine/ui/main → apps）、T64（lib/platform facade 模块）、T65（lib/runtime 核心模块）
- 可执行路径：`build/debug/src/pipecad`

## 下一个任务

**T62 — 迁移 model/engine/ui/main 到 apps/pipecad**

工作目标：
1. 将 `src/model/`、`src/engine/`、`src/ui/` 所有 `.h` 和 `.cpp` 文件复制到 `src/apps/pipecad/model/`、`src/apps/pipecad/engine/`、`src/apps/pipecad/ui/`
2. 将 `src/main.cpp` 复制到 `src/apps/pipecad/main.cpp`
3. 在 `src/apps/pipecad/model/CMakeLists.txt`、`engine/CMakeLists.txt`、`ui/CMakeLists.txt` 建立对应 STATIC 目标（lib::apps::pipecad::model 等），或根据实际 m2 规格确定目标名
4. 更新 `src/model/CMakeLists.txt`、`src/engine/CMakeLists.txt`、`src/ui/CMakeLists.txt` 改为向后兼容别名
5. 确保编译通过，41/41 测试全部通过

同时可并行推进 T64（lib/platform facade 模块）和 T65（lib/runtime 核心模块）。

推荐模型：**GPT-5.4 Codex**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/m2-directory-migration.md`（T62 规格）
2. `docs/tasks/phase4-lib-app-refactor/m3-lib-modules.md`（T64/T65 规格）
3. `src/model/CMakeLists.txt`（当前 model 构建文件）
4. `src/engine/CMakeLists.txt`（当前 engine 构建文件）
5. `src/ui/CMakeLists.txt`（当前 ui 构建文件）
6. `src/apps/pipecad/model/CMakeLists.txt`（当前占位文件）
7. `src/apps/pipecad/engine/CMakeLists.txt`（当前占位文件）
8. `src/apps/pipecad/ui/CMakeLists.txt`（当前占位文件）
9. `docs/tasks/status.md`（确认 T62 状态为 ready）

