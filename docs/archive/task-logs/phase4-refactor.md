# Phase 4 完成记录 — lib/apps 架构重构 (T50–T77)

> **实施计划书**: `docs/lib-app-refactor-plan.md`
> **任务卡目录**: `docs/tasks/phase4-lib-app-refactor/`
> **任务状态**: `docs/tasks/status.md`

---

### T50 — 冻结目录与目标命名规则 (2026-04-05)

**产出文件**: `docs/tasks/phase4-lib-app-refactor/t50-directory-target-freeze.md` · `docs/tasks/phase4-lib-app-refactor/m0-rule-freeze.md` · `docs/lib-app-refactor-plan.md` · `src/CMakeLists.txt`

**接口**: → `docs/tasks/phase4-lib-app-refactor/t50-directory-target-freeze.md`

**设计决策**:
- 确立 `src/lib` 与 `src/apps` 为唯一长期代码根目录
- 架构统一静态库固定命名 `pipecad_lib`
- 当前 app 业务库固定命名 `pipecad_app`，可执行固定命名 `pipecad`
- 新增 app 采用 `<name>_app`（业务库）+ `<name>`（可执行）命名对
- 新增 app 目录模板固定为 `src/apps/<name>/model|engine|workbench|ui|resources|main.cpp`
- 旧 target（`app`、`command`）过渡期保留 ALIAS 壳，在 T75 清理

**已知限制**:
- 本任务仅冻结规则文档，代码目录结构与构建拓扑仍在旧结构，需等 T53–T62 逐步落地

### T51 — 冻结 include/import 规则 (2026-04-05)

**产出文件**: `docs/tasks/phase4-lib-app-refactor/t51-include-import-freeze.md` · `docs/tasks/phase4-lib-app-refactor/m0-rule-freeze.md` · `docs/lib-app-refactor-plan.md`

**接口**: → `docs/tasks/phase4-lib-app-refactor/t51-include-import-freeze.md`

**设计决策**:
- 冻结 include/import/CMake 依赖三类统一约束，规则同时覆盖 `#include`、`import`、`target_link_libraries`
- 冻结分层依赖方向：`lib::base -> lib::platform -> lib::runtime -> lib::framework -> apps/<name>`
- 明确 apps 禁止直接 include/import OCCT/VSG/VTK，第三方访问统一经 `lib::platform` 收口
- 明确 `lib::runtime` 禁止反向依赖 `lib::framework`（编译期与链接期均禁止）
- 明确禁止跨 app 直接依赖，跨 app 复用必须先下沉到 `src/lib`

**已知限制**:
- 当前仓库仍处于旧目录拓扑，历史违规 include 需在 T53/T55/T59-T62 迁移阶段逐步消除

### T52 — 冻结线程安全边界 (2026-04-05)

**产出文件**: `docs/tasks/phase4-lib-app-refactor/t52-thread-boundary-freeze.md` · `docs/tasks/phase4-lib-app-refactor/m0-rule-freeze.md`

**接口**: → `docs/tasks/phase4-lib-app-refactor/t52-thread-boundary-freeze.md`

**设计决策**:
- 冻结单一可变状态线程原则：所有写操作（Document/CommandStack/SceneManager/Qt对象）主线程独占
- 冻结主线程专属对象清单：CommandStack、Document 写接口、DependencyGraph 写接口、RecomputeEngine、SceneManager 写操作、SelectionManager 写接口、WorkbenchManager、所有 Qt/QML/VTK 对象
- 冻结后台线程允许行为：只读快照消费、const 方法、独立 OCCT 几何推导、Qt::QueuedConnection 信号发射
- 冻结快照边界组成：版本令牌（用于失效检测）+ PipePoint/PipeSpec 值拷贝 + 依赖拓扑只读视图
- 冻结结果回投协议：唯一合法通道为 Qt::QueuedConnection；回投前校验版本令牌，不匹配则静默丢弃
- 明确失效丢弃双重检查：版本号校验 + 对应 UUID 对象是否仍存在
- 明确迁移期约束：T68 前保持全串行，T68-T71 搭建并发框架，M4 后方可启用后台几何重算

**已知限制**:
- 当前系统全程单线程执行，本文档的并发规则在 T68-T71 才实际落地；本任务仅冻结设计约束

### T53 — 设计 lib/apps 顶层 CMake 拓扑 (2026-04-05)

**产出文件**: `docs/tasks/phase4-lib-app-refactor/t53-cmake-topology.md` · `src/CMakeLists.txt`

