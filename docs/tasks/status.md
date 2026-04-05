# 任务状态跟踪

> **⚠️ 此文件由 AI Agent 自动维护，人类也可手动编辑。**  
> **AI 读取此文件确定下一个任务、获取前置上下文。**

---

## 状态说明

| 状态 | 含义 |
|------|------|
| `pending` | 未开始 |
| `ready` | 前置依赖已完成，可以开始 |
| `in-progress` | 正在进行 |
| `done` | 已完成并验证通过 |
| `blocked` | 被阻塞（记录原因） |

---

## 任务状态

| ID | 任务名 | 状态 | 依赖 | 推荐模型 | 完成日期 |
|----|--------|------|------|---------|---------|
| T01 | 构建系统搭建 | `done` | — | Sonnet | 2026-03-28 |
| T02 | Foundation 层 | `done` | T01 | Sonnet | 2026-03-28 |
| T03 | OCCT 几何封装 | `done` | T01 | Sonnet | 2026-03-28 |
| T04 | OCCT 网格化 + STEP I/O | `done` | T01 | Sonnet | 2026-03-28 |
| T05 | 核心文档对象 | `done` | T02 | **Opus** | 2026-03-28 |
| T06 | 附属对象与梁 | `done` | T05 | Sonnet | 2026-03-28 |
| T07 | 弯头几何计算器 | `done` | T05, T02 | **Opus** | 2026-03-28 |
| T08 | 管件几何 (Run/Reducer/Tee) | `done` | T07, T03 | Sonnet | 2026-03-28 |
| T09 | 管件几何 (Valve/Flex/Beam) | `done` | T03, T05 | Sonnet | 2026-03-28 |
| T10 | 拓扑管理与约束 | `done` | T05 | Sonnet | 2026-03-28 |
| T11 | OCCT→VSG 网格转换 | `done` | T04 | Sonnet | 2026-03-28 |
| T12 | VSG 场景管理 | `done` | T11 | Sonnet | 2026-03-28 |
| T13 | 相机控制与场景基础设施 | `done` | T12 | Sonnet | 2026-03-28 |
| T14 | 3D 拾取与高亮 | `done` | T12 | Sonnet | 2026-03-28 |
| T15 | VSG-QML 桥接 | `done` | T12, T13 | **Opus** | 2026-03-28 |
| T16 | 应用层核心 | `done` | T05, T07 | **Opus** | 2026-03-28 |
| T17 | 工作台 + QML 桥接 | `done` | T16 | Sonnet | 2026-03-28 |
| T18 | QML 表格模型层 | `done` | T17 | Sonnet | 2026-03-28 |
| T19 | QML UI 面板 | `done` | T18 | Sonnet | 2026-03-28 |
| T20 | JSON 序列化 | `done` | T05, T06 | Sonnet | 2026-03-28 |
| T21 | STEP 导出 | `done` | T08, T09, T04 | Sonnet | 2026-03-28 |
| T25 | 集成测试 | `done` | 全部 | **Opus** | 2026-03-28 |

### 二期任务 (Phase 2)

| ID | 任务名 | 状态 | 依赖 | 推荐模型 | 完成日期 |
|----|--------|------|------|---------|---------|
| T30 | ViewManager 视图管理器 | `done` | — | Sonnet | 2026-03-29 |
| T31 | ComponentCatalog 参数化构件模板 | `done` | — | **Opus** | 2026-03-29 |
| T32 | Load 载荷数据模型 | `done` | — | Sonnet | 2026-03-29 |
| T33 | LoadCase 与 LoadCombination | `done` | T32 | Sonnet | 2026-03-29 |
| T34 | DesignWorkbench 工作台 | `done` | T31 | Sonnet | 2026-03-29 |
| T35 | SpecWorkbench 工作台 | `done` | T31 | Sonnet | 2026-03-29 |
| T36 | DesignTree + ParameterPanel 重构 | `done` | T34 | **Codex** | 2026-03-29 |
| T37 | OCCT→VTK 网格转换 | `done` | T32 | **Codex** | 2026-03-29 |
| T38 | VTK 场景管理 | `done` | T37 | **Codex** | 2026-03-29 |
| T39 | 工作台切换 + QML 面板动态加载 | `done` | T34, T35 | **Gemini** | 2026-03-29 |
| T40 | StatusBar + 右键菜单 + 框选 | `done` | T36 | Sonnet | 2026-03-29 |
| T41 | ComponentToolStrip 元件插入 | `done` | T31, T36 | Sonnet | 2026-03-29 |
| T42 | VTK-QML 桥接 | `done` | T38 | **Gemini** | 2026-03-29 |
| T43 | 序列化扩展 (Load/LoadCase) | `done` | T33 | **Codex** | 2026-03-29 |
| T44 | AnalysisWorkbench 工作台 | `done` | T33, T39, T42 | **Opus** | 2026-03-29 |
| T45 | 端到端集成测试 | `done` | T41, T43, T44 | **Opus** | 2026-03-29 |

### 三期任务 (Phase 3 — 命令模式)

> **设计文档**: `docs/archive/task-specs/command-pattern-design.md` (v3.0, 已归档)

