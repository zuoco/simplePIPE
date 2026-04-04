# PipeCAD 项目 — AI Agent 工作指令

> **本文件是 AI Agent 的入口指令。每次新会话，AI 应首先阅读此文件。**

---

## 项目概述

**PipeCAD** (`qml-vsg-occt`) 是一个基于 OCCT (OpenCASCADE 几何内核) + VSG (Vulkan 渲染) + VTK (分析可视化) + Qt6/QML (UI 框架) 构建的海上油气/风电平台管道系统参数化建模与应力分析软件。

**核心设计理念**: 以 **管点(PipePoint)** 为中心的数据模型 —— 管点是带坐标和类型的文档对象，管件几何由管点序列 + 管线特性(PipeSpec) 推导生成。

**项目状态**: Phase 1 (T01-T25) 和 Phase 2 (T30-T45) 已全部完成（45/45）。Phase 3（命令模式 T0-T10）进行中。

---

## 技术栈

| 技术 | 版本 | 来源 | 用途 |
|------|------|------|------|
| OCCT | 8.0.0 | `lib/occt/` | 几何建模内核 (BRep/STEP/Mesh) |
| VSG | 1.1.13 | `lib/vsg/` | Vulkan 3D 渲染引擎 |
| VTK | 9.6.0 | `lib/vtk/` | CAE 分析可视化 (应力/梁单元) |
| Qt6 | ≥6.5 | pixi (conda-forge) | UI 框架 (Quick/QML) |
| nlohmann/json | * | pixi (conda-forge) | JSON 工程文件序列化 |
| GTest | * | pixi (conda-forge) | 单元测试框架 |
| CMake | ≥3.24 | pixi (conda-forge) | 构建系统 |
| Ninja | * | pixi (conda-forge) | 并行构建后端 |
| C++ 标准 | 17 | — | — |

---

## 构建与测试命令

### 首次环境初始化
```bash
bash scripts/setup.sh              # 安装 pixi、依赖、CMake 配置
bash scripts/setup.sh --verify     # 验证环境状态
```

### 构建命令
```bash
# 使用 pixi 任务
pixi run configure-debug           # CMake 配置 (Debug)
pixi run configure-release         # CMake 配置 (Release)
pixi run build-debug               # 编译 Debug
pixi run build-release             # 编译 Release
pixi run test                      # 编译 Debug + 运行全部测试
pixi run clean                     # 清除 build/ 目录

# 或使用构建脚本 wrapper
bash scripts/build.sh              # 默认 Debug 构建
bash scripts/build.sh release      # Release 构建
bash scripts/build.sh test         # 构建 + 运行全部测试
bash scripts/build.sh test -R Engine   # 仅运行 Engine 相关测试
bash scripts/build.sh run          # 构建并运行主程序
bash scripts/build.sh full         # clean + build + test 全量构建
bash scripts/build.sh status       # 查看构建状态
bash scripts/build.sh help         # 显示完整帮助
```

### 运行主程序
```bash
./build/debug/src/pipecad_app
```

---

## 架构分层

项目采用 **8 层架构**，每层编译为 static library，显式依赖:

```
foundation → geometry → model → engine → visualization → app → ui → pipecad_app
               └──→ vtk-visualization ──────────↑
```

| 层 | 目录 | 职责 |
|----|------|------|
| Layer 1: Foundation | `src/foundation/` | 基础类型: UUID、Variant、Math、Signal、Log |
| Layer 2: Geometry | `src/geometry/` | OCCT 封装: ShapeBuilder、BooleanOps、StepIO、ShapeMesher |
| Layer 3: Model | `src/model/` | 文档模型: PipePoint、PipeSpec、Segment、Route、Load 层次结构 |
| Layer 4: Engine | `src/engine/` | 管道领域引擎: ComponentCatalog、BendCalculator、各 Builder、RecomputeEngine |
| Layer 5: Visualization | `src/visualization/` | VSG 渲染: OcctToVsg、SceneManager、PickHandler、ViewManager |
| Layer 5b: VTK Visualization | `src/vtk-visualization/` | VTK 分析视图: OcctToVtk、VtkSceneManager、VtkViewport |
| Layer 6: Application | `src/app/` | 应用层: Document、Workbench 系统、ProjectSerializer、SelectionManager、TransactionManager |
| Layer 7: UI | `src/ui/` | QML 桥接: VsgQuickItem、VtkViewport、Table/Tree Models、AppController |

