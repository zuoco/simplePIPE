# 工业软件 Claude Code Skills 使用手册

> 面向工业软件开发者的 AI 辅助编码指南
> 覆盖 CAD / CAM / CAE / EDA / 仿真 / 可视化全技术域

---

## 目录

- [1. 概述](#1-概述)
- [2. 安装与配置](#2-安装与配置)
  - [2.1 前置条件](#21-前置条件)
  - [2.2 快速安装](#22-快速安装)
  - [2.3 应用到你的项目](#23-应用到你的项目)
  - [2.4 文件结构说明](#24-文件结构说明)
- [3. 命令详解与示例](#3-命令详解与示例)
  - [3.1 /industrial-dev — 开源库集成与代码开发](#31-industrial-dev--开源库集成与代码开发)
  - [3.2 /source-analysis — 源码架构分析](#32-source-analysis--源码架构分析)
  - [3.3 /architecture-design — 架构设计与技术选型](#33-architecture-design--架构设计与技术选型)
  - [3.4 /cae-simulation — CAE/仿真开发](#34-cae-simulation--cae仿真开发)
  - [3.5 /eda-dev — EDA/PCB 开发](#35-eda-dev--edapcb-开发)
- [4. 直接提问模式](#4-直接提问模式)
- [5. 非交互模式（脚本与批处理）](#5-非交互模式脚本与批处理)
- [6. 参考文档体系](#6-参考文档体系)
- [7. 命令速查表](#7-命令速查表)
- [8. 场景实战](#8-场景实战)
  - [8.1 从零搭建 CAD 平台](#81-从零搭建-cad-平台)
  - [8.2 集成 OCCT + OSG 渲染管线](#82-集成-occt--osg-渲染管线)
  - [8.3 开发 CAE 后处理系统](#83-开发-cae-后处理系统)
  - [8.4 开发 PCB 自动布线工具](#84-开发-pcb-自动布线工具)
  - [8.5 工业设备监控系统](#85-工业设备监控系统)
- [9. 高级技巧](#9-高级技巧)
- [10. 常见问题](#10-常见问题)

---

## 1. 概述

### 什么是 Claude Code Skills

Claude Code Skills 是一套基于 **Claude Code**（Anthropic 官方命令行 AI 编程工具）的扩展命令系统，通过自定义 Slash 命令为 AI 注入工业软件领域的专业知识。

### 能力全景

| 技术域 | 覆盖内容 | 核心开源库 |
|--------|----------|-----------|
| CAD 图形平台 | 参数化建模、插件架构、2D/3D CAD | FreeCAD、OpenSCAD、QCAD、LibreCAD |
| 几何造型器（内核） | BRep 建模、NURBS、计算几何 | OpenCASCADE、AMCAX、CGAL、openNURBS |
| 显示渲染器 | 场景图、科学可视化、体渲染 | OpenSceneGraph、VTK、Mayo |
| CAE / 仿真 | CFD、有限元、网格生成 | OpenFOAM、deal.II、Gmsh、FastCAE |
| 几何约束求解器 | 2D/3D 草图约束、参数化 | SolveSpace、PlaneGCS |
| EDA | PCB 原理图、布局布线 | LibrePCB |
| UI 界面 | Ribbon 控件 | SARibbon、QtRibbonGUI |
| 空间索引 | R-tree、KNN | libspatialindex |
| 数据格式 | BIM/IFC | IFC++ |
| 字体引擎 | 3D 文字渲染 | FreeType |
| 工业协议 | Modbus、OPC UA、MQTT、EtherCAT | — |
| 设备互联 | 工业网关、边缘计算 | — |
| 安全合规 | IEC 62443、等保 2.0 | — |

### 工作原理

```
用户输入命令               命令模板（.claude/commands/xxx.md）
    │                            │
    └──────── Claude Code ───────┘
                 │
                 ├── 1. 加载 CLAUDE.md（编码规范 + 项目上下文）
                 ├── 2. 解析命令中的执行流程
                 ├── 3. 按需读取 references/ 参考文档
                 └── 4. 按流程输出结果
```

---

## 2. 安装与配置

### 2.1 前置条件

- **Claude Code** 已安装（安装方式：`npm install -g @anthropic-ai/claude-code`）
- **Claude 账户** 已登录

### 2.2 快速安装

**方式一：克隆本仓库**

```bash
git clone https://github.com/haiziyan/industrial-software-dev-skill.git
cd industrial-software-dev-skill
claude
```

启动后 `CLAUDE.md` 自动加载，5 个 Slash 命令立即可用。

**方式二：在已有项目中集成**

```bash
# 进入你的工业软件项目目录
cd /your-industrial-project

# 复制 Skills 文件
cp -r /path/to/industrial-software-dev-skill/.claude .
cp /path/to/industrial-software-dev-skill/CLAUDE.md .

# 确保 references/ 目录也被复制（命令会按需读取）
cp -r /path/to/industrial-software-dev-skill/references .

# 启动 Claude Code
claude
```

### 2.3 应用到你的项目

根据你的项目类型选择性地复制：

```bash
# 如果你只做 CAD/几何内核开发（只需主命令）
mkdir -p .claude/commands
cp industrial-software-dev-skill/.claude/commands/industrial-dev.md .claude/commands/
cp industrial-software-dev-skill/CLAUDE.md .
cp -r industrial-software-dev-skill/references .

# 如果你做全栈工业软件开发（复制所有命令）
cp -r industrial-software-dev-skill/.claue .
cp industrial-software-dev-skill/CLAUDE.md .
cp -r industrial-software-dev-skill/references .
```

### 2.4 文件结构说明

```
your-project/
├── CLAUDE.md                          # 项目上下文（自动加载）
│   ├── 编码规范（OCCT Handle / OSG ref_ptr / C++17）
│   ├── 架构规范（三层分离 / 插件化）
│   ├── 性能规范（BVH / LOD / Pipeline）
│   └── 安全规范（异常处理 / 线程安全）
│
├── .claude/
│   └── commands/                      # Slash 命令目录
│       ├── industrial-dev.md          # /industrial-dev  库集成与代码开发
│       ├── source-analysis.md         # /source-analysis 源码架构分析
│       ├── architecture-design.md     # /architecture-design 架构设计
│       ├── cae-simulation.md          # /cae-simulation  CAE/仿真
│       └── eda-dev.md                 # /eda-dev         EDA/PCB
│
└── references/                        # 参考文档库（约 5,700 行）
    ├── open-source-libraries.md       # 20+ 工业开源库 API 示例
    ├── architecture-patterns.md       # 架构模式
    ├── development-guide.md           # 开发环境配置
    ├── industrial-protocols.md        # 工业协议
    ├── device-connectivity.md         # 设备互联
    ├── security-compliance.md         # 安全合规
    └── market-analysis.md             # 市场分析
```

---

## 3. 命令详解与示例

### 3.1 /industrial-dev — 开源库集成与代码开发

**用途**：当你需要集成某个工业开源库、编写基于 OCCT/OSG/VTK 等的代码时使用。

**执行流程**：
```
理解需求 → 按需读取参考文档进行库选型 → 输出完整代码与集成方案
```

**典型用法**：

```
# OCCT 几何建模
/industrial-dev 用 OCCT 7.8 实现圆柱体与长方体的布尔差运算，输出完整 C++ 代码和 CMakeLists.txt

# OSG 场景渲染
/industrial-dev 实现 OCCT TopoDS_Shape 到 osg::Geometry 的网格转换器，包括法线计算和材质配置

# VTK 可视化
/industrial-dev 用 VTK 加载 STL 文件并实现鼠标拾取功能，集成到 Qt 窗口

# FreeCAD 插件开发
/industrial-dev 为 FreeCAD 开发一个自定义导出插件，将模型导出为自定义格式

# 多库联合集成
/industrial-dev 在 Qt 应用中同时集成 OCCT 几何内核和 OSG 渲染引擎，给出集成架构和代码框架

# Python 绑定
/industrial-dev 用 pybind11 为 OCCT 的 BRepAlgoAPI 布尔运算封装 Python 接口

# 国产内核集成
/industrial-dev 对比 AMCAX 九韶内核与 OCCT 的 API 差异，给出迁移方案
```

**输出内容**：
- 技术选型说明（为什么选这个库）
- 完整可编译的 C++ / Python 代码
- CMakeLists.txt 构建配置
- 集成注意事项（内存管理、线程安全、版本兼容）
- 性能优化建议

---

### 3.2 /source-analysis — 源码架构分析

**用途**：当你需要深入理解某个开源工业软件的架构设计、阅读源码、提炼可复用模式时使用。

**执行流程**：
```
读取参考文档 → 解析目录结构 → 追踪核心 API 链路 → 提炼设计模式 → 输出分析报告
```

**典型用法**：

```
# FreeCAD 插件机制分析
/source-analysis 分析 FreeCAD 的 Workbench 插件机制，包括插件注册、发现、加载的完整流程

# OCCT 核心架构分析
/source-analysis 分析 OpenCASCADE 的 BRep 拓扑数据结构，解释 TopoDS_Shape / TopoDS_Solid / TopoDS_Face 的关系

# OSG 渲染管线分析
/source-analysis 分析 OpenSceneGraph 的 NodeVisitor 遍历机制和 StateSet 状态管理

# VTK Pipeline 分析
/source-analysis 分析 VTK 的 Pipeline 数据流架构，解释 Algorithm 和 Filter 的执行机制

# SolveSpace 求解器分析
/source-analysis 分析 SolveSpace 的几何约束求解核心算法，解释约束图构建和求解过程

# 设计模式提炼
/source-analysis 从 FreeCAD 源码中提炼可复用的设计模式，特别是插件架构和文档模型
```

**输出内容**：
- 架构概览（模块划分与依赖关系）
- 核心类职责说明
- 设计模式总结
- API 使用指南
- 可借鉴要点

---

### 3.3 /architecture-design — 架构设计与技术选型

**用途**：当你需要设计系统架构、做技术选型决策、评估多个候选方案时使用。

**执行流程**：
```
读取架构参考文档 → 需求分析 → 架构模式选择 → 输出设计文档
```

**典型用法**：

```
# CAD 平台架构
/architecture-design 为工业 CAD 平台设计插件化架构，参考 FreeCAD Workbench 机制

# CAE 系统架构
/architecture-design 为有限元仿真系统设计分层架构：前处理、求解器、后处理

# 工业物联网平台
/architecture-design 设计一个工业物联网数据采集平台，支持 Modbus 和 OPC UA 协议

# 微服务架构
/architecture-design 将单体工业软件重构为微服务架构，分析利弊和迁移策略

# 技术选型
/architecture-design OCCT vs CGAL vs Open3D 用于几何建模的对比选型，考虑性能、License、API 易用性

# 安全架构
/architecture-design 为工业数据平台设计安全架构，满足 IEC 62443 和等保 2.0 要求
```

**输出内容**：
- 架构决策记录（ADR）
- 模块划分与接口定义
- 技术选型对比表
- 关键设计决策（数据一致性、通信协议、容错）
- 风险评估

---

### 3.4 /cae-simulation — CAE/仿真开发

**用途**：当你需要开发仿真相关功能，包括网格划分、有限元求解、CFD、后处理可视化时使用。

**执行流程**：
```
读取 CAE 参考文档 → 需求分类与方案设计 → 输出仿真流程与代码
```

**典型用法**：

```
# 网格生成
/cae-simulation 用 Gmsh API 对 STEP 文件进行四面体网格划分，控制网格尺寸为 2mm

# 有限元求解
/cae-simulation 用 deal.II 实现二维平面应力问题的有限元求解器

# CFD 仿真
/cae-simulation 用 OpenFOAM 实现管道内流的稳态仿真，给出案例配置

# 后处理可视化
/cae-simulation 用 VTK 实现有限元应力云图可视化，使用 vtkUnstructuredGrid 加载网格，vtkLookupTable 映射颜色

# 变形动画
/cae-simulation 用 VTK 的 vtkWarpVector 实现结构变形动画，集成到 Qt 窗口

# CAE 平台集成
/cae-simulation 设计一个 FastCAE 插件，集成自定义的有限元求解器

# 完整仿真流程
/cae-simulation 实现从 STEP 几何导入 → Gmsh 网格划分 → deal.II 求解 → VTK 后处理的完整流程
```

**输出内容**：
- 仿真流程设计（几何 → 网格 → 求解 → 可视化）
- 技术选型说明
- 完整代码实现 + CMakeLists.txt
- 性能优化建议

---

### 3.5 /eda-dev — EDA/PCB 开发

**用途**：当你需要开发 EDA 相关功能，包括原理图编辑、PCB 布局布线、DRC 检查时使用。

**执行流程**：
```
读取 EDA 参考文档 → 需求分类与方案设计 → 输出技术方案与代码
```

**典型用法**：

```
# 自动布线
/eda-dev 设计一个基于 A* 算法的 PCB 自动布线器，支持双层板

# DRC 检查
/eda-dev 用 libspatialindex 实现高效的 PCB DRC 设计规则检查引擎

# 原理图编辑器
/eda-dev 参考 LibrePCB 的架构，设计一个原理图编辑器的核心数据模型

# Gerber 解析
/eda-dev 实现一个 Gerber RS-274X 文件解析器，支持 aperture 定义和绘制命令

# 元件库
/eda-dev 设计一个 PCB 元件库管理系统，基于 SQLite 存储，支持导入 KiCad 格式
```

**输出内容**：
- 系统架构设计
- 核心数据结构定义
- 关键算法实现 + CMakeLists.txt
- 文件格式支持方案

---

## 4. 直接提问模式

即使不使用 Slash 命令，`CLAUDE.md` 也会自动加载，为所有对话提供工业软件领域的专业上下文。适用于简单问题或快速查询：

```
# API 查询
> OCCT 的 Handle<T> 和 std::shared_ptr 有什么区别？

# 代码片段
> 帮我写一个 OSG 的 NodeVisitor 遍历场景图并打印节点类型的代码

# 概念解释
> BRep 建模中的 TopoDS_Shape、TopoDS_Solid、TopoDS_Face、TopoDS_Edge 有什么层次关系？

# 错误排查
> 我的 OCCT 布尔运算在大型装配体上很慢，可能的原因有哪些？

# 版本迁移
> OCCT 6.x 的 BRepAlgo_Cut 在 7.x 中对应的 API 是什么？
```

---

## 5. 非交互模式（脚本与批处理）

适用于自动化代码生成、批量处理、CI/CD 集成场景。

### 单次执行

```bash
# 生成代码并输出到文件
claude -p "/industrial-dev 用 OCCT 实现布尔并运算 BRepAlgoAPI_Fuse" > fuse_example.cpp

# 批量生成多个示例
claude -p "/industrial-dev 为 BRepAlgoAPI_Fuse、BRepAlgoAPI_Cut、BRepAlgoAPI_Common 各生成完整 C++ 示例"
```

### 配合管道使用

```bash
# 生成分析报告
claude -p "/source-analysis 分析 FreeCAD 的 Workbench 插件架构" > freecad_analysis.md

# 生成架构设计文档
claude -p "/architecture-design 为工业 CAD 平台设计插件化架构" > architecture.md
```

### 在 Shell 脚本中使用

```bash
#!/bin/bash
# batch_generate.sh — 批量生成 OCCT 布尔运算示例

OPERATIONS=("Fuse" "Cut" "Common")
for op in "${OPERATIONS[@]}"; do
    echo "Generating BRepAlgoAPI_${op}..."
    claude -p "/industrial-dev 用 OCCT 7.8 实现 BRepAlgoAPI_${op} 完整示例，包含 CMakeLists.txt" \
        > "example_${op,,}.cpp"
done
echo "Done."
```

---

## 6. 参考文档体系

命令执行时会**按需读取**以下文档，无需手动指定：

| 文档 | 行数 | 触发场景 | 主要内容 |
|------|------|----------|----------|
| `open-source-libraries.md` | ~2220 | 几乎所有命令 | 20+ 库的 API 示例、集成指南 |
| `architecture-patterns.md` | ~514 | `/architecture-design` | 分层/插件化/微服务架构模式 |
| `development-guide.md` | ~1149 | 环境配置需求 | CMake/vcpkg/Conan 工具链配置 |
| `industrial-protocols.md` | ~305 | 工业协议集成 | Modbus/OPC UA/MQTT 协议特性 |
| `device-connectivity.md` | ~523 | 设备互联 | 数据采集架构、边缘计算 |
| `security-compliance.md` | ~386 | 安全设计 | IEC 62443、等保 2.0 |
| `market-analysis.md` | ~627 | 商业决策 | 市场规模、竞品分析 |

你也可以在对话中直接要求 Claude 读取特定文档：

```
> 读取 references/open-source-libraries.md 中 OpenCASCADE 章节，帮我实现 STEP 文件导入
```

---

## 7. 命令速查表

| 命令 | 用途 | 适用场景 | 涉及核心库 |
|------|------|----------|-----------|
| `/industrial-dev` | 库集成 + 代码开发 | 写代码、集成开源库、功能实现 | OCCT、OSG、VTK、FreeCAD、CGAL 等 |
| `/source-analysis` | 源码分析 + 架构解读 | 阅读开源项目、学习设计模式 | 按分析目标而定 |
| `/architecture-design` | 架构设计 + 技术选型 | 新项目设计、技术评估、重构 | 视需求而定 |
| `/cae-simulation` | CAE/仿真开发 | 网格、求解、后处理可视化 | OpenFOAM、deal.II、Gmsh、VTK |
| `/eda-dev` | EDA/PCB 开发 | 原理图、布线、DRC、Gerber | LibrePCB、CGAL、libspatialindex |
| 直接提问 | 快速查询 | API 查询、概念解释、错误排查 | — |

---

## 8. 场景实战

### 8.1 从零搭建 CAD 平台

**目标**：开发一个类似 FreeCAD 的工业 CAD 软件。

```
# 第1步：架构设计
/architecture-design 为工业 CAD 平台设计插件化架构，要求：
- 支持几何建模、草图、装配三个核心模块
- 参考_FreeCAD Workbench 机制
- 几何内核使用 OCCT，渲染使用 OSG

# 第2步：OCCT 几何建模核心
/industrial-dev 封装 OCCT 几何建模服务层，提供统一的 Shape 创建、布尔运算、特征操作接口

# 第3步：OSG 渲染集成
/industrial-dev 实现 OCCT TopoDS_Shape 到 OSG 场景图的渲染管线，支持材质、光照和拾取

# 第4步：插件系统
/source-analysis 分析 FreeCAD 的 Workbench 和 Module 机制，提炼可复用的插件注册/加载设计

# 第5步：UI 界面
/industrial-dev 用 SARibbon 为 CAD 软件实现 Office 风格的 Ribbon 工具栏
```

### 8.2 集成 OCCT + OSG 渲染管线

**目标**：将 OCCT 几何模型在 OSG 场景中渲染。

```
# 第1步：理解 OCCT 网格化
/industrial-dev 用 OCCT 的 BRepMesh_IncrementalMesh 将 TopoDS_Shape 三角化，提取顶点和法线

# 第2步：构建 OSG 转换器
/industrial-dev 实现 TopoDS_Shape → osg::Geometry 转换器，处理法线、颜色和材质

# 第3步：场景管理
/industrial-dev 设计 OSG 场景图管理器，支持模型加载、更新、删除和拣选交互

# 第4步：性能优化
/industrial-dev 为大规模装配体实现 OSG LOD 和 Impostor 优化策略
```

### 8.3 开发 CAE 后处理系统

**目标**：可视化有限元仿真结果。

```
# 第1步：架构设计
/cae-simulation 设计 CAE 后处理可视化系统架构：数据加载 → 标量映射 → 渲染 → 交互

# 第2步：应力云图
/cae-simulation 用 VTK 实现有限元应力云图：vtkUnstructuredGrid + vtkLookupTable + vtkScalarBarActor

# 第3步：变形动画
/cae-simulation 用 vtkWarpVector 实现结构变形动画，支持动画播放控制

# 第4步：Qt 集成
/industrial-dev 将 VTK 渲染窗口嵌入 Qt 主窗口，支持工具栏交互
```

### 8.4 开发 PCB 自动布线工具

**目标**：实现 PCB 自动布线功能。

```
# 第1步：架构参考
/source-analysis 分析 LibrePCB 的项目结构和核心数据模型

# 第2步：布线算法
/eda-dev 实现基于 A* 的 PCB 自动布线算法，支持双层板和过孔

# 第3步：DRC 检查
/eda-dev 用 libspatialindex 实现空间索引加速的 DRC 设计规则检查

# 第4步：Gerber 输出
/eda-dev 实现 PCB 布局到 Gerber RS-274X 文件的导出
```

### 8.5 工业设备监控系统

**目标**：监控工厂设备，采集工业协议数据。

```
# 第1步：架构设计
/architecture-design 设计工业设备监控系统，支持 Modbus TCP 和 OPC UA 多协议采集

# 第2步：协议集成
/industrial-dev 用 pymodbus 实现 Modbus TCP 多设备轮询采集，支持断线重连

# 第3步：数据可视化
/industrial-dev 用 FastAPI + WebSocket 实现设备数据实时推送和 Web 可视化

# 第4步：安全合规
/architecture-design 设计工业数据安全方案，满足 IEC 62443 要求
```

---

## 9. 高级技巧

### 多命令协作

复杂项目可以串联多个命令，每个命令专注一个领域：

```
# 先做架构设计
/architecture-design 设计分层架构

# 再基于架构做代码开发
/industrial-dev 实现几何服务层接口

# 分析开源项目的实现方式
/source-analysis 看看 FreeCAD 是怎么做的

# 回到具体代码
/industrial-dev 按照架构设计实现渲染层
```

### 追问与迭代

命令输出的结果可以作为追问的起点：

```
/industrial-dev 用 OCCT 实现布尔差运算
→ [输出代码]

> 上面代码中 BRepMesh_IncrementalMesh 的 deflection 参数怎么选择？
→ [详细解释]

> 帮我把这段代码封装成一个函数，支持异步执行和进度回调
→ [重构代码]
```

### 自定义命令扩展

你可以在 `.claude/commands/` 下新增自己的命令：

```bash
# 例如：创建一个专门的 OpenFOAM 命令
cat > .claude/commands/openfoam-dev.md << 'EOF'
你是 OpenFOAM 仿真专家。

## 任务需求
$ARGUMENTS

## 执行流程
1. 读取 references/open-source-libraries.md 中 OpenFOAM 章节
2. 分析仿真需求，选择合适的求解器
3. 输出完整的案例配置和运行脚本
EOF
```

新命令立即可用：`/openfoam-dev 设计一个管道内流仿真案例`

### 调整 CLAUDE.md 适配你的项目

`CLAUDE.md` 中的编码规范可以根据你的项目定制：

```markdown
## C++ 编码规范（自定义示例）
- 命名规范：类名 PascalCase，函数 camelCase，成员变量 m_
- 使用 Qt 信号槽机制处理异步操作
- 所有几何运算通过 GeometryService 抽象接口访问
```

---

## 10. 常见问题

### Q: Claude Code 在哪里获取？

```bash
npm install -g @anthropic-ai/claude-code
```

安装后运行 `claude` 即可启动。

### Q: Slash 命令不显示怎么办？

确认 `.claude/commands/` 目录在你的项目根目录下，且 `.md` 文件格式正确。在 Claude Code 中输入 `/` 可以看到所有可用命令列表。

### Q: 参考文档必须和命令在同一个目录吗？

是的。命令中通过相对路径 `references/xxx.md` 引用文档，需要保证目录结构一致：

```
your-project/
├── .claude/commands/    ← 命令
├── references/          ← 参考文档（命令按路径引用）
└── CLAUDE.md            ← 项目上下文
```

### Q: 可以只安装部分命令吗？

可以。只复制你需要的 `.md` 文件到 `.claude/commands/` 即可。例如只做 CAD 开发：

```bash
mkdir -p .claude/commands
cp industrial-software-dev-skill/.claude/commands/industrial-dev.md .claude/commands/
```

### Q: 命令输出的代码可以直接编译吗？

命令会尽量输出完整可编译的代码和 CMakeLists.txt，但需要注意：

- 确认 OCCT/OSG/VTK 的本地安装路径
- 部分代码需要根据实际版本调整 API
- 建议先编译验证，再集成到项目中

### Q: 与 Cursor 版本有什么区别？

| 对比项 | Cursor 版本 | Claude Code 版本 |
|--------|-------------|-----------------|
| 入口 | `SKILL.md` 单文件 | `CLAUDE.md` + 5 个 Slash 命令 |
| 触发 | 自动识别关键词 | 用户输入 `/命令名` 精确触发 |
| 场景覆盖 | 全场景混合 | 按场景拆分为独立命令 |
| 参考文档 | Skill 内声明按需读取 | 命令内指定读取路径 |
| 编码规范 | `.cursorrules` | `CLAUDE.md` 内置 |
| 非交互模式 | 不支持 | `claude -p` 支持脚本批处理 |
| 适用场景 | IDE 交互式开发 | CLI + 自动化 + 脚本 |

### Q: 如何更新？

```bash
cd industrial-software-dev-skill
git pull
# 然后重新复制到你的项目目录
```

### Q: 支持哪些编程语言？

核心覆盖 **C++** 和 **Python**，部分命令也支持 C#（如 Macad3D）、TypeScript（如 JSketcher）等语言的指导。

---

## 附录：开源库 License 速查

| 库 | License | 商业使用 |
|----|---------|----------|
| OpenCASCADE (OCCT) | LGPL 2.1 | 动态链接可免版权 |
| OpenSceneGraph | LGPL 2.1 | 动态链接可免版权 |
| VTK | BSD | 自由使用 |
| FreeCAD | LGPL 2.1 | 动态链接可免版权 |
| CGAL | GPL / LGPL | 注意选型 |
| LibrePCB | GPL 3.0 | 衍生作品需开源 |
| SolveSpace | GPL 3.0 | 衍生作品需开源 |
| SARibbon | MIT | 自由使用 |
| deal.II | LGPL 2.1 | 动态链接可免版权 |
| Gmsh | GPL 3.0 | 衍生作品需开源 |