**接口**: → `docs/tasks/phase4-lib-app-refactor/t53-cmake-topology.md`

**设计决策**:
- 在 `src/CMakeLists.txt` 新增 `pipecad_lib`（INTERFACE，M1 过渡版），通过 `app` 传递引入全部 6 个旧架构子库（foundation/geometry/model/engine/vtk_visualization/visualization）及 command 源码和 RecomputeEngine.cpp
- 所有旧 target 名（foundation、geometry、model、engine、visualization、vtk_visualization、app、command、ui）保持不变，零破坏
- 可执行 `pipecad_app` 暂不改名（T55 负责，届时 `pipecad_app` 名释放给业务静态库，可执行改为 `pipecad`）
- 冻结三大核心目标职责：`pipecad_lib`（架构聚合，不含 ui）、`pipecad_app`（业务库，T55 后 = ui 层）、`pipecad`（可执行入口，T55 后正式命名）
- 冻结 T54-T62 实施前置约束（详见设计文档各节 "后续任务前置约束"）
- M1 使用 INTERFACE 避免实际编译单元：M2 目录迁移完成后升级为 STATIC

**已知限制**:
- `pipecad_lib` 当前为 INTERFACE 转发层，M2（T56-T62）目录物理迁移后才升级为真正的 STATIC 聚合库
- `model` 与 `engine` 按最终架构应归 apps 域，M1 期间仍暂留 lib 层平级目录（T62 物理迁移时解决）

### T54 — 解除 RecomputeEngine 源文件级拼接 (2026-04-05)

**产出文件**: `src/app/CMakeLists.txt` · `src/engine/CMakeLists.txt`

**接口**: → `src/engine/RecomputeEngine.h`（不变）、`src/engine/RecomputeEngine.cpp`（归还 engine 库）

**设计决策**:
- 从 `src/app/CMakeLists.txt` 删除 `${CMAKE_SOURCE_DIR}/src/engine/RecomputeEngine.cpp` 跨层引用
- 将 `RecomputeEngine.cpp` 加入 `src/engine/CMakeLists.txt` 的 STATIC 库源文件列表
- `engine` 不在 CMake 中声明对 `app` 的链接依赖（避免循环）：`engine.a` 允许对 `Document`/`DependencyGraph` 有未解析符号，这些符号在最终链接（通过 `app`）时得到解析
- 由于 `engine` 已有 `${CMAKE_SOURCE_DIR}/src` 作为 include 目录，`app/Document.h` 和 `app/DependencyGraph.h` 均可在编译期找到，无需修改 include 路径
- 仅使用 `RecomputeEngine` 的测试（`test_app_core`、`test_integration`、`test_phase2_integration`）均已链接 `app`，链接可正常解析
- 仅链接 `engine` 但不链接 `app` 的测试（`test_engine` 等）不引用 `RecomputeEngine`，linker 不会拉入 `RecomputeEngine.o`，因此无未解析符号问题

**已知限制**:
- `RecomputeEngine.h` 仍 include `app/Document.h` 和 `app/DependencyGraph.h`，从包含关系看是越层引用；T71（重构 RecomputeEngine 异步管线）时才真正解耦

### T55 — 设计过渡兼容层 (2026-04-05)

**产出文件**: `src/CMakeLists.txt` · `scripts/build.sh`

**接口**: → `src/CMakeLists.txt`（pipecad_app INTERFACE + pipecad EXE）

**设计决策**:
- 将可执行目标从 `pipecad_app`（EXE）改名为 `pipecad`（EXE）
- 在 `src/CMakeLists.txt` 新增 `pipecad_app`（INTERFACE）作为业务库占位，初期通过链接 `ui + Qt6` 等价于 ui 层；T57 物理迁移后升级为 STATIC
- `pipecad`（EXE）改为链接 `pipecad_app`（INTERFACE），不再直接链接 `ui` 和 Qt6
- `scripts/build.sh` 中所有 `pipecad_app` 可执行路径引用更新为 `pipecad`
- 测试不直接链接 `pipecad_app`（EXE），测试链接的旧 target（app/engine/ui 等）均保持不变，零破坏
- 选用 INTERFACE（而非 STATIC）是因为 M1 期间不移动源文件，STATIC 需要源文件，INTERFACE 直接转发依赖更简洁

**已知限制**:
- `pipecad_app` 当前为 INTERFACE，T57 后物理迁移 ui 源文件时才升级为 STATIC

