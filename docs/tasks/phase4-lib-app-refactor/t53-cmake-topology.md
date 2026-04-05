# T53 lib/apps 顶层 CMake 拓扑设计（冻结版）

> 任务：T53  
> 日期：2026-04-05  
> 状态：冻结完成（done）  
> 前置：T50、T51、T52

---

## 1. 适用范围

本文档设计 Phase 4 M1（构建重编组）的顶层 CMake 装配方案，作为 T54-T62 实施的前置约束。

**约束对象**：
1. CMake target 命名与拓扑关系
2. `pipecad_lib` / `pipecad_app` / `pipecad` 三大核心 target 的职责边界
3. 过渡兼容层策略（旧 target 名保留规则）
4. 各后续迁移任务对本拓扑的接口约束

**不约束**：
1. 具体源文件搬迁顺序（由 T56-T62 决定）
2. C++20 Modules 接口文件设计（由 T63-T66 决定）
3. 并发快照与任务调度（由 T68-T71 决定）

---

## 2. 现有 CMake 拓扑分析

### 2.1 现有依赖链（8 层）

```
foundation (STATIC)
    ↓
geometry (STATIC) ─────┐
model    (STATIC) ─────┤
    ↓                  │
engine   (STATIC) ─────┤
    ↓                  │
vtk_visualization (STATIC) ──┐
visualization     (STATIC) ──┤
    ↓                        │
app  (STATIC) ←──────────────┘
│   ┌─ 吸收：command/*.cpp
│   ┌─ 吸收：engine/RecomputeEngine.cpp
command (INTERFACE) → app   [向后兼容别名]
    ↓
ui   (STATIC) → app + Qt6
    ↓
pipecad_app (EXE) → ui + Qt6
```

### 2.2 现有层职责归属（规则 T50/T51 视角）

| 旧 target | 归属域 | 未来归属 |
|-----------|--------|---------|
| foundation | lib::base | src/lib/base |
| geometry | lib::platform::occt | src/lib/platform/occt |
| model | apps/pipecad/model | src/apps/pipecad/model |
| engine | apps/pipecad/engine | src/apps/pipecad/engine |
| vtk_visualization | lib::platform::vtk | src/lib/platform/vtk |
| visualization | lib::platform::vsg | src/lib/platform/vsg |
| app | lib::runtime + lib::framework | src/lib/runtime + src/lib/framework |
| command | lib::runtime (内嵌 app) | 与 app 合并 |
| ui | apps/pipecad/ui | src/apps/pipecad/ui |
| pipecad_app (EXE) | apps/pipecad 可执行 | src/apps/pipecad/main.cpp |

> **注意**：`model` 与 `engine` 当前在 lib 层，但按最终架构应归于 apps 域。  
> M1 过渡期先以逻辑 target 表达归属意图，物理迁移由 T56-T62 完成。

### 2.3 存在的问题

1. **命名冲突**：当前 `pipecad_app` 是可执行文件，但未来它应是业务静态库目标。  
   → **解决方案**：T55 引入过渡后，将可执行目标改名为 `pipecad`，并将 `pipecad_app` 释放给业务库使用。

2. **RecomputeEngine 源文件级拼接**：app 吞并了 `engine/RecomputeEngine.cpp`，形成跨层源文件引用。  
   → **解决方案**：T54 解除此拼接，将 RecomputeEngine 还给 engine 层。

3. **model 与 engine 归属模糊**：当前与 lib 并列，但按 T50 规则应属于 apps 域。  
   → **解决方案**：T62 迁移到 `src/apps/pipecad/`，M1 期间保持原位。

---

## 3. M1 目标 CMake 拓扑设计

### 3.1 三大核心目标

| 目标名 | 类型 | 职责 | 对应旧 target |
|--------|------|------|--------------|
| `pipecad_lib` | INTERFACE（过渡期） / STATIC（M2 后） | 统一架构库，聚合所有平台/运行时/框架能力 | app（传递引入全部子库） |
| `pipecad_app` | STATIC（T55 后引入） | pipecad 业务库（model+engine+workbench+ui） | ui（先作为 apps 域入口） |
| `pipecad` | EXE（T55 后引入） | pipecad 可执行入口 | pipecad_app（EXE，当前名） |

### 3.2 M1 过渡期目标关系图

