---
name: industrial-software-dev
description: "专注工业软件代码研发：提供 OpenCASCADE、OpenSceneGraph、VTK、FreeCAD、SolveSpace、LibrePCB、FastCAE 等主流工业开源库的深度集成指导、源码阅读分析、架构设计与工程实践；覆盖 CAD 图形平台、几何造型器、显示渲染器、几何约束求解器、CAE/CAM、EDA、UI 界面、空间索引、数据格式等全领域；当用户需要开发 CAD/CAM/CAE/仿真/可视化/EDA 软件、参考开源库源码实现、集成几何内核或约束求解器、搭建工业软件研发环境、进行技术选型或解决 3D 建模/渲染/数值计算难题时使用"
---

# 工业软件代码研发

## 技能定位
本 Skill 聚焦于**工业软件代码研发**，核心能力包括：
- **CAD 图形平台**：FreeCAD、OpenSCAD、QCAD、LibreCAD、BRL-CAD、JSketcher、Macad3D 等
- **几何造型器（内核）**：OpenCASCADE (OCCT)、AMCAX 九韶内核（国产）、openNURBS、LNLib、CGAL、libigl
- **显示渲染器**：OpenSceneGraph (OSG)、VTK、Mayo
- **CAE/仿真**：OpenFOAM、FastCAE（国产）、deal.II、Gmsh
- **几何约束求解器**：SolveSpace、PlaneGCS、psketcher
- **EDA**：LibrePCB
- **UI 界面**：SARibbon、QtRibbonGUI（Qt Ribbon 控件）
- **空间数据索引**：libspatialindex（R-tree/KNN）
- **数据格式**：IFC++（BIM/IFC）
- **字体**：FreeType
- **GitHub 源码参考**：阅读、分析工业开源项目源码，提炼可复用设计模式
- **Cursor & Claude 辅助研发**：针对工业软件场景的 AI 辅助编码最佳实践

## 核心开源库速查

### CAD 图形平台（集成）