### 核心概念

- **PipePoint**: 带坐标、类型、PipeSpec 引用的文档对象，管件几何由其序列推导
- **Bend 4 管点模型**: 交点(Axx)、近端N、中点M、远端F —— 均为可选中的 SpatialObject
- **ComponentCatalog**: 参数化构件模板注册表单例，模板定义 `deriveParams()` + `buildShape()`
- **Workbench 系统**: SpecWorkbench(规格管理) / DesignWorkbench(路由设计) / AnalysisWorkbench(应力分析)
- **双渲染引擎**: VSG 用于设计工作台，VTK 用于分析工作台

---

## 代码风格指南

### 命名规范
- **类名**: PascalCase (`PipePoint`, `ComponentCatalog`)
- **函数/方法**: camelCase (`setType()`, `deriveParams()`)
- **成员变量**: 前缀 `m_` (`m_type`, `m_pipeSpec`)
- **私有成员**: 后缀 `_` (`typeParams_`, `accessories_`)
- **常量**: kPascalCase 或全大写下划线
- **枚举**: PascalCase + 成员 PascalCase (`PipePointType::Bend`)

### C++ 规范
- 标准: **C++17** (std::optional, std::variant, structured bindings)
- OCCT 对象: 使用 `Handle<T>` 管理，禁止裸指针指向 Transient 对象
- VSG 对象: 使用 `vsg::ref_ptr<T>` 管理
- 信号/槽: 使用 `foundation::Signal<T>` (轻量级，不依赖 Qt)
- 异常: OCCT 异常用 `Standard_Failure` 捕获，非 std::exception
- 线程安全: **OCCT 非线程安全**，多线程访问需同步
- 数值精度: 几何算法使用 OCCT 常量 `Precision::Confusion()`

### 文件头模板
```cpp
// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once
// 头文件内容
```

---

## 测试策略

### 测试组织
- 测试文件: `tests/test_<name>.cpp`
- 框架: GTest (非 Qt 测试) / Qt6::Test + GTest (Qt 相关测试)
- 配置: `tests/CMakeLists.txt` 定义测试目标并链接对应层

### 运行测试
```bash
# 运行全部测试
pixi run test

# 运行指定测试 (通过名称过滤)
ctest --test-dir build/debug -R <TestName> --output-on-failure

# 直接运行测试可执行文件 (便于 gdb 调试)
./build/debug/tests/test_<name>
```

### 测试覆盖要求
- 每个 Task 必须包含对应单元测试
- 集成测试覆盖跨层联动场景
- Phase 2 结束时有 35/35 测试通过 (100%)

---

## 工作流程

当用户说 **"完成任务 TXX"**、**"继续下一个任务"** 或新会话开始时，按以下步骤执行：

### Step 1: 读取状态文件（必读入口）
```
读取 docs/tasks/current.md
```
- 这是**唯一入口**，包含：当前上下文、下一个任务 ID、推荐模型、需要读取的文件列表
- **严禁自行决定读取文件清单**，必须以此文件为唯一起点

### Step 2: 确认任务前置依赖
```
读取 docs/tasks/status.md 状态表
```
- 找到目标任务，确认其前置依赖均为 `done`
- 如果前置未完成，报告阻塞原因

### Step 3: 读取任务详情
```
读取 docs/command-pattern-design.md 中对应章节（命令模式任务）
或 docs/development-plan.md 中对应章节（其他任务）
```
- 获取: 交付物列表、接口定义、验收标准

### Step 4: 按需读取前置上下文
```
按 current.md「给 AI 的指令」中列出的文件逐一读取
```
- 只读 current.md 明确列出的头文件、源文件
- **直接读取前置任务的 `.h` 头文件**比读日志更准确

### Step 5: 实现代码
- 按交付物列表创建/编辑文件
- C++17 标准，遵循已有代码风格
- 必须包含单元测试

### Step 6: 编译验证
```bash
pixi run build-debug
pixi run test
```
- 确保编译通过、**全部**测试通过

### Step 7: 更新状态（任务完成后立即执行）

**Step 7a**: 更新 `docs/tasks/status.md` 状态表:
1. 将当前任务标记为 `done`，填写完成日期
2. 检查依赖当前任务的后续任务，若所有依赖都 `done`，将其状态从 `pending` 改为 `ready`