| ID | 任务名 | 状态 | 依赖 | 推荐模型 | 完成日期 |
|----|--------|------|------|---------|---------|
| T0 | Variant 类型扩展 (bool/Vec3) | `done` | — | Sonnet 4.6 | 2026-04-04 |
| T1 | DocumentObject setProperty/getProperty 虚方法 | `done` | T0 | Sonnet 4.6 | 2026-04-04 |
| T2 | Command 基类 + MacroCommand + PropertyApplier | `done` | T1 | Sonnet 4.6 | 2026-04-04 |
| T3 | CommandStack 命令栈管理器 | `done` | T2 | **Opus 4.6** | 2026-04-04 |
| T4 | PropertyCommands (SetProperty/BatchSetProperty) | `done` | T2, T3 | Sonnet 4.6 | 2026-04-04 |
| T5 | CommandRegistry 统一工厂 + 序列化 | `done` | T4 | Sonnet 4.6 | 2026-04-04 |
| T6 | Application 集成 + main.cpp 信号连线 | `done` | T3, T5 | Sonnet 4.6 | 2026-04-04 |
| T7 | UI 原子迁移 (AppController/TableModel) | `done` | T4, T6 | **Opus 4.6** | 2026-04-04 |
| T8 | 结构命令 (CreatePipePoint/DeletePipePoint) | `done` | T3, T5 | **Opus 4.6** | 2026-04-04 |
| T9 | InsertComponentCommand + 完整迁移 | `done` | T7, T8 | **Opus 4.6** | 2026-04-04 |
| T10 | 清理 TransactionManager | `done` | T9 | Sonnet 4.6 | 2026-04-05 |

### 四期任务 (Phase 4 — lib/apps 架构重构)

> **实施计划书**: `docs/lib-app-refactor-plan.md`
>  
> **任务卡目录**: `docs/tasks/phase4-lib-app-refactor/`

| ID | 任务名 | 状态 | 依赖 | 推荐模型 | 完成日期 |
|----|--------|------|------|---------|---------|
| T50 | 冻结目录与目标命名规则 | `done` | — | GPT-5.4 | 2026-04-05 |
| T51 | 冻结 include/import 规则 | `done` | T50 | GPT-5.4 | 2026-04-05 |
| T52 | 冻结线程安全边界 | `done` | T50 | GPT-5.4 | 2026-04-05 |
| T53 | 设计 lib/apps 顶层 CMake 拓扑 | `done` | T50, T51, T52 | GPT-5.4 | 2026-04-05 |
| T54 | 解除 RecomputeEngine 源文件级拼接 | `done` | T53 | GPT-5.4 | 2026-04-05 |
| T55 | 设计过渡兼容层 | `done` | T53 | GPT-5.4 | 2026-04-05 |
| T56 | 建立 src/lib 目录骨架 | `ready` | T53 | GPT-5.4 | — |
| T57 | 建立 src/apps/pipecad 目录骨架 | `ready` | T53 | GPT-5.4 | — |
| T58 | 迁移 foundation 到 src/lib/base | `pending` | T56 | GPT-5.4 | — |
| T59 | 迁移 geometry 到 src/lib/platform/occt | `pending` | T56 | GPT-5.4 | — |
| T60 | 拆分 visualization 与 vtk-visualization | `pending` | T56, T57 | GPT-5.4 | — |
| T61 | 迁移 app 与 command 到 lib | `pending` | T56 | GPT-5.4 | — |
| T62 | 迁移 model/engine/ui/main 到 apps | `pending` | T57, T61 | GPT-5.4 | — |
| T63 | 为 lib/base 建立第一批模块接口单元 | `pending` | T58 | GPT-5.4 | — |
| T64 | 为 lib/platform 建立 facade 模块 | `pending` | T59, T60, T63 | GPT-5.4 | — |
| T65 | 为 lib/runtime 建立核心模块 | `pending` | T61, T63 | GPT-5.4 | — |
| T66 | 为 lib/framework 建立框架模块 | `pending` | T60, T61, T65 | GPT-5.4 | — |
| T67 | 定义 DocumentSnapshot 契约 | `pending` | T61 | GPT-5.4 | — |
| T68 | 建立任务队列与线程工作组 | `pending` | T52, T65 | GPT-5.4 | — |
| T69 | 建立结果回投与任务版本控制 | `pending` | T68 | GPT-5.4 | — |
| T70 | 为共享状态补齐同步策略 | `pending` | T67, T69 | GPT-5.4 | — |
| T71 | 重构 RecomputeEngine 异步管线 | `pending` | T62, T67, T68, T69, T70 | GPT-5.4 | — |
| T72 | 后台化 ShapeMesher 与批量重算 | `pending` | T71 | GPT-5.4 | — |
| T73 | 后台化加载恢复与保存前准备 | `pending` | T68, T69, T71 | GPT-5.4 | — |
| T74 | 建立并发回归测试 | `pending` | T71, T72, T73 | GPT-5.4 | — |
| T75 | 清理旧目录兼容层 | `pending` | T62, T64, T65, T66, T74 | GPT-5.4 | — |
| T76 | 固化 pipecad app 模板 | `pending` | T62, T75 | GPT-5.4 | — |
| T77 | 更新文档与开发规范 | `pending` | T75 | GPT-5.4 | — |

---

## 完成记录索引

> 完成记录已按阶段拆分为独立文件，**按需读取对应文件**，不要全部读取。

| 文件 | 覆盖范围 |
|------|---------|
| `docs/archive/task-logs/t01-t25.md` | Phase 1: T01–T25（构建系统、Foundation、几何、模型、场景、应用层、UI） |
| `docs/archive/task-logs/t30-t45.md` | Phase 2: T30–T45（ViewManager、载荷、工作台、VTK、序列化扩展） |
| `docs/archive/task-logs/command-pattern.md` | Phase 3: T0–T10（命令模式架构实现） |
| `docs/archive/task-logs/phase4-refactor.md` | Phase 4: T50–T77（lib/apps 架构重构） |