```
# ─── 旧架构子目标（保留，作为 pipecad_lib 的构成组件）──────────────
foundation (STATIC)  ← lib::base
geometry   (STATIC)  ← lib::platform::occt
model      (STATIC)  ← 暂留 lib 域，T62 迁到 apps
engine     (STATIC)  ← 暂留 lib 域，T62 迁到 apps（T54 后 RecomputeEngine 归回 engine）
vtk_visualization (STATIC) ← lib::platform::vtk
visualization     (STATIC) ← lib::platform::vsg
app        (STATIC)  ← lib::runtime + lib::framework（含 command 源码）
command    (INTERFACE) → app   [向后兼容别名，T75 清理]

# ─── T53 新增：pipecad_lib 聚合目标 ─────────────────────────────────
pipecad_lib (INTERFACE)  → app
              # 传递可达：foundation, geometry, model, engine,
              #           vtk_visualization, visualization

# ─── 旧可执行（M1 期间保留，T55 后改名）────────────────────────────
ui          (STATIC)  → pipecad_lib (T55 前: → app)
pipecad_app (EXE)     → ui + Qt6   [T55 后改名为 pipecad，pipecad_app 名释放给业务库]
```

### 3.3 T55 后目标关系图（过渡兼容层完成）

```
# ─── 架构统一库（不含 ui）──────────────────────────────────────────
pipecad_lib (INTERFACE/STATIC) → app
                  # 传递：foundation, geometry, model, engine, visualization, vtk_visualization

# ─── 业务库（初期 = ui 层，T62 后扩展）──────────────────────────────
pipecad_app (STATIC) → pipecad_lib + Qt6
                  # 实际源码：ui/*.cpp

# ─── 可执行（main.cpp 入口）──────────────────────────────────────────
pipecad (EXE) → pipecad_app + Qt6
```

### 3.4 最终目标关系图（M2 物理迁移完成后，T62+）

```
# src/lib → 编译为 pipecad_lib.a
pipecad_lib (STATIC)
├── lib::base        ← src/lib/base/
├── lib::platform    ← src/lib/platform/{occt,vsg,vtk}/
├── lib::runtime     ← src/lib/runtime/
└── lib::framework   ← src/lib/framework/

# src/apps/pipecad → 编译为 pipecad_app.a + pipecad
pipecad_app (STATIC) → pipecad_lib
├── model/           ← src/apps/pipecad/model/
├── engine/          ← src/apps/pipecad/engine/
├── workbench/       ← src/apps/pipecad/workbench/
└── ui/              ← src/apps/pipecad/ui/

pipecad (EXE) → pipecad_app + Qt6
└── main.cpp         ← src/apps/pipecad/main.cpp
```

---

## 4. 过渡兼容层策略

### 4.1 旧 target 保留规则（有效期至 T75）

| 旧 target | 保留形式 | 清理时机 |
|-----------|---------|---------|
| `foundation` | 原 STATIC，成为 pipecad_lib 组成 | T75 |
| `geometry` | 原 STATIC，成为 pipecad_lib 组成 | T75 |
| `model` | 原 STATIC，迁移后成为 pipecad_app 组成 | T62 后替换 |
| `engine` | 原 STATIC，T54 后恢复完整，T62 迁移 | T62 后替换 |
| `vtk_visualization` | 原 STATIC，成为 pipecad_lib 组成 | T75 |
| `visualization` | 原 STATIC，成为 pipecad_lib 组成 | T75 |
| `app` | 原 STATIC，成为 pipecad_lib 核心 | T75 |
| `command` | INTERFACE 别名 → app | T75 清理 |
| `ui` | 原 STATIC，T55 后由 pipecad_app 取代 | T57 后替换 |
| `pipecad_app` (EXE) | T55 后改名为 `pipecad` | T55 |

### 4.2 测试层影响

- 测试文件链接的旧 target（app、engine、foundation 等）**全部保留**，测试无需修改。
- M1 完成后，新测试可选择链接 `pipecad_lib`（最终推荐做法）或继续链接旧 target（过渡允许）。
- T75 清理时，测试统一迁移到链接 `pipecad_lib` 或 `pipecad_app`。

---

## 5. T53 实施步骤（M1 第一步）

### 5.1 修改 `src/CMakeLists.txt`

在 `add_subdirectory(command)` 之后、`add_executable(pipecad_app ...)` 之前添加：