**Step 7b**: 追加完成记录到日志文件 `docs/tasks/log/command-pattern.md`

使用以下精简格式（**禁止粘贴 C++ 代码块**）:
```markdown
### TX — 任务名 (YYYY-MM-DD)

**产出文件**: `A.h` · `A.cpp` · `test_a.cpp`

**接口**: → `src/layer/A.h`, `src/layer/B.h`

**设计决策**:
- 决策1
- 决策2

**已知限制**:
- 限制1（如无则写"无"）
```

**Step 7c**: Git 提交
```bash
git add -A
git commit -m "feat: TX — 功能描述

- 功能点1
- 功能点2
- 功能点3"
```
- 提交信息使用**中文**，`feat:` 前缀
- 正文逐条罗列功能点，逻辑清晰

**Step 7d**: **清空并重写** `docs/tasks/current.md`，写入：
1. **当前状态**（对下一个任务有价值的上下文信息）
2. **下一个任务** ID + 名称 + 具体工作描述
3. **推荐模型**（Opus 4.6 / Sonnet 4.6 / Gemini 3.1 Pro / GPT 5.3 Codex）
4. **需要读取的文件列表**（精确到文件路径）

### Step 8: 输出切换指令
向用户报告：
- 完成了什么 + 创建/修改了哪些文件
- 测试是否通过 + Git 提交信息
- **输出下一步指令**，格式：`将模型切换到 XXX，开始任务 TY`

---

### 循环机制

```
┌─────────────────────────────────────────┐
│  新会话开始                              │
│  ↓                                      │
│  读取 docs/tasks/current.md             │
│  ↓                                      │
│  确认任务 + 读取上下文                    │
│  ↓                                      │
│  实现代码 + 编译测试                      │
│  ↓                                      │
│  更新 status.md + 日志 + Git 提交         │
│  ↓                                      │
│  清空并重写 current.md（写入下一个任务）    │
│  ↓                                      │
│  输出：「将模型切换到 XXX，开始任务 TY」    │
│  ↓                                      │
│  用户切换模型 → 新会话 → 回到顶部 ↑       │
└─────────────────────────────────────────┘
```

AI 读取状态文件 → 执行指定任务 → 更新状态 → 输出下一步指令 → 循环往复，直到所有任务完成。

---

## 关键文件索引

| 文件 | 用途 |
|------|------|
| `docs/tasks/current.md` | **状态文件 — 当前上下文 + 下一个任务 + 推荐模型（AI 入口）** |
| `docs/tasks/status.md` | **任务状态表 + 完成记录索引** |
| `docs/command-pattern-design.md` | **命令模式架构设计 v3.0（Phase 3 实现规格）** |
| `docs/architecture.md` | 架构设计（数据模型、分层、UI设计、工作台系统） |
| `docs/development-plan.md` | Phase 1/2 任务详情（交付物、验收标准、依赖关系） |
| `docs/tasks/log/t01-t25.md` | Phase 1 完成记录（T01–T25） |
| `docs/tasks/log/t30-t45.md` | Phase 2 完成记录（T30–T45） |
| `docs/tasks/log/command-pattern.md` | Phase 3 完成记录（命令模式 T0–T10） |
| `lib/vsg/AGENTS.md` | VSG API 使用指南 |

---

## 规则

1. **每次任务必须从 `current.md` 出发**，不得自行决定读取文件清单
2. **每次只做一个任务**，除非用户明确要求并行
3. **不要修改不属于当前任务的已有代码**，除非是修 bug
4. **状态文件 (`status.md`) 是唯一的真相来源**，必须保持更新
5. **完成记录必须足够详细**，让完全无上下文的 AI 也能接续工作（禁止粘贴 C++ 代码块）
6. **遇到阻塞（编译错误、设计冲突）时**，在状态文件中记录 `blocked` + 原因
7. **编译必须通过后才能标 `done`**，否则标 `blocked`
8. **任务完成后必须立即更新状态**：更新 `status.md` 状态表、追加日志文件记录、更新接力文件 `current.md`
9. **遇到无法修复的故障必须立即停止**：出现无法自行修复的编译错误、运行时崩溃、环境问题时，立即停止，标记 `blocked`，向用户报告

---

## 文档语言

本项目主要使用 **中文** 进行注释和文档编写。AI Agent 在生成代码注释、文档字符串、提交信息时应优先使用中文。