### T56 — 建立 src/lib 目录骨架 (2026-04-05)

**产出文件**: `src/lib/CMakeLists.txt` · `src/lib/base/CMakeLists.txt` · `src/lib/platform/CMakeLists.txt` · `src/lib/platform/occt/CMakeLists.txt` · `src/lib/platform/vsg/CMakeLists.txt` · `src/lib/platform/vtk/CMakeLists.txt` · `src/lib/runtime/CMakeLists.txt` · `src/lib/framework/CMakeLists.txt` · `src/CMakeLists.txt`（新增 add_subdirectory(lib)）

**接口**: → `src/lib/CMakeLists.txt`（含 base/platform/runtime/framework 四子域）

**设计决策**:
- 严格按 t53-cmake-topology.md §3.4 最终拓扑创建目录：base / platform/{occt,vsg,vtk} / runtime / framework
- 各 CMakeLists.txt 均为空占位，注明最终迁入时间线（T58-T61）
- `src/CMakeLists.txt` 在 add_subdirectory(ui) 之后追加 add_subdirectory(lib)（物理内容尚未迁入，占位编译无误）
- platform 下分三子域（occt/vsg/vtk）与 t53 §3.4 保持一致，便于 T59/T60 独立推进

**已知限制**:
- 所有 lib 子目录当前均无源文件，CMakeLists.txt 仅为命名空间占位

### T57 — 建立 src/apps/pipecad 目录骨架 (2026-04-05)

**产出文件**: `src/apps/CMakeLists.txt` · `src/apps/pipecad/CMakeLists.txt` · `src/apps/pipecad/model/CMakeLists.txt` · `src/apps/pipecad/engine/CMakeLists.txt` · `src/apps/pipecad/workbench/CMakeLists.txt` · `src/apps/pipecad/ui/CMakeLists.txt` · `src/CMakeLists.txt`（新增 add_subdirectory(apps)）

**接口**: → `src/apps/CMakeLists.txt`（含 pipecad 子目录）；`src/apps/pipecad/CMakeLists.txt`（含 model/engine/workbench/ui 四子域）

**设计决策**:
- 严格按 t53 §3.4 与 t50 目录模板创建：model / engine / workbench / ui 四子域
- 各 CMakeLists.txt 均为空占位，注明最终迁入时间线（T62）并附注子域来源
- `src/CMakeLists.txt` 追加 add_subdirectory(apps)，与 add_subdirectory(lib) 并列
- 新增 app 时可直接复制 pipecad/ 目录模板并修改，验收标准已满足

**已知限制**:
- 所有 apps/pipecad 子目录当前均无源文件，CMakeLists.txt 仅为命名空间占位

### T58 — 迁移 foundation 到 src/lib/base (2026-04-05)

**产出文件**: `src/lib/base/foundation/Math.h` · `src/lib/base/foundation/Log.h` · `src/lib/base/foundation/Signal.h` · `src/lib/base/foundation/Types.h` · `src/lib/base/CMakeLists.txt` · `src/foundation/CMakeLists.txt` · `src/CMakeLists.txt`

**接口**: → `src/lib/base/CMakeLists.txt`（lib_base INTERFACE + lib::base ALIAS）；`src/foundation/CMakeLists.txt`（foundation ALIAS lib_base）

**设计决策**:
- 将四个头文件原样复制到 `src/lib/base/foundation/`（保持 `#include "foundation/..."` 路径不变）
- `lib_base` 为 INTERFACE 目标，include dir 设为 `${CMAKE_CURRENT_SOURCE_DIR}`（即 `src/lib/base/`），使 `#include "foundation/..."` 解析到新位置
- 添加 `lib::base` ALIAS（为未来显式使用 lib 命名空间的代码准备）
- `foundation` 目标改为 `ALIAS lib_base`，旧链接代码（geometry、model、engine、tests 等）无需修改
- 将 `add_subdirectory(lib)` 与 `add_subdirectory(apps)` 移到 `add_subdirectory(foundation)` 之前，确保 `lib_base` 在 `foundation` 别名创建前已定义
- 旧 `src/foundation/` 目录的头文件原样保留（过渡期双份存在，`placeholder.cpp` 不再被编译但不删除）

**已知限制**:
- `src/foundation/*.h` 与 `src/lib/base/foundation/*.h` 目前双份存在，T75 清理旧目录兼容层时一并删除

### T59 — 迁移 geometry 到 src/lib/platform/occt (2026-04-05)