| 库名 | GitHub | 语言 | 核心能力 | 典型用途 |
|------|--------|------|----------|----------|
| FreeCAD | [FreeCAD/FreeCAD](https://github.com/FreeCAD/FreeCAD) | C++/Python | 参数化建模、插件架构 | CAD 平台参考、插件开发 |
| OpenSCAD | [openscad/openscad](https://github.com/openscad/openscad) | C++ | CSG 脚本建模 | 3D 打印、参数化零件 |
| QCAD | [qcad/qcad](https://github.com/qcad/qcad) | C++/Qt | 2D CAD、DXF/DWG | 技术图纸、建筑平面图 |
| LibreCAD | [LibreCAD/LibreCAD](https://github.com/LibreCAD/LibreCAD) | C++/Qt | 2D 绘图、DXF | 轻量级 2D CAD |
| BRL-CAD | [BRL-CAD/brlcad](https://github.com/BRL-CAD/brlcad) | C/C++ | CSG+BRep、光线追踪 | 国防/工程仿真 |
| JSketcher | [xibyte/jsketcher](https://github.com/xibyte/jsketcher) | TypeScript/WASM | Web 3D 建模、2D 约束 | Web CAD、在线设计 |
| Macad3D | [Macaad3d/Macad3D](https://github.com/Macaad3d/Macad3D) | C#/.NET | C#+OCCT 3D CAD | .NET CAD 参考 |
| PythonOCC-CAD | [qunat/Pythonocc-CAD](https://github.com/qunat/Pythonocc-CAD) | Python | Python CAD 框架 | 快速原型开发 |

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

### UI 界面

| 库名 | GitHub | 语言 | 核心能力 | 典型用途 |
|------|--------|------|----------|----------|
| SARibbon | [czyt1988/SARibbon](https://github.com/czyt1988/SARibbon) | C++/Qt | Ribbon 控件 | CAD/工业软件 Office 风格 UI |
| QtRibbonGUI | [liang1057/QtRibbonGUI](https://github.com/liang1057/QtRibbonGUI) | C++/Qt | Ribbon 界面 | Qt Ribbon 快速集成 |

### 空间索引 / 数据格式 / 字体

| 库名 | GitHub | 语言 | 核心能力 | 典型用途 |
|------|--------|------|----------|----------|
| libspatialindex | [libspatialindex/libspatialindex](https://github.com/libspatialindex/libspatialindex) | C++ | R-tree/KNN 空间索引 | 拾取加速、点云检索 |
| IFC++ | [ifcquery/ifcplusplus](https://github.com/ifcquery/ifcplusplus) | C++ | BIM/IFC 格式读写 | 建筑数字孪生、BIM 软件 |
| FreeType | [freetype.org](https://freetype.org/) | C | 字体渲染引擎 | 3D 软件文字、3D 文字挤出 |

### EDA

| 库名 | GitHub | 语言 | 核心能力 | 典型用途 |
|------|--------|------|----------|----------|
| LibrePCB | [LibrePCB/LibrePCB](https://github.com/LibrePCB/LibrePCB) | C++/Qt | PCB 原理图+布局 | 开源 EDA 工具 |

## 前置准备
- 根据任务类型按需读取参考文档（渐进式披露原则）：
  - 涉及几何内核/CAD 开发时：读取 `references/open-source-libraries.md` → 几何造型器章节（OCCT/AMCAX/openNURBS）
  - 涉及 CAD 集成平台时：读取 `references/open-source-libraries.md` → CAD 图形平台章节（FreeCAD/OpenSCAD/QCAD）
  - 涉及 3D 可视化/渲染时：读取 `references/open-source-libraries.md` → 显示渲染器章节（OSG/VTK/Mayo）
  - 涉及几何约束/草图时：读取 `references/open-source-libraries.md` → 几何约束求解器章节（SolveSpace/PlaneGCS）
  - 涉及 CAE/仿真时：读取 `references/open-source-libraries.md` → CAE 章节（OpenFOAM/FastCAE/deal.II/Gmsh）
  - 涉及 UI 界面开发时：读取 `references/open-source-libraries.md` → UI 界面章节（SARibbon/QtRibbonGUI）
  - 涉及 BIM/IFC 格式时：读取 `references/open-source-libraries.md` → 数据格式章节（IFC++）
  - 涉及 PCB/EDA 时：读取 `references/open-source-libraries.md` → EDA 章节（LibrePCB）
  - 涉及空间检索加速时：读取 `references/open-source-libraries.md` → 空间数据索引章节（libspatialindex）
  - 涉及字体/3D 文字时：读取 `references/open-source-libraries.md` → 字体章节（FreeType）
  - 涉及工业协议集成时：读取 `references/industrial-protocols.md`
  - 涉及安全合规时：读取 `references/security-compliance.md`
  - 涉及设备连接时：读取 `references/device-connectivity.md`
  - 涉及技术架构时：读取 `references/architecture-patterns.md`
  - 涉及市场分析时：读取 `references/market-analysis.md`
  - 涉及开发环境配置时：读取 `references/development-guide.md`

## 操作步骤

### 流程A：开源库源码参考与代码研发

#### 阶段1：需求理解与库选型
1. **明确技术需求**
   - 确认功能域（几何建模 / 3D 渲染 / 数值仿真 / 数据采集 / 可视化）
   - 评估性能要求（实时渲染帧率 / 计算精度 / 并发量）
   - 确认语言绑定（纯 C++ / Python 绑定 / 混合）
2. **开源库速查**（参考上方核心库速查表 + `references/open-source-libraries.md`）
   - 对比候选库的 GitHub Star 趋势、Issue 活跃度、最近提交频率
   - 查阅 License（LGPL/Apache/MIT）确认商业可用性
   - 下载并运行官方 Examples 验证可行性
3. **关键输出**：技术选型报告、POC 验证结果

#### 阶段2：GitHub 源码阅读与分析
1. **源码结构解析**
   - 克隆目标仓库，理解顶层目录结构（src/include/examples/tests）
   - 阅读 CMakeLists.txt 理解构建依赖关系
   - 定位核心模块（如 OCCT 的 BRep 模块、OSG 的 NodeKit 体系）
2. **关键路径追踪**
   - 从 Examples 入手，追踪核心 API 的实现链路
   - 阅读头文件接口定义，理解类继承关系和设计模式
   - 重点关注：数据结构、算法实现、内存管理策略
3. **可复用设计提炼**
   - 识别可借鉴的设计模式（Visitor、Command、Observer 等）
   - 提炼接口设计规范和命名约定
   - 记录关键 API 和使用限制
4. **关键输出**：源码阅读笔记、API 使用指南、架构关系图

#### 阶段3：集成开发
1. **环境搭建**（参考 `references/development-guide.md`）
   - 配置 C++ 工具链（CMake + vcpkg/Conan）
   - 配置 Python 绑定环境（conda / venv）
   - 配置 Cursor .cursorrules 工业软件研发规范
2. **接口封装**
   - 封装第三方库接口，隔离上层业务与底层库耦合
   - 设计适配器层，统一错误处理和资源管理
   - 编写类型安全的 C++ Wrapper 或 Python 绑定
3. **核心功能实现**
   - 参照开源库 Examples 实现业务功能
   - 遵循库的最佳实践（如 OCCT Handle 机制、OSG ref_ptr 智能指针）
   - 编写单元测试和集成测试
4. **关键输出**：集成代码、封装层、测试套件、技术文档

#### 阶段4：性能优化与调试
1. **性能分析**
   - 使用 VTune / perf / gprof 定位热点
   - 分析内存使用（Valgrind / AddressSanitizer）
   - 3D 渲染性能：检查 Draw Call、LOD、Culling
2. **优化策略**
   - 几何计算：利用 OCCT BVH 加速空间查询
   - 渲染优化：OSG StateSet 合批、OSG Impostor LOD
   - 并行化：OpenMP / TBB 加速数值计算
3. **关键输出**：性能基准报告、优化方案

### 流程B：工业软件全生命周期管理

#### 阶段1: 需求分析
1. **需求收集**
   - 识别业务场景和用户角色
   - 明确功能需求和非功能需求（性能、可靠性、安全性）
   - 梳理工业现场约束（设备兼容性、网络环境、部署环境）
2. **需求分析**
   - 生成需求规格说明书
   - 识别关键业务流程和数据流
   - 评估技术可行性和风险
3. **关键输出物**：需求规格说明书、业务流程图、风险评估报告

#### 阶段2: 系统设计
1. **架构设计**（参考 `references/architecture-patterns.md`）
   - 选择合适架构模式（分层 / 微服务 / 插件化）
   - 设计系统组件和模块划分
   - 定义接口规范和数据模型
2. **技术选型**
   - 根据业务需求选择技术栈
   - 选择合适的开源库（参考 `references/open-source-libraries.md`）
   - 评估工业协议兼容性（参考 `references/industrial-protocols.md`）
3. **关键输出物**：系统架构设计文档、技术选型方案、接口定义文档

#### 阶段3: 开发实施
1. **开发准备**（参考 `references/development-guide.md`）
   - 搭建开发环境和构建工具链
   - 建立代码规范和版本管理策略
2. **编码实现**
   - 按照设计文档进行模块化开发
   - 集成开源库（参考 `references/open-source-libraries.md`）
   - 实现工业协议适配层（参考 `references/device-connectivity.md`）
3. **关键输出物**：源代码库、单元测试、技术文档

#### 阶段4: 测试验证
1. **测试策略**：功能测试、性能测试、安全测试、兼容性测试
2. **工业场景测试**：模拟现场环境、设备互联稳定性、异常恢复
3. **关键输出物**：测试报告、缺陷清单、性能基准

#### 阶段5: 部署上线
1. **部署准备**：部署计划、回滚方案、生产环境配置
2. **灰度发布**：分阶段上线、监控告警、用户反馈
3. **关键输出物**：部署文档、监控配置、上线报告

#### 阶段6: 运维优化
1. **运维监控**：监控体系、告警机制、定期巡检
2. **持续改进**：收集用户反馈、迭代优化、维护技术文档
3. **关键输出物**：运维手册、性能优化报告、版本迭代计划

### 流程C：技术选型与架构设计

#### 技术选型决策框架
1. **需求分析**：明确业务需求、技术约束、关键性能指标
2. **方案评估**：列出候选方案、建立评估维度（功能/成本/学习曲线/License）、POC 验证
3. **决策输出**：技术选型对比表、决策依据、风险评估、技术路线图

#### 架构设计指导
1. **架构模式选择**（参考 `references/architecture-patterns.md`）
   - 根据系统规模选择：分层架构 / 插件化 / 微服务 / 边缘计算
   - CAD/CAE 软件优先考虑插件化架构（参考 FreeCAD 的 Workbench 机制）
2. **关键设计决策**：数据一致性策略、服务间通信协议、容错方案
3. **架构评审**：识别潜在风险、优化方案

### 流程D：市场分析与商业化策略

#### 市场分析
1. **市场调研**：参考 `references/market-analysis.md`，分析目标市场规模与趋势
2. **竞品分析**：调研 CATIA、SolidWorks、NX 等商业软件，对比开源竞品
3. **用户画像**：描述典型用户角色、使用场景、痛点与优先级

#### 商业化策略
1. **产品定位**：核心价值与差异化（技术深度 / 垂直行业 / 国产替代）
2. **定价策略**：开源核心 + 商业增值服务、按席位/按使用量
3. **推广策略**：技术社区、行业展会、合作伙伴生态

## Cursor 专用配置

### .cursorrules 工业软件研发规范

在项目根目录创建 `.cursorrules` 文件，内容如下：

```
# 工业软件研发规范（Cursor Rules）

## 技术栈上下文
- 核心几何内核：OpenCASCADE (OCCT) C++ API
- 3D 渲染引擎：OpenSceneGraph (OSG) 或 VTK
- 构建系统：CMake 3.20+
- 包管理：vcpkg 或 Conan
- Python 绑定：pybind11 / pythonocc-core

## C++ 编码规范
- 使用 OCCT Handle<T> 智能指针管理几何对象生命周期，禁止裸指针持有 Transient 对象
- OSG 对象使用 osg::ref_ptr<T>，禁止裸指针
- 类名使用 PascalCase，函数名使用 camelCase，成员变量前缀 m_
- 每个公开 API 必须有 Doxygen 注释
- 使用 C++17 特性（std::optional, std::variant, structured bindings）
- 几何算法实现要处理数值精度问题（使用 Precision::Confusion() 等 OCCT 精度常量）

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

### Claude 常用提示词模板

```
# 开源库 API 查询
查阅 OpenCASCADE 源码，帮我实现 [具体功能]，要求使用 OCCT 7.x API，给出 C++ 示例代码

# 源码分析
分析 FreeCAD 的 [模块名] 模块源码结构，解释其设计模式和关键类的职责

# 架构设计
参考 OpenSceneGraph 的场景图设计，为我的工业仿真软件设计渲染层架构

# 集成方案
我需要在 Qt 应用中集成 VTK 渲染窗口，同时使用 OCCT 做几何建模，给出集成方案和代码框架

# 性能问题排查
我的 OCCT 布尔运算在大型装配体上很慢，帮我分析原因并给出优化方案

# 代码迁移
将以下 OCCT 6.x 代码迁移到 7.x API：[代码]
```

## 资源索引

| 文档 | 用途 |
|------|------|
| [references/open-source-libraries.md](references/open-source-libraries.md) | OCCT/OSG/VTK/FreeCAD/CGAL 等库的详细介绍、API 示例、集成指南 |
| [references/architecture-patterns.md](references/architecture-patterns.md) | 工业软件架构设计模式、技术栈选择 |
| [references/development-guide.md](references/development-guide.md) | 开发环境搭建、Cursor/Claude 配置、项目脚手架 |
| [references/industrial-protocols.md](references/industrial-protocols.md) | 工业协议特性与选型（Modbus/OPC UA/MQTT 等） |
| [references/device-connectivity.md](references/device-connectivity.md) | 设备互联技术方案与最佳实践 |
| [references/security-compliance.md](references/security-compliance.md) | 工业数据安全要求与合规标准 |
| [references/market-analysis.md](references/market-analysis.md) | 工业软件市场分析方法与商业化策略 |

## 注意事项

### 开源库使用注意
- **OCCT Handle 机制**：所有继承自 `Standard_Transient` 的对象必须使用 `Handle<T>` 管理，切勿使用裸指针或 `std::shared_ptr`
- **OSG ref_ptr**：OSG 对象使用引用计数，必须用 `osg::ref_ptr<T>`，注意循环引用问题
- **VTK Pipeline**：VTK 采用流水线架构，修改数据后需调用 `Modified()` 触发更新
- **License 合规**：OCCT/FreeCAD/OSG 均为 LGPL，商业产品动态链接可免版权，但需保留库的版权声明
- **线程安全**：OCCT 核心库非线程安全，多线程场景需做好同步
- **版本兼容性**：OCCT 7.x 与 6.x API 有较大变化，集成前确认版本

### AI 辅助研发注意
- **上下文提供**：向 Claude/Cursor 提问时，提供具体的库版本号、头文件路径、错误信息
- **参考官方 Examples**：先阅读库的官方 Examples，再让 AI 帮助定制扩展
- **代码验证**：AI 生成的 OCCT/OSG 代码需在本地编译验证，API 可能随版本变化
- **渐进式提问**：复杂功能分步骤提问，先实现最小可用版本再逐步完善

### 工业领域特殊性
- 工业软件需要高可靠性、实时性、兼容性，与通用软件开发有显著差异
- 数据安全：工业数据涉及生产安全，参考 `references/security-compliance.md`
- 设备互联：参考 `references/device-connectivity.md` 设计稳定的通信机制

## 使用示例

### 示例1：使用 OCCT 实现 CAD 布尔运算
**场景**：需要在工业软件中实现零件的求差（挖孔）功能

**执行方式**：
1. 读取 `references/open-source-libraries.md` → OCCT 章节，查阅 BooleanOperations API
2. 使用 OCCT `BRepAlgoAPI_Cut` 实现布尔差运算
3. 通过 `BRep_Builder` + `TopoDS_Compound` 管理复合体
4. 使用 `BRepMesh_IncrementalMesh` 生成可视化网格
5. 输出：可运行的 C++ 代码 + CMakeLists.txt 配置

### 示例2：参考 FreeCAD 源码设计插件架构
**场景**：开发自己的 CAD 平台，需要设计可扩展的插件系统

**执行方式**：
1. 分析 FreeCAD GitHub 仓库的 `src/App/` 和 `src/Gui/` 结构
2. 提炼 FreeCAD Workbench 机制的核心设计模式
3. 设计基于接口的插件注册/发现/加载机制
4. 输出：插件架构设计文档 + 核心接口 C++ 定义

### 示例3：OSG + OCCT 联合集成
**场景**：将 OCCT 的几何模型在 OSG 场景中渲染

**执行方式**：
1. 读取 `references/open-source-libraries.md` → OCCT 和 OSG 章节
2. 实现 OCCT TopoDS_Shape → osg::Geometry 的网格转换器
3. 管理 OSG 场景图中几何节点的更新与删除
4. 配置 OSG 光照、材质与拾取（Pick）交互
5. 输出：完整的集成代码框架 + 使用示例

### 示例4：VTK 实现 CAE 后处理可视化
**场景**：需要可视化有限元仿真结果（应力云图、变形动画）

**执行方式**：
1. 读取 `references/open-source-libraries.md` → VTK 章节
2. 使用 `vtkUnstructuredGrid` 加载有限元网格数据
3. 使用 `vtkLookupTable` + `vtkScalarBarActor` 实现应力云图
4. 使用 `vtkWarpVector` 实现变形动画
5. 输出：VTK 可视化代码 + Qt 窗口集成方案

### 示例5：Open3D 实现工业点云质检
**场景**：使用 3D 扫描仪采集点云，与 CAD 模型对比进行尺寸检测

**执行方式**：
1. 读取 `references/open-source-libraries.md` → Open3D 章节
2. 使用 Open3D 加载并预处理点云（去噪、下采样、法线估计）
3. 使用 ICP（迭代最近点）算法将点云配准到 CAD 参考模型
4. 计算点到面距离，生成偏差热图
5. 输出：完整质检流程代码 + 可视化报告

### 示例6：开发工业设备监控系统
**场景**：监控工厂生产设备，采集 Modbus/OPC UA 数据并可视化

**执行方式**：
1. 读取 `references/industrial-protocols.md` 了解协议特性
2. 读取 `references/device-connectivity.md` 设计数据采集架构
3. 读取 `references/architecture-patterns.md` 选择合适架构模式
4. 使用 pymodbus + opcua-asyncio 实现多协议采集
5. 输出：系统设计方案 + 技术选型建议 + 关键代码实现

### 示例7：配置 Claude Code 开发工业物联网平台
**场景**：使用 Claude Code 开发工业物联网平台

**执行方式**：
1. 读取 `references/development-guide.md` 获取环境配置指南
2. 确定技术栈（FastAPI + InfluxDB + Redis）
3. 生成项目脚手架和配置文件
4. 提供 Claude Code 的专用配置和提示词模板
5. 输出：完整的项目结构 + 开发指南
 