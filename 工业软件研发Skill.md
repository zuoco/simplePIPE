# 工业软件代码研发 Skill

> 专为工业软件开发者设计的 Cursor AI Skill，深度覆盖 CAD/CAM/CAE/EDA/仿真/可视化全技术域，提供开源库集成指导、源码分析、架构设计与工程实践。

<p align="center">
  <img src="https://img.shields.io/badge/平台-Cursor-blue" />
  <img src="https://img.shields.io/badge/领域-工业软件-orange" />
  <img src="https://img.shields.io/badge/语言-C%2B%2B%20%7C%20Python-brightgreen" />
  <img src="https://img.shields.io/badge/License-MIT-lightgrey" />
</p>

---

## 目录

- [简介](#简介)
- [核心能力覆盖](#核心能力覆盖)
- [快速开始](#快速开始)
- [Skill 工作流程](#skill-工作流程)
- [参考文档体系](#参考文档体系)
- [开源库速查表](#开源库速查表)
- [典型使用场景](#典型使用场景)
- [Cursor 配置指南](#cursor-配置指南)
- [Claude Code 使用指南](#claude-code-使用指南)
- [项目结构](#项目结构)
- [注意事项](#注意事项)
- [贡献指南](#贡献指南)
- [License](#license)

---

## 简介

本项目是一个面向 **AI Coding** 的工业软件研发 Skill，旨在帮助开发者快速获得工业软件领域的专业 AI 辅助能力。

工业软件开发与通用软件开发存在显著差异：

- **几何内核复杂**：需要深入理解 OpenCASCADE、NURBS 曲面、BRep 拓扑等专业概念
- **渲染要求特殊**：场景图架构、LOD、大规模装配体渲染等非标准问题
- **协议种类繁多**：Modbus、OPC UA、MQTT、EtherCAT 等工业通信协议
- **精度与稳定性要求高**：数值精度、实时性、高可靠性是工业场景的硬性约束
- **开源生态复杂**：OCCT、OSG、VTK、FreeCAD 等库各有独特的编程模型

本 Skill 通过结构化的参考文档体系 + 精心设计的操作流程，让 Cursor 的 AI 能力真正适配工业软件研发场景。

---

## 核心能力覆盖

### 技术域全景

| 技术域 | 覆盖内容 |
|--------|----------|
| **CAD 图形平台** | FreeCAD、OpenSCAD、QCAD、LibreCAD、BRL-CAD、JSketcher、Macad3D |
| **几何造型器（内核）** | OpenCASCADE (OCCT)、AMCAX 九韶内核、openNURBS、LNLib、CGAL、libigl |
| **显示渲染器** | OpenSceneGraph (OSG)、VTK、Mayo |
| **CAE / 仿真** | OpenFOAM、FastCAE、deal.II、Gmsh |
| **几何约束求解器** | SolveSpace、PlaneGCS、psketcher |
| **EDA** | LibrePCB |
| **UI 界面** | SARibbon、QtRibbonGUI |
| **空间数据索引** | libspatialindex（R-tree / KNN）|
| **数据格式** | IFC++（BIM/IFC）|
| **字体引擎** | FreeType |
| **工业协议** | Modbus、OPC UA、MQTT、EtherCAT、PROFINET |
| **设备互联** | 工业网关、边缘计算、多协议适配 |
| **安全合规** | IEC 62443、等保 2.0、GDPR、工业数据安全 |
| **架构设计** | 插件化、微服务、分层架构、边缘计算架构 |
| **市场分析** | CAD/CAM/CAE/EDA/IoT 市场规模与竞品分析 |

### AI 辅助研发能力

- **源码阅读分析**：深入解析 FreeCAD、OCCT、OSG、VTK 等开源项目的架构与核心模块
- **API 查询与示例**：快速获取 OCCT 7.x / OSG / VTK API 的正确用法与 C++ 示例代码
- **集成方案设计**：OCCT + OSG、OCCT + VTK、Qt + OCCT 等多库联合集成方案
- **性能优化指导**：BVH 加速、Draw Call 合批、OSG LOD、OCCT 布尔运算优化
- **架构设计辅助**：插件体系、渲染层、几何服务层的架构设计建议
- **国产替代方案**：AMCAX 九韶内核、FastCAE 等国产工业软件开源库的集成指导


---

## 快速开始

### 方式一：直接在 Cursor 中使用（推荐）

**第一步：克隆本仓库**

```bash
git clone https://github.com/haiziyan/industrial-software-dev-skill.git
```

**第二步：在 Cursor 中添加 Skill**

打开 Cursor 设置 → **Rules** → **Add Rule** → 选择 `SKILL.md` 文件，或将其内容粘贴为 Project Rule。

**第三步：开始提问**

在 Cursor Chat 中直接描述你的工业软件开发需求，Skill 会自动按需读取参考文档：

```
帮我用 OCCT 实现一个圆柱体与长方体的布尔差运算，输出带 CMakeLists.txt 的 C++ 示例
```

### 方式二：作为项目 Rules 使用

将 `SKILL.md` 内容添加到项目根目录的 `.cursor/rules/` 目录下，配合 `references/` 目录一起使用，让整个团队共享同一套 AI 辅助规范。

### 方式三：配合工业编码规范使用

参见下方 [Cursor 配置指南](#cursor-配置指南)，在项目中同时部署 `.cursorrules` 工业编码规范，获得最佳 AI 辅助效果。

---

## Skill 工作流程

本 Skill 内置四种工作流，覆盖工业软件研发全场景：

### 流程 A：开源库源码参考与代码研发

适用于：集成 OCCT/OSG/VTK 等工业开源库、阅读分析第三方库源码、快速实现几何/渲染/仿真功能。

```
需求理解与库选型 → GitHub 源码阅读与分析 → 集成开发 → 性能优化与调试
```

**阶段详解：**

1. **需求理解与库选型**：明确功能域（几何建模/3D 渲染/数值仿真），对比候选库的 Star 趋势、License、API 成熟度，运行官方 Examples 验证可行性
2. **GitHub 源码阅读与分析**：解析顶层目录结构，追踪核心 API 实现链路，提炼可复用设计模式（Visitor、Command、Observer 等）
3. **集成开发**：搭建 CMake + vcpkg 工具链，封装第三方库接口，编写适配器层隔离业务与底层库耦合
4. **性能优化与调试**：使用 VTune/perf 定位热点，分析 OCCT BVH 加速、OSG StateSet 合批、VTK Pipeline 优化

### 流程 B：工业软件全生命周期管理

适用于：从零开始完整研发一套工业软件系统。

```
需求分析 → 系统设计 → 开发实施 → 测试验证 → 部署上线 → 运维优化
```

### 流程 C：技术选型与架构设计

适用于：评估多个候选技术方案、设计系统架构、输出技术选型报告。

```
需求分析 → 候选方案评估（功能/成本/学习曲线/License）→ POC 验证 → 决策输出
```

### 流程 D：市场分析与商业化策略

适用于：产品定位、竞品调研、商业模式设计。

```
市场调研（规模/趋势）→ 竞品分析（CATIA/SolidWorks/NX）→ 用户画像 → 商业化策略
```

### 渐进式文档读取机制

Skill 采用**按需读取**策略，避免一次性加载所有文档导致上下文过长：

| 用户问题类型 | 自动读取文档 |
|-------------|-------------|
| 几何内核 / CAD 开发 | open-source-libraries.md § 几何造型器 |
| CAD 平台集成 | open-source-libraries.md § CAD 图形平台 |
| 3D 渲染 / 可视化 | open-source-libraries.md § 显示渲染器 |
| 几何约束 / 草图 | open-source-libraries.md § 约束求解器 |
| CAE / 仿真 | open-source-libraries.md § CAE 章节 |
| UI / Ribbon 界面 | open-source-libraries.md § UI 界面 |
| BIM / IFC 格式 | open-source-libraries.md § 数据格式 |
| PCB / EDA | open-source-libraries.md § EDA |
| 空间检索加速 | open-source-libraries.md § 空间数据索引 |
| 工业协议集成 | industrial-protocols.md |
| 设备连接方案 | device-connectivity.md |
| 技术架构设计 | architecture-patterns.md |
| 安全与合规 | security-compliance.md |
| 市场分析 | market-analysis.md |
| 开发环境配置 | development-guide.md |


---

## 参考文档体系

`references/` 目录包含 7 份深度参考文档，合计约 5,700+ 行专业内容：

| 文档 | 主要内容 | 适用场景 |
|------|----------|----------|
| [open-source-libraries.md](references/open-source-libraries.md) | 20+ 工业开源库的详细介绍、API 示例、集成指南（OCCT/OSG/VTK/FreeCAD/CGAL/LibrePCB 等） | CAD/CAM/CAE/EDA 开发 |
| [architecture-patterns.md](references/architecture-patterns.md) | 分层、插件化、微服务、边缘计算等架构模式；CAP 理论工业场景权衡 | 系统设计、架构评审 |
| [development-guide.md](references/development-guide.md) | C++/Python 开发环境搭建、CMake/vcpkg/Conan 工具链、Cursor 配置、项目脚手架 | 环境配置、工具链选型 |
| [industrial-protocols.md](references/industrial-protocols.md) | Modbus/OPC UA/MQTT/EtherCAT/PROFINET 协议特性、配置参数、选型建议 | 工业协议集成 |
| [device-connectivity.md](references/device-connectivity.md) | 集中/分布/混合连接架构、数据采集策略、边缘计算、通信稳定性保障 | 设备互联方案设计 |
| [security-compliance.md](references/security-compliance.md) | IEC 62443、等保 2.0、GDPR、CIA 三要素、工业数据安全技术方案 | 安全设计、合规认证 |
| [market-analysis.md](references/market-analysis.md) | MES/SCADA/PLM/工业 IoT 市场规模、趋势分析、竞品对比、商业化策略 | 产品定位、商业决策 |

---

## 开源库速查表

### CAD 图形平台

| 库名 | GitHub | 语言 | 核心能力 | 典型用途 |
|------|--------|------|----------|----------|
| FreeCAD | [FreeCAD/FreeCAD](https://github.com/FreeCAD/FreeCAD) | C++/Python | 参数化建模、插件架构 | CAD 平台参考、插件开发 |
| OpenSCAD | [openscad/openscad](https://github.com/openscad/openscad) | C++ | CSG 脚本建模 | 3D 打印、参数化零件 |
| QCAD | [qcad/qcad](https://github.com/qcad/qcad) | C++/Qt | 2D CAD、DXF/DWG | 技术图纸、建筑平面图 |
| LibreCAD | [LibreCAD/LibreCAD](https://github.com/LibreCAD/LibreCAD) | C++/Qt | 2D 绘图、DXF | 轻量级 2D CAD |
| BRL-CAD | [BRL-CAD/brlcad](https://github.com/BRL-CAD/brlcad) | C/C++ | CSG+BRep、光线追踪 | 国防/工程仿真 |
| JSketcher | [xibyte/jsketcher](https://github.com/xibyte/jsketcher) | TypeScript/WASM | Web 3D 建模、2D 约束 | Web CAD、在线设计 |
| Macad3D | [Macaad3d/Macad3D](https://github.com/Macaad3d/Macad3D) | C#/.NET | C#+OCCT 3D CAD | .NET CAD 参考 |

### 几何造型器（内核）

| 库名 | GitHub/官网 | 语言 | 核心能力 | 典型用途 |
|------|------------|------|----------|----------|
| OpenCASCADE (OCCT) | [Open-Cascade-SAS/OCCT](https://github.com/Open-Cascade-SAS/OCCT) | C++ | BRep 建模内核、STEP/IGES | CAD/CAM 核心 |
| AMCAX 九韶内核 | [amcax.net](https://amcax.net/) | C++ | 国产 CAD/CAE/CAM 内核 | 国产替代 OCCT |
| openNURBS | [mcneel/opennurbs](https://github.com/mcneel/opennurbs) | C++ | NURBS、.3dm 格式 | Rhino 模型读写 |
| LNLib | [BIMCoderLiang/LNLib](https://github.com/BIMCoderLiang/LNLib) | C++ | NURBS 算法 | 曲面造型研究 |
| CGAL | [CGAL/cgal](https://github.com/CGAL/cgal) | C++ | 计算几何、网格布尔 | 碰撞检测、网格处理 |
| libigl | [libigl/libigl](https://github.com/libigl/libigl) | C++ | 几何处理算法 | 网格变形、UV 展开 |
| Open3D | [isl-org/Open3D](https://github.com/isl-org/Open3D) | C++/Python | 点云处理、3D 重建 | 工业测量、质检 |

### 显示渲染器

| 库名 | GitHub | 语言 | 核心能力 | 典型用途 |
|------|--------|------|----------|----------|
| OpenSceneGraph (OSG) | [openscenegraph/OpenSceneGraph](https://github.com/openscenegraph/OpenSceneGraph) | C++ | 场景图、3D 渲染 | 工业仿真、虚拟现实 |
| VTK | [Kitware/VTK](https://github.com/Kitware/VTK) | C++/Python | 科学可视化、体渲染 | CAE 后处理、医学影像 |
| Mayo | [fougue/mayo](https://github.com/fougue/mayo) | C++/Qt | 3D CAD 查看器 | 格式转换、OCCT 参考 |

### CAE / 仿真

| 库名 | GitHub | 语言 | 核心能力 | 典型用途 |
|------|--------|------|----------|----------|
| OpenFOAM | [OpenFOAM/OpenFOAM-dev](https://github.com/OpenFOAM/OpenFOAM-dev) | C++ | CFD 流体仿真 | 流体动力学 |
| FastCAE | [DISOGitHub/FastCAE](https://github.com/DISOGitHub/FastCAE) | C++/Python | 国产 CAE 集成平台 | CAE 前后处理插件框架 |
| deal.II | [dealii/dealii](https://github.com/dealii/dealii) | C++ | 有限元计算 | CAE 数值仿真 |
| Gmsh | [gmsh-org/gmsh](https://github.com/gmsh-org/gmsh) | C++/Python | 网格生成 | CAE 前处理 |

### 几何约束求解器

| 库名 | GitHub | 语言 | 核心能力 | 典型用途 |
|------|--------|------|----------|----------|
| SolveSpace | [solvespace/solvespace](https://github.com/solvespace/solvespace) | C/C++ | 参数化 3D CAD、约束求解 | 草图约束、机构仿真 |
| PlaneGCS | [CadQuery/PlaneGCS](https://github.com/CadQuery/PlaneGCS) | C++ | 2D 几何约束求解 | CAD 草图模块集成 |
| psketcher | [peizhan/psketcher](https://github.com/peizhan/psketcher) | C++ | 参数化草图约束 | 2D/3D 草图 |

### UI 界面 / 其他

| 库名 | GitHub | 语言 | 核心能力 | 典型用途 |
|------|--------|------|----------|----------|
| SARibbon | [czyt1988/SARibbon](https://github.com/czyt1988/SARibbon) | C++/Qt | Ribbon 控件 | CAD/工业软件 Office 风格 UI |
| QtRibbonGUI | [liang1057/QtRibbonGUI](https://github.com/liang1057/QtRibbonGUI) | C++/Qt | Ribbon 界面 | Qt Ribbon 快速集成 |
| libspatialindex | [libspatialindex/libspatialindex](https://github.com/libspatialindex/libspatialindex) | C++ | R-tree/KNN 空间索引 | 拾取加速、点云检索 |
| IFC++ | [ifcquery/ifcplusplus](https://github.com/ifcquery/ifcplusplus) | C++ | BIM/IFC 格式读写 | 建筑数字孪生、BIM 软件 |
| FreeType | [freetype.org](https://freetype.org/) | C | 字体渲染引擎 | 3D 软件文字、3D 文字挤出 |
| LibrePCB | [LibrePCB/LibrePCB](https://github.com/LibrePCB/LibrePCB) | C++/Qt | PCB 原理图+布局 | 开源 EDA 工具 |

---

## 典型使用场景

### 场景 1：用 OCCT 实现 CAD 布尔运算

**需求**：在工业软件中实现零件的求差（挖孔）功能。

**提问示例**：
```
帮我用 OpenCASCADE 7.x 实现圆柱体与长方体的布尔差运算，
要求：
1. 使用 BRepAlgoAPI_Cut
2. 用 BRepMesh_IncrementalMesh 生成可视化网格
3. 输出完整 C++ 代码 + CMakeLists.txt
```

**Skill 执行路径**：读取 open-source-libraries.md § OCCT → 生成带 Handle 机制的正确代码

---

### 场景 2：参考 FreeCAD 源码设计插件架构

**需求**：开发自己的 CAD 平台，需要设计可扩展的插件系统。

**提问示例**：
```
分析 FreeCAD 的 Workbench 插件机制源码结构，
帮我设计一套类似的插件注册/发现/加载机制，输出 C++ 接口定义
```

**Skill 执行路径**：读取 open-source-libraries.md § FreeCAD → 提炼 Workbench 设计模式 → 输出接口设计

---

### 场景 3：OSG + OCCT 联合集成

**需求**：将 OCCT 几何模型在 OSG 场景中渲染。

**提问示例**：
```
我需要将 OCCT 的 TopoDS_Shape 转换为 osg::Geometry 并在 OSG 场景中渲染，
请给出完整的转换器代码框架，包括法线计算和材质配置
```

**Skill 执行路径**：读取 open-source-libraries.md § OCCT + § OSG → 生成网格转换器代码

---

### 场景 4：VTK 实现 CAE 后处理可视化

**需求**：可视化有限元仿真结果（应力云图、变形动画）。

**提问示例**：
```
用 VTK 实现有限元应力云图可视化：
1. 使用 vtkUnstructuredGrid 加载网格数据
2. 用 vtkLookupTable 映射应力值到颜色
3. 添加 vtkScalarBarActor 图例
4. 集成到 Qt 窗口中
```

**Skill 执行路径**：读取 open-source-libraries.md § VTK → 生成 VTK Pipeline 

---

## Cursor 配置指南

在你的工业软件项目根目录创建 `.cursorrules` 文件，内容如下：

```
# 工业软件研发规范（Cursor Rules）

## 技术栈上下文
- 核心几何内核：OpenCASCADE (OCCT) C++ API
- 3D 渲染引擎：OpenSceneGraph (OSG) 或 VTK
- 构建系统：CMake 3.20+
- 包管理：vcpkg 或 Conan
- Python 绑定：pybind11 / pythonocc-core

## C++ 编码规范
- 使用 OCCT Handle<T> 管理几何对象生命周期，禁止裸指针持有 Transient 对象
- OSG 对象使用 osg::ref_ptr<T>，禁止裸指针
- 类名 PascalCase，函数名 camelCase，成员变量前缀 m_
- 每个公开 API 必须有 Doxygen 注释
- 使用 C++17 特性（std::optional, std::variant, structured bindings）
- 几何算法处理数值精度问题（使用 Precision::Confusion() 等 OCCT 精度常量）

## 架构规范
- 业务逻辑层不直接依赖 OCCT/OSG/VTK 具体类型，通过抽象接口访问
- 几何建模、可视化、数据管理三层严格分离
- 插件/模块通过接口注册，支持动态加载
- 大型几何运算放入后台线程，UI 线程保持响应

## 性能规范
- 3D 场景更新使用 OSG 的 NodeVisitor 模式，避免全场景遍历
- OCCT 布尔运算等耗时操作异步执行，提供进度回调
- 网格数据优先使用 VTK 的流水线（Pipeline）模式处理
- 启用 OCCT 的 BVH 空间加速结构用于碰撞检测和拾取

## 安全与稳定性
- OCCT 异常使用 Standard_Failure 捕获，不用 std::exception
- 文件格式解析（STEP/IGES/STL）必须做错误处理和格式验证
- 多线程访问 OCCT 对象需加锁（OCCT 非线程安全）
```

---

## Claude Code 使用指南

本 Skill 同样适用于 **Claude Code**（Anthropic 官方命令行 AI 编程工具），通过 `@SKILL.md` 引用即可获得与 Cursor 相同的工业软件研发辅助能力。

### 快速接入

**第一步：克隆仓库**

```bash
git clone https://github.com/haiziyan/industrial-software-dev-skill.git
```

**第二步：在对话中引用 SKILL.md**

Claude Code 支持通过 `@文件路径` 语法加载任意文件作为上下文，直接引用本仓库的 `SKILL.md` 即可：

```bash
# 启动 Claude Code
claude

# 引用 SKILL.md 作为上下文，然后提问
> @SKILL.md 帮我用 OCCT 实现圆柱体与长方体的布尔差运算
```

**第三步：按需引用参考文档**

针对具体问题，可同时引用对应的参考文档获得更精准的回答：

```bash
> @SKILL.md @references/open-source-libraries.md 帮我用 OCCT 实现布尔运算
> @SKILL.md @references/industrial-protocols.md 设计一个 OPC UA 数据采集方案
> @SKILL.md @references/architecture-patterns.md 为 CAD 软件设计插件架构
```

> **提示**：若需频繁使用，可将本仓库路径加入 Claude Code 的常用路径，避免每次输入完整路径。

### Cursor vs Claude Code 对比

| 功能 | Cursor | Claude Code |
|------|--------|-------------|
| Skill 加载方式 | `SKILL.md` 添加为 Project Rule，自动生效 | `@SKILL.md` 手动引用 |
| 参考文档加载 | Rules 中声明，按需自动读取 | `@文件名` 显式引用 |
| 代码编辑集成 | 内嵌编辑器，直接修改文件 | 命令行，支持直接编辑文件 |
| 多文件上下文 | 自动感知打开的文件 | 通过 `@` 显式指定 |
| 适合场景 | 交互式开发、大型项目 | 自动化任务、脚本批处理、CI/CD |

### Claude Code 提问模板

```bash
# OCCT 几何内核开发
> @SKILL.md @references/open-source-libraries.md
  用 OCCT 7.8 实现 [具体功能]，给出完整 C++ 代码和 CMakeLists.txt

# 工业协议集成
> @SKILL.md @references/industrial-protocols.md
  用 pymodbus 实现 Modbus TCP 多设备轮询采集，要求支持断线重连

# 系统架构设计
> @SKILL.md @references/architecture-patterns.md
  为工业 CAD 平台设计插件化架构，参考 FreeCAD Workbench 机制

# 批量生成示例代码（非交互模式）
claude -p "@SKILL.md @references/open-source-libraries.md 为 BRepAlgoAPI_Fuse、BRepAlgoAPI_Cut、BRepAlgoAPI_Common 各生成完整 C++ 示例，输出到 examples/ 目录"
```

---

## 项目结构

```
industrial-software-dev/
├── SKILL.md                        # Skill 主文件（Cursor & Claude Code 通用入口）
├── README.md                       # 本文档
└── references/                     # 深度参考文档库
    ├── open-source-libraries.md    # 20+ 工业开源库详细介绍与 API 示例
    ├── architecture-patterns.md    # 工业软件架构模式
    ├── development-guide.md        # 开发环境配置与工具指南
    ├── industrial-protocols.md     # 工业协议参考
    ├── device-connectivity.md      # 设备互联互通方案
    ├── security-compliance.md      # 数据安全与合规指南
    └── market-analysis.md          # 市场分析框架
```

---

## 注意事项

### 开源库使用注意

| 注意点 | 说明 |
|--------|------|
| **OCCT Handle 机制** | 所有继承自 `Standard_Transient` 的对象必须用 `Handle<T>` 管理，切勿使用裸指针或 `std::shared_ptr` |
| **OSG ref_ptr** | OSG 对象使用引用计数，必须用 `osg::ref_ptr<T>`，注意循环引用问题 |
| **VTK Pipeline** | VTK 采用流水线架构，修改数据后需调用 `Modified()` 触发更新 |
| **License 合规** | OCCT/FreeCAD/OSG 均为 LGPL，商业产品动态链接可免版权，但需保留库的版权声明 |
| **线程安全** | OCCT 核心库非线程安全，多线程场景需做好同步 |
| **版本兼容性** | OCCT 7.x 与 6.x API 有较大变化，集成前确认版本 |

### AI 辅助研发注意

- **提供上下文**：向 AI 提问时，提供具体的库版本号、头文件路径、错误信息
- **参考官方 Examples**：先阅读库的官方 Examples，再让 AI 帮助定制扩展
- **代码验证**：AI 生成的 OCCT/OSG 代码需在本地编译验证，API 可能随版本变化
- **渐进式提问**：复杂功能分步骤提问，先实现最小可用版本再逐步完善

### 工业领域特殊性

- 工业软件需要高可靠性、实时性、兼容性，与通用软件开发有显著差异
- 数据安全涉及生产安全，参考 `references/security-compliance.md`
- 设备互联参考 `references/device-connectivity.md` 设计稳定的通信机制

---

## 贡献指南

欢迎提交 PR 和 Issue！贡献方式：

1. **补充开源库**：在 `references/open-source-libraries.md` 中添加新的工业开源库介绍
2. **完善 API 示例**：为现有库补充更多实用代码示例
3. **更新工业协议**：在 `references/industrial-protocols.md` 中补充新协议
4. **优化工作流**：在 `SKILL.md` 中完善操作流程和提示词模板
5. **修正错误**：发现文档中的错误或过时信息，欢迎提 Issue 或直接 PR

### 贡献规范

- 代码示例需经过实际编译验证
- 标注库的版本号（如 OCCT 7.8.x）
- 中文撰写，技术术语保留英文原名
- 遵循现有文档的格式风格

---

## License

本项目采用 [MIT License](LICENSE) 开源。

参考文档中涉及的各开源库遵循其各自的许可证：

| 库 | License |
|----|--------|
| OpenCASCADE (OCCT) | LGPL 2.1 |
| OpenSceneGraph | LGPL 2.1 |
| VTK | BSD |
| FreeCAD | LGPL 2.1 |
| CGAL | GPL/LGPL |
| LibrePCB | GPL 3.0 |
| SolveSpace | GPL 3.0 |
| SARibbon | MIT |

---

<p align="center">
如果本项目对你有帮助，欢迎 Star 支持！
</p>