**产出文件**: `src/lib/platform/occt/geometry/`（全部 .h/.cpp）· `src/lib/platform/occt/CMakeLists.txt` · `src/geometry/CMakeLists.txt`

**接口**: → `src/lib/platform/occt/CMakeLists.txt`（lib_platform_occt STATIC + lib::platform::occt ALIAS）；`src/geometry/CMakeLists.txt`（geometry ALIAS lib_platform_occt）

**设计决策**:
- 将 `src/geometry/` 所有 .h/.cpp 原样复制到 `src/lib/platform/occt/geometry/` 子目录，保持 `#include "geometry/..."` 路径兼容
- `lib_platform_occt` 为 STATIC 目标，include dir 包含 `${CMAKE_CURRENT_SOURCE_DIR}`（即 `src/lib/platform/occt/`）和 `${CMAKE_SOURCE_DIR}/src`
- 链接 `lib::base`（由 lib_base 传递 include 路径），OCCT 库链接为 PRIVATE
- `geometry` 目标改为 `ALIAS lib_platform_occt`，旧链接代码（model/engine/visualization 等）无需修改
- `src/lib/platform/CMakeLists.txt` 调整子目录顺序：occt → vtk → vsg（vtk 算法层不依赖 Qt，vsg 依赖 vtk_visualization）

**已知限制**:
- `src/geometry/*.{h,cpp}` 与 `src/lib/platform/occt/geometry/*.{h,cpp}` 双份存在，T75 清理时删除旧目录

### T60 — 拆分 visualization 与 vtk-visualization (2026-04-05)

**产出文件**: `src/lib/platform/vsg/visualization/`（全部 .h/.cpp）· `src/lib/platform/vsg/CMakeLists.txt` · `src/lib/platform/vtk/vtk-visualization/`（OcctToVtk/BeamMeshBuilder/VtkSceneManager）· `src/lib/platform/vtk/CMakeLists.txt` · `src/visualization/CMakeLists.txt` · `src/vtk-visualization/CMakeLists.txt`

**接口**: → `lib_platform_vsg` STATIC（ALIAS visualization）；`lib_platform_vtk` STATIC（纯算法无Qt）；`vtk_visualization` STATIC（VtkViewport，链接 lib_platform_vtk + Qt）

**设计决策**:
- 将 `src/visualization/` 全部文件复制到 `src/lib/platform/vsg/visualization/`
- `lib_platform_vsg` 继承原 visualization 的全部依赖（engine、lib::platform::occt、vsg::vsg、vtk_visualization）
- `src/visualization/CMakeLists.txt` 改为 `ALIAS lib_platform_vsg`
- vtk 分拆为两层：纯算法层（OcctToVtk/BeamMeshBuilder/VtkSceneManager）→ `lib_platform_vtk`（无Qt）；Qt集成层（VtkViewport）→ `vtk_visualization`（STATIC，链接 lib_platform_vtk + Qt）
- `vtk_visualization` 源文件缩减为仅 `VtkViewport.cpp`，通过 `lib_platform_vtk` 间接获得 VTK 依赖

**已知限制**:
- 旧 `src/visualization/*.{h,cpp}` 和 `src/vtk-visualization/OcctToVtk.* BeamMeshBuilder.* VtkSceneManager.*` 双份存在，T75 清理

### T61 — 迁移 app 与 command 到 lib (2026-04-05)

**产出文件**: `src/lib/runtime/command/`（全部 command .h/.cpp）· `src/lib/runtime/app/`（Document/DependencyGraph/SelectionManager）· `src/lib/runtime/CMakeLists.txt` · `src/lib/framework/app/`（Application/WorkbenchManager/Workbenches/ProjectSerializer/StepExporter）· `src/lib/framework/CMakeLists.txt` · `src/app/CMakeLists.txt` · `src/command/CMakeLists.txt`（保持不变）

**接口**: → `lib_runtime` STATIC（ALIAS lib::runtime）；`lib_framework` STATIC（ALIAS lib::framework）；`app` ALIAS lib_framework