```cmake
# T53: pipecad_lib 聚合架构目标（INTERFACE，M1 过渡版）
# 汇聚全部架构子库，供未来 apps/pipecad 业务库使用
# 当前通过 app 传递引入：foundation, geometry, model, engine,
#   vtk_visualization, visualization, command 源码, RecomputeEngine.cpp
add_library(pipecad_lib INTERFACE)
target_link_libraries(pipecad_lib INTERFACE app)
```

### 5.2 不变项

- 旧 target 名（foundation、geometry、model、engine 等）保持不变
- 测试链接配置不变
- 可执行 `pipecad_app` 暂时不改名（T55 负责）

### 5.3 验收标准

- `pixi run build-debug` 编译通过，新增 target `pipecad_lib` 出现在构建图
- 所有测试（`pixi run test`）通过率与 T52 基线相同（38/41）

---

## 6. 后续任务前置约束

### T54 约束（解除 RecomputeEngine 源文件级拼接）

- 解除 `app/CMakeLists.txt` 中对 `engine/RecomputeEngine.cpp` 的直接引用
- 将 `RecomputeEngine.cpp` 还给 `engine` 静态库，engine 对外声明 `RecomputeEngine` 接口
- app 改为通过 `engine` target 的正常链接访问 `RecomputeEngine`
- 修改后 `pipecad_lib` 传递拓扑不变，只是源文件归属变正确

### T55 约束（过渡兼容层）

- 引入 `pipecad_app` 业务静态库目标（先以 `ui` 为基础）
- 将可执行目标改名为 `pipecad`（target 名 + 输出 binary 名）
- 为旧名 `pipecad_app`（EXE）提供 CMake 消息/警告，指导升级
- 更新 `scripts/build.sh` 中所有 `pipecad_app` 可执行引用为 `pipecad`

### T56 约束（建立 src/lib 骨架）

- 创建 `src/lib/base/`、`src/lib/platform/`、`src/lib/runtime/`、`src/lib/framework/` 目录
- 各子目录创建 `CMakeLists.txt`，先为空 STATIC placeholder
- `src/CMakeLists.txt` 添加 `add_subdirectory(lib)`
- `pipecad_lib` target 从 INTERFACE 升级为真正的 STATIC 聚合（通过 `add_subdirectory(lib)` 来构建）

### T57 约束（建立 src/apps/pipecad 骨架）

- 创建 `src/apps/pipecad/{model,engine,workbench,ui,resources}/` 目录骨架
- `src/apps/CMakeLists.txt` 引导 `add_subdirectory(pipecad)`
- `src/apps/pipecad/CMakeLists.txt` 定义 `pipecad_app` (STATIC) + `pipecad` (EXE)
- 此时 `src/CMakeLists.txt` 中对旧 `pipecad_app` (EXE) 和 `ui` 的定义可注释/删除

### T58-T62 约束（代码迁移）

- 迁移时，保持 pipecad_lib 对外暴露的头文件路径与接口不变
- 迁移后，旧 target 的 `target_include_directories(PUBLIC ${CMAKE_SOURCE_DIR}/src)` 仍需保留或以 install 接口接管
- 每次迁移后必须确保全部测试通过

---

## 7. 关键设计决策

### 7.1 pipecad_lib 在 M1 使用 INTERFACE

**理由**：M1 不移动任何源文件，所有静态库仍独立编译。INTERFACE target 只需转发链接依赖，无需实际编译单元。  
**转变时机**：T56 建立 src/lib 骨架后，pipecad_lib 改为提供实际编译单元的 STATIC 库，届时子库改为 OBJECT 库或模块单元。

### 7.2 通过 app 传递引入所有子库

**理由**：`app` 是现有依赖链的末端，`pipecad_lib INTERFACE → app` 即可传递引入全部 6 个旧子库（foundation, geometry, model, engine, vtk_visualization, visualization）以及 command 源码和 RecomputeEngine。最简化 T53 实施范围。  
**T54 后调整**：T54 解除 RecomputeEngine 拼接后，`app` 不再吞并 RecomputeEngine.cpp，拓扑关系自动变正确，pipecad_lib 无需修改。

### 7.3 ui 暂不合入 pipecad_lib

**理由**：按 T50/T51 规则，`ui` 属于 `apps/pipecad` 域，不属于架构库。M1 期间 ui 仍独立链接 app，T55 后 ui 成为 `pipecad_app` 业务库的一部分。  
**约束**：新增测试或功能，禁止将 `ui` 合入 `pipecad_lib`。