**设计决策**:
- `lib_runtime`：包含 Document/DependencyGraph/SelectionManager（app 域）+ 全部 command/*.cpp，链接 engine + nlohmann_json
- `lib_framework`：包含 Application/WorkbenchManager/全部 Workbench/ProjectSerializer/StepExporter，链接 lib_runtime + visualization + OCCT AFX (PRIVATE)
- `app` 目标改为 `ALIAS lib_framework`（命令层仍通过 `command` INTERFACE→app→lib_framework 传递，链接关系无需重写）
- command 源文件从 `app/CMakeLists.txt` 中的绝对路径引用改为在 `lib_runtime` 的相对路径引用（来自复制后的 command/ 子目录）
- `src/app/CMakeLists.txt` 改为单行别名 `add_library(app ALIAS lib_framework)`

**已知限制**:
- 旧 `src/app/*.{h,cpp}` 和 `src/command/*.{h,cpp}` 双份存在，T75 清理
- engine 仍与 Document/DependencyGraph 有头文件级耦合（RecomputeEngine），T71 解耦

### T63 — 为 lib/base 建立第一批模块接口单元 (2026-04-05)

**产出文件**: `src/lib/base/baseMod/pipecad.base.math.cppm` · `src/lib/base/baseMod/pipecad.base.types.cppm` · `src/lib/base/baseMod/pipecad.base.cppm` · `src/lib/base/CMakeLists.txt`（新增 lib_base_modules STATIC + lib::base::modules ALIAS）

**接口**: → `import pipecad.base;`（聚合）、`import pipecad.base.math;`、`import pipecad.base.types;`

**设计决策**:
- 使用 C++20 模块，需 CMake `FILE_SET CXX_MODULES`（≥3.28）和 GCC 15/C++20 标准
- 每个 .cppm 文件使用 `module;` 全局模块片段 + `#include` 遗留头文件，再在命名模块中 export
- 使用 `pipecad::` 包装命名空间（而非直接 `foundation::`），避免与全局模块片段隐式暴露的符号发生重声明冲突
- `pipecad.base.cppm` 使用 `export import` 聚合子模块，使用者可一次性 `import pipecad.base;` 获得全部基础类型
- `lib_base` INTERFACE 通过 `target_link_libraries(lib_base INTERFACE lib_base_modules)` 将模块传递给所有链接者

### T67 — 定义 DocumentSnapshot 契约 (2026-04-05)

**产出文件**: `src/lib/runtime/app/DocumentSnapshot.h` · `src/lib/runtime/app/DocumentSnapshot.cpp` · `src/lib/runtime/app/Document.h` · `src/lib/runtime/app/Document.cpp` · `src/lib/runtime/app/DependencyGraph.h` · `src/lib/runtime/app/DependencyGraph.cpp` · `tests/test_runtime_tasking.cpp`

**接口**: → `src/lib/runtime/app/DocumentSnapshot.h`, `src/lib/runtime/app/Document.h`, `src/lib/runtime/app/DependencyGraph.h`

**设计决策**:
- 将文档版本令牌收口到 `Document`，通过对象 `changed` 信号、文档名变更和增删对象统一推进 `currentVersion()`
- 以值语义冻结 `PipeSpec`、`PipePoint`、`Segment`、`Route` 和依赖图双向边视图，避免后台线程持有可变文档引用
- `DocumentSnapshot` 由主线程同步构建，版本号、拓扑顺序和脏集合均在提交任务前固化
- 为过渡期兼容 include 路径，同步扩展旧 `src/app/Document.h` 与 `src/app/DependencyGraph.h` 声明，避免旧头遮蔽新 API

**已知限制**:
- 当前快照仍聚焦管道几何主链路，附件、载荷和场景节点尚未进入后台快照面

### T68 — 建立任务队列与线程工作组 (2026-04-05)

**产出文件**: `src/lib/runtime/task/TaskQueue.h` · `src/lib/runtime/task/TaskQueue.cpp` · `src/lib/runtime/CMakeLists.txt` · `src/lib/runtime/runtimeMod/pipecad.runtime.task.cppm` · `tests/test_runtime_tasking.cpp`

**接口**: → `src/lib/runtime/task/TaskQueue.h`, `src/lib/runtime/runtimeMod/pipecad.runtime.task.cppm`

**设计决策**:
- 在 runtime 层引入 `TaskQueue`、`TaskHandle`、`CancellationToken` 与 `WorkerGroup`，统一提供提交、等待、取消和退出回收能力
- 任务执行采用协作式取消：运行中任务通过 `CancellationToken` 观察取消请求，待执行任务则由队列直接标记完成并丢弃
- `WorkerGroup::shutdown(true)` 同时取消活动任务与排队任务，并等待线程安全回收，满足 T68 的退出语义
- 在 `pipecad.runtime.task` 模块边界中仅导出快照与调度类型前向边界，保持模块接口轻量

**已知限制**:
- 当前任务调度尚未包含主线程结果回投与过期结果丢弃，这部分留给 T69 完成

**已知限制**:
- 导出符号位于 `pipecad::` 命名空间，与旧 `foundation::` 命名空间并存；T75 后可考虑统一命名空间
- 当前项目编译仍用 C++17，模块仅由 `lib_base_modules` 单独以 C++20 编译，其他文件不使用 import 语句

### T62 — 迁移 model/engine/ui/main 到 apps (2026-04-05)

**产出文件**: `src/apps/pipecad/model/*` · `src/apps/pipecad/engine/*` · `src/apps/pipecad/ui/*` · `src/apps/pipecad/main.cpp` · `src/apps/pipecad/CMakeLists.txt` · `src/apps/pipecad/model/CMakeLists.txt` · `src/apps/pipecad/engine/CMakeLists.txt` · `src/apps/pipecad/ui/CMakeLists.txt` · `src/model/CMakeLists.txt` · `src/engine/CMakeLists.txt` · `src/ui/CMakeLists.txt` · `src/CMakeLists.txt`

**接口**: → `src/apps/pipecad/CMakeLists.txt`, `src/apps/pipecad/main.cpp`, `src/apps/pipecad/model/CMakeLists.txt`, `src/apps/pipecad/engine/CMakeLists.txt`, `src/apps/pipecad/ui/CMakeLists.txt`

**设计决策**:
- 将 `src/model/`、`src/engine/`、`src/ui/` 全量复制到 `src/apps/pipecad/`，保持 `#include "model/..."`、`#include "engine/..."`、`#include "ui/..."` 路径不变
- 将入口文件迁移到 `src/apps/pipecad/main.cpp`，可执行目标 `pipecad` 改由 apps 子树定义
- 新建 `pipecad_app_model`、`pipecad_app_engine`、`pipecad_app_ui` 三个 STATIC 目标，并提供 `lib::apps::pipecad::*` ALIAS
- 旧 `model`、`engine`、`ui` 目标全部改为兼容 ALIAS，现有测试和遗留链接无需改名
- 顶层 `src/CMakeLists.txt` 删除旧 `pipecad`/`pipecad_app` 定义，统一交由 `src/apps/pipecad/` 承担当前 app 闭包

**已知限制**:
- `src/model/`、`src/engine/`、`src/ui/` 旧目录物理文件仍保留，当前仅作为兼容入口，T75 再清理

### T64 — 为 lib/platform 建立 facade 模块 (2026-04-05)

**产出文件**: `src/lib/platform/occt/pipecad.platform.occt.cppm` · `src/lib/platform/vsg/pipecad.platform.vsg.cppm` · `src/lib/platform/vtk/pipecad.platform.vtk.cppm` · `src/lib/platform/occt/CMakeLists.txt` · `src/lib/platform/vsg/CMakeLists.txt` · `src/lib/platform/vtk/CMakeLists.txt`

**接口**: → `src/lib/platform/occt/pipecad.platform.occt.cppm`, `src/lib/platform/vsg/pipecad.platform.vsg.cppm`, `src/lib/platform/vtk/pipecad.platform.vtk.cppm`

**设计决策**:
- 为 OCCT、VSG、VTK 三个子域分别建立独立的 C++20 facade 模块目标：`lib_platform_occt_modules`、`lib_platform_vsg_modules`、`lib_platform_vtk_modules`
- 模块接口只导出前向声明与稳定别名，不直接 `export` 第三方重头类型，规避 GCC 模块与 OCCT/VSG/VTK 头的兼容问题
- 公开命名空间统一为 `pipecad::platform::{occt,vsg,vtk}`，同时保留原有头文件 include 兼容路径
- 将模块目标挂入对应 platform 静态库依赖，使后续 app/framework 可通过 link 获得模块编译产物

**已知限制**:
- facade 模块当前冻结的是边界而不是完整 API 面；apps 代码仍主要通过公开头访问 platform，真正 import 落地留待后续阶段推进

### T65 — 为 lib/runtime 建立核心模块 (2026-04-05)

**产出文件**: `src/lib/runtime/runtimeMod/pipecad.runtime.document.cppm` · `src/lib/runtime/runtimeMod/pipecad.runtime.graph.cppm` · `src/lib/runtime/runtimeMod/pipecad.runtime.command.cppm` · `src/lib/runtime/runtimeMod/pipecad.runtime.serialize.cppm` · `src/lib/runtime/runtimeMod/pipecad.runtime.task.cppm` · `src/lib/runtime/CMakeLists.txt`

**接口**: → `src/lib/runtime/runtimeMod/pipecad.runtime.document.cppm`, `src/lib/runtime/runtimeMod/pipecad.runtime.graph.cppm`, `src/lib/runtime/runtimeMod/pipecad.runtime.command.cppm`, `src/lib/runtime/runtimeMod/pipecad.runtime.serialize.cppm`, `src/lib/runtime/runtimeMod/pipecad.runtime.task.cppm`

**设计决策**:
- 建立 `document`、`graph`、`command`、`serialize`、`task` 五个运行时模块接口单元，并汇总到 `lib_runtime_modules` STATIC 目标
- `task` 模块在本任务只冻结运行时任务边界，使用 `TaskBoundary` 占位，避免提前引入 T68 的线程调度实现
- 命令系统继续通过公开头工作，模块层只提供稳定类型边界，兼容现有 `CommandContext` 中对 `TopologyManager` 的引用
- `lib_runtime` 主体仍保留现有静态库构建方式，模块目标与传统头文件接口并存

**已知限制**:
- runtime 模块尚未被业务代码直接 `import`；当前阶段目标是固定边界，不做调用点迁移

### T66 — 为 lib/framework 建立框架模块 (2026-04-05)

**产出文件**: `src/lib/framework/frameworkMod/pipecad.framework.application.cppm` · `src/lib/framework/frameworkMod/pipecad.framework.workbench.cppm` · `src/lib/framework/frameworkMod/pipecad.framework.scene.cppm` · `src/lib/framework/frameworkMod/pipecad.framework.cppm` · `src/lib/framework/CMakeLists.txt`

**接口**: → `src/lib/framework/frameworkMod/pipecad.framework.application.cppm`, `src/lib/framework/frameworkMod/pipecad.framework.workbench.cppm`, `src/lib/framework/frameworkMod/pipecad.framework.scene.cppm`, `src/lib/framework/frameworkMod/pipecad.framework.cppm`

**设计决策**:
- 为 application/workbench/scene 建立前向声明式 facade 模块，延续 T64/T65 的边界冻结风格，不在接口单元中暴露 VSG/OCCT 重型实现
- 新增 `lib_framework_modules` STATIC 目标，并由 `lib_framework` 公开链接，使传统头接口与模块接口并存
- 增加聚合模块 `pipecad.framework`，统一 re-export `application`、`workbench`、`scene` 三个子模块
- `scene` 模块以 `SceneManager` 为稳定场景宿主边界，同时提供 `Scene` 别名，便于后续并发任务只依赖 framework 级抽象

**已知限制**:
- framework 模块当前只冻结类型边界，业务代码仍主要通过公开头访问；真正 import 调用点迁移留待后续阶段

### T69 — 建立结果回投与任务版本控制 (2026-04-05)

**产出文件**: `src/lib/runtime/task/ResultChannel.h` · `src/lib/runtime/task/ResultChannel.cpp` · `src/lib/runtime/CMakeLists.txt` · `src/lib/runtime/runtimeMod/pipecad.runtime.task.cppm` · `tests/test_runtime_tasking.cpp`

**接口**: → `src/lib/runtime/task/ResultChannel.h`, `src/lib/runtime/runtimeMod/pipecad.runtime.task.cppm`

**设计决策**:
- 引入 `task::ResultItem`（携带 `submittedVersion` + `applyFn`）和 `task::ResultChannel`（线程安全回投队列）
- 后台线程调用 `ResultChannel::post(submittedVersion, applyFn)` 提交结果；主线程调用 `drainFresh(currentVersion)` 按版本过滤，不匹配则静默丢弃
- 额外提供 `drainAll()`（强制刷新）和 `discard()`（丢弃全部），满足不同使用场景
- `pipecad.runtime.task` 模块边界新增 `ResultItem` 和 `ResultChannel` 的前向声明导出

**已知限制**:
- `ResultChannel` 目前为通用回调通道，具体回投到哪个场景对象由调用方（T71 中的 RecomputeEngine）决定，本任务不约束

### T70 — 为共享状态补齐同步策略 (2026-04-05)

**产出文件**: `src/lib/runtime/task/SceneUpdateAdapter.h` · `src/lib/runtime/task/SceneUpdateAdapter.cpp` · `src/lib/runtime/CMakeLists.txt` · `src/lib/runtime/runtimeMod/pipecad.runtime.task.cppm` · `src/lib/runtime/app/DependencyGraph.h`（注释增强）· `src/lib/runtime/app/DocumentSnapshot.h`（注释增强）· `src/apps/pipecad/engine/RecomputeEngine.h`（注释增强）· `tests/test_runtime_tasking.cpp`（新增 6 个测试）· `docs/tasks/phase4-lib-app-refactor/t70-sync-policy.md`

**接口**: → `src/lib/runtime/task/SceneUpdateAdapter.h`

**设计决策**:
- 实现 `task::SceneUpdateAdapter`，封装主线程消费 ResultChannel 的协议（drain/drainAll/discard/pendingCount）
- 采用 VersionProvider 函数对象而非直接持有 Document 引用，提高可测试性并避免不必要的头文件依赖
- 在 DependencyGraph.h dirty 方法注释中冻结"主线程独占"规则，并明确 collectDirty/clearDirty 必须在同一主线程帧内原子执行
- 在 DocumentSnapshot.h 的 makeDocumentSnapshot 注释中冻结"快照窗口"协议：collectDirty → makeDocumentSnapshot → clearDirty → submit task 的顺序约束
- 在 RecomputeEngine.h 注释中说明主线程独占原因（OCCT 非线程安全）和 T71 计划的异步化路径
- `pipecad.runtime.task` 模块边界新增 `SceneUpdateAdapter` 前向声明导出
- 在 tests/test_runtime_tasking.cpp 新增 6 个 SceneUpdateAdapter 单测：版本匹配执行、版本延迟查询（versionProvider 调用时机验证）、drainAll 跳过版本校验、discard 不执行 applyFn、WorkerGroup 联动、pendingCount 状态追踪

**已知限制**:
- SceneUpdateAdapter 目前未接入实际的 RecomputeEngine 异步调用路径，这部分留给 T71 完成
- 当前系统仍为主线程串行执行，本任务的同步策略注释在 T71 真正引入后台线程时才发挥约束作用

### T71 — 重构 RecomputeEngine 异步管线 (2026-04-05)

**产出文件**: `src/apps/pipecad/engine/GeometryDeriver.h` · `src/apps/pipecad/engine/GeometryDeriver.cpp` · `src/apps/pipecad/engine/RecomputeEngine.h` · `src/apps/pipecad/engine/RecomputeEngine.cpp` · `src/engine/GeometryDeriver.h`（shim 更新）· `src/engine/RecomputeEngine.h`（shim 更新）· `src/apps/pipecad/main.cpp` · `tests/test_async_recompute.cpp` · `tests/CMakeLists.txt`

**接口**: → `src/apps/pipecad/engine/RecomputeEngine.h`, `src/apps/pipecad/engine/GeometryDeriver.h`

**设计决策**:
- 采用类型擦除 `AsyncFn = std::function<void()>` + `DrainFn = std::function<std::size_t()>` 二元组注入，避免 `pipecad_app_engine` 对 `lib_runtime` 产生 CMake 循环依赖
- `asyncRecompute()` 只检查脏集非空后调用 `asyncFn_()`，不直接调用 `makeDocumentSnapshot`；快照构建、clearDirty、后台提交、结果回投全部封装在注入的 `asyncFn_` 中（来自 main.cpp）
- `GeometryDeriver::deriveFromSnapshot()` 新增静态方法，接受只读 `PipePointSnapshot/PipeSpecSnapshot`，后台线程安全；内部访问 `fields["OD"]`、`fields["wallThickness"]` 等字段替代 `spec->od()` 活动对象方法
- main.cpp 中的 `asyncFn` 完整封装 T70 快照窗口协议：makeDocumentSnapshot → clearDirty → workers.submit → [后台] deriveFromSnapshot → resultChannel.post → [主线程 drain] sceneCb
- 头文件包含顺序問題：`main.cpp` 位于 `src/apps/pipecad/`，相对路径包含 `"engine/GeometryDeriver.h"` 会解析到 `src/apps/pipecad/engine/GeometryDeriver.h`（而非 shim），导致重定义；解决方案是不重复 include，依赖 RecomputeEngine.h → shim GeometryDeriver.h 的传递包含

**已知限制**:
- GeometryDeriver shim (`src/engine/GeometryDeriver.h`) 与 canonical header 必须手动同步接口，直到 T75 清理兼容层
- 目前 asyncFn 在 pipecad 可执行里定义于 main.cpp，若未来需要在别处复用需提取单独构件
