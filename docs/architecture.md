# 管道系统参数化建模软件 — 架构设计文档

> **版本**: 0.4.1 | **日期**: 2026-04-06 | **状态**: Phase 4 完成，架构稳定期

---

## 1. 概述

基于 **OCCT**(几何内核) + **VSG**(Vulkan 渲染) + **VTK**(分析可视化) + **QML**(UI 框架) 构建海上油气/风电平台管道系统参数化建模与应力分析软件。

**核心设计理念**: 以 **管点(PipePoint)** 为中心的数据模型 — 管点是带坐标和类型的文档对象，管件几何由管点序列 + 管线特性(PipeSpec) 推导生成。管点按段(Segment)组织，段按树状结构组成路由(Route)。

采用 **8 层工程分层（7 个核心库层 + 1 个应用入口层）**，QML 通过 FBO 嵌入 VSG 3D 视口，仅法兰连接，STEP 格式数据交换。

**2026-04-06 同步说明**: `src/lib` 与 `src/apps` 已成为唯一构建入口；命令编辑流程统一使用 `CommandStack` + `CommandRegistry` + `CommandContext`；当前基线为测试 46/46 全通过。构建脚本 (`scripts/`) 已移除，统一通过 pixi 任务执行构建与测试。

---

## 2. 需求摘要

| 项目 | 决策 |
|------|------|
| **核心数据模型** | 管点(PipePoint) — 坐标+类型，管件几何由管点序列推导 |
| **管点类型** | Run(管段)、Bend(弯头)、Reducer(异径)、Tee(三通)、Valve(阀门)、FlexJoint(柔性接头) |
| **管点命名** | 字母=段ID + 数字=序号，如 A00→A01→A02；三通处分叉新段 B00→... |
| **管线特性(PipeSpec)** | 文档对象，管点级别引用；核心字段: OD/WT/材料等，支持自定义扩展 |
| **Bend 特殊管点** | 4个: 交点(Axx)、近端N、中点M、远端F；弯曲半径=OD×倍数(1.5D/2D/5D) |
| **几何推导** | 管件几何尺寸由管点坐标+PipeSpec决定 |
| **段与路由** | 路由=段的树状集合，三通处分叉 |
| **附属构件** | 固定点/支架/法兰/垫片/密封圈 — 独立文档对象 |
| **梁(Beam)** | 独立文档对象，引用两个管点 |
| **连接方式** | 仅法兰连接 |
| **UI** | QML (表格输入管点) + VSG 渲染到纹理(FBO) |
| **数据格式** | 工程文件用 JSON(只存参数); STEP 仅作为几何导出格式 |
| **VTK** | AnalysisWorkbench 采用 VTK 渲染，支持实体/线条(梁单元)双渲染模式 |
| **工作台** | 三工作台: Specification(规格管理) / Design(路由设计) / Analysis(应力分析) |
| **规模** | 海上平台, 5k-30k 构件 |
| **单位制** | 初期全公制(mm/deg/MPa)，内部统一公制存储，单位转换后续增加 |

---

## 3. 核心数据模型

### 3.1 文档对象体系

```
DocumentObject (基类: ID + name, 无坐标)
  │
  ├─── SpatialObject (有坐标基类: + position:gp_Pnt)
  │      ├── PipePoint       管点 (类型枚举 + PipeSpec引用 + 类型参数)
  │      ├── Accessory       附属构件基类 (关联管点引用 + offset + subType)
  │      │    ├── Support    支撑件 (subType: Rigid/Spring/Constant/Shoe/Trunnion/Guide)
  │      │    ├── Restraint  限位件 (subType: Anchor/LineStop/Snubber)
  │      │    ├── Flange     法兰   (subType: WeldNeck/SlipOn/Blind)
  │      │    └── Gasket     密封件 (subType: SpiralWound/RingJoint)
  │      └── Beam            梁 (双端管点引用 + 截面参数)
  │
  ├─── PropertyObject (无坐标基类: 属性/配置文档)
  │      ├── PipeSpec        管线特性 (可扩展字段: OD/WT/材料...)
  │      ├── ProjectConfig   工程配置 (名称/作者/标准/单位制/重力方向)
  │      ├── Load            载荷基类 (作用范围 + 载荷类型标识)
  │      │    ├── DeadWeightLoad    自重载荷
  │      │    ├── ThermalLoad       热载荷 (安装温度/操作温度)
  │      │    ├── PressureLoad      压力载荷 (内压/外压)
  │      │    ├── WindLoad          风载荷 (风速/方向)
  │      │    ├── SeismicLoad       地震载荷 (加速度/方向)
  │      │    ├── DisplacementLoad  位移载荷 (强制位移/转角)
  │      │    └── UserDefinedLoad   自定义载荷 (集中力/力矩)
  │      ├── LoadCase        基本工况 (一种物理状态下的载荷集合)
  │      └── LoadCombination 组合工况 (按规范对基本工况进行代数运算)
  │
  └─── ContainerObject (无坐标: 组织容器, 关联起始管点)
         ├── Segment         段 (有序管点列表 + 段ID + 关联起始管点)
         └── Route           路由 (段的树状集合 + 关联起始管点)
```

### 3.2 管点(PipePoint)

```
PipePoint {
  id: UUID
  name: string          // "A00", "A06", "A06N", "A06M", "A06F"
  position: gp_Pnt      // 3D坐标 (仅位置，方向由相邻点推导)
  type: enum            // Run | Bend | Reducer | Tee | Valve | FlexJoint
  pipeSpec: ref<PipeSpec>  // 引用管线特性
  
  // 类型特有参数 (variant/map存储)
  // Bend: bendMultiplier (1.5 | 2.0 | 5.0)
  // Valve: valveType, bodyDimensions
  // FlexJoint: flexParams
  // Run/Reducer/Tee: 无额外参数(由管点序列+PipeSpec推导)
}
```

### 3.3 Bend 的 4 管点模型

```
    A05 ----[A06N]---arc---[A06M]---arc---[A06F]---- A07
                  \                         /
                   \       [A06]           /  
                    \   (交点,虚拟)       /
                     \_________________/
                     
A06  = 弯头管点(交点，前后管段中心线延长交点)
A06N = Near端(靠近A05)
A06M = Mid中点(弧中点)  
A06F = Far端(靠近A07)

弯曲半径 R = OD × bendMultiplier
弯头角度 θ = π - angle(A05→A06, A06→A07)
```

**关键特性**: 弯头上的 4 个管点(A06/A06N/A06M/A06F)都是 **可选择的 SpatialObject**，各自拥有精确坐标。这对 AnalysisWorkbench 至关重要 — 弯头上的每个管点可以独立添加载荷（如在 A06N 上施加集中力、在 A06M 上挂载支撑等）。在 3D 视口中，这 4 个管点均可被拾取和高亮。

### 3.4 管线特性(PipeSpec)

```
PipeSpec {
  id: UUID
  name: string              // "6inch-Sch40"
  fields: map<string, Variant>  // 可扩展字段字典
  
  // 初期核心字段:
  //   "OD"             外径 (mm)
  //   "wallThickness"  壁厚 (mm)
  //   "material"       材料名称 (string)
  //   "designPressure" 设计压力 (MPa) [可选]
  //   "designTemp"     设计温度 (℃) [可选]
}
```

### 3.5 工程配置(ProjectConfig)

```
ProjectConfig {
  id: UUID
  name: string              // 工程名称
  author: string
  standard: string          // "ASME B31.3"
  unitSystem: enum          // SI | Imperial
  gravityAxis: enum         // X | Y | Z
}
```

### 3.6 几何推导规则

| 管点类型 | 几何推导方式 |
|---------|-------------|
| **Run** | 圆柱壳(外径-内径), 长度=\|pos[i+1]-pos[i]\|, 方向=normalize(pos[i+1]-pos[i]) |
| **Bend** | 弯管壳, R=OD×mult, 角度由交点和前后方向计算, N/M/F由R和角度导出 |
| **Reducer** | 锥壳, 大端OD=前段PipeSpec.OD, 小端OD=后段PipeSpec.OD |
| **Tee** | 主管+支管融合, 主管OD=当前PipeSpec.OD, 支管OD=分支段PipeSpec.OD |
| **Valve** | 参数化阀体, 端口OD=PipeSpec.OD, 阀体尺寸由valveSubType+OD查ComponentCatalog模板生成 |
| **FlexJoint** | 波纹管壳, OD=PipeSpec.OD, 长度=两点距离 |

### 3.7 参数化构件模板库 (ComponentCatalog)

管道软件与通用 CAD 的核心差异：构件形状拓扑固定，尺寸按管径参数化缩放。不需要草图系统或 STEP 文件库，而是**参数化生成规则的代码库**。

```
ComponentCatalog (单例, Layer 4: engine/)
  │
  ├── FittingTemplate (管线构件模板基类)
  │     ├── PipeTemplate          直管 — 圆柱壳
  │     ├── ElbowTemplate         弯头 — 弯管壳
  │     ├── TeeTemplate           三通 — 主管+支管融合
  │     ├── ReducerTemplate       异径管 — 锥壳
  │     ├── ValveTemplate         阀门
  │     │     ├── GateValve       闸阀 — 阀体+手轮
  │     │     ├── GlobeValve      截止阀 — 球形阀体
  │     │     ├── BallValve       球阀 — 扁圆阀体+操作手柄
  │     │     ├── CheckValve      止回阀 — 非对称阀体
  │     │     └── ButterflyValve  蝶阀 — 薄盘片阀体
  │     └── FlangeTemplate        法兰
  │           ├── WeldNeckFlange  对焘法兰
  │           ├── SlipOnFlange    平焘法兰
  │           └── BlindFlange     盲板法兰
  │
  └── AccessoryTemplate (附属构件模板基类)
        ├── SupportTemplate       支撑件
        │     ├── RigidSupport    刚性支撑 — 简单底板+立柱
        │     ├── SpringHanger    弹簧支吊架 — 弹簧体+吊杆
        │     ├── ConstantHanger  恒力支吊架 — 恒力机构
        │     ├── PipeShoe        管鞋/管托 — U形或鞍形
        │     ├── Trunnion        耳轴 — 圆管+底板
        │     └── Guide           导向 — 两侧挡板
        ├── RestraintTemplate     限位
        │     ├── Anchor          锚固 — 全约束块
        │     ├── LineStop        限位挡 — 单向/双向挡板
        │     └── Snubber         减振器 — 液压缸体
        └── GasketTemplate       密封
              ├── SpiralWound     缠绕垫片 — 扁环
              └── RingJoint       环连接垫片 — 八角截面环
```

#### 3.7.1 模板的核心职责

每个模板定义三样东西:

**1. 参数表** — 由管径 OD 驱动，所有尺寸都是 OD 的函数:
```cpp
struct ComponentParams {
    double od;              // 管外径 (mm) — 主驱动参数
    double wallThickness;   // 壁厚 (mm)
    double bodyLength;      // 构件长度
    double bodyWidth;       // 构件宽度
    double bodyHeight;      // 构件高度
    // ... 各类型有不同参数
};
```

**2. 尺寸比例规则** — 保证任何管径下构件外观协调:
```cpp
// 示例: 闸阀的参数化规则
ComponentParams GateValveTemplate::deriveParams(double od, double wt) {
    return {
        .od = od,
        .wallThickness = wt,
        .bodyLength  = od * 1.2,      // 阀体长 ≈ 1.2×OD
        .bodyWidth   = od * 1.5,      // 阀体宽 ≈ 1.5×OD
        .bodyHeight  = od * 3.0,      // 含手轮高 ≈ 3.0×OD
        .stemDia     = od * 0.15,     // 阀杆直径
        .handwheelDia = od * 1.8,     // 手轮直径
    };
}
```

**3. 固定拓扑的几何生成** — 形状拓扑写死，尺寸参数化:
```cpp
class ComponentTemplate {
public:
    virtual ~ComponentTemplate() = default;
    virtual std::string templateId() const = 0;
    virtual ComponentParams deriveParams(double od, double wt) const = 0;
    virtual TopoDS_Shape buildShape(const ComponentParams& p) const = 0;
};
```

#### 3.7.2 ComponentCatalog 注册表

```cpp
class ComponentCatalog {
public:
    static ComponentCatalog& instance();
    void registerTemplate(std::unique_ptr<ComponentTemplate> tpl);
    ComponentTemplate* getTemplate(const std::string& templateId) const;
    std::vector<std::string> allTemplateIds() const;
};
```

初始化时注册所有内置模板，`GeometryDeriver` 通过 templateId 查表获取对应模板:
```
用户操作: 插入闸阀  →  PipePoint.type=Valve, valveSubType="GateValve"
  → GeometryDeriver: catalog.getTemplate("GateValve")
  → deriveParams(OD=168.3, WT=7.11)
  → buildShape(params) → TopoDS_Shape
  → OcctToVsg → 3D 渲染
```

#### 3.7.3 与现有 Builder 的关系

现有的 `*Builder` 类是模板的雏形。演进路径:

| 现状 | 演进后 |
|------|--------|
| `ValveBuilder::build(point, spec)` — 生成一种阀门 | `ComponentCatalog::getTemplate("GateValve") → buildShape(params)` |
| `AccessoryBuilder::build(acc)` — 生成一种支撑 | `ComponentCatalog::getTemplate("SpringHanger") → buildShape(params)` |
| 参数硬编码在 Builder 里 | 提取为独立的 `deriveParams()` 规则 |
| 无统一注册/查询机制 | `ComponentCatalog` 单例查表 |

`GeometryDeriver` 仍为统一入口，内部从 `ComponentCatalog` 获取模板而非硬编码分发。

#### 3.7.4 方案对比

| 方案 | 适合场景 | 本项目选择 |
|------|---------|----------|
| **STEP 文件库** — 预存各种尺寸的 3D 文件 | 通用零件库(McMaster-Carr) | ✘ 组合爆炸 |
| **参数化代码模板** — 代码写死拓扑，运行时按 OD 生成 | PDS/PDMS/E3D 的标准做法 | ✔ 采用 |
| **脚本化模板** — DSL/Python 定义构件 | 需要用户自定义构件时 | 后续可扩展 |

---

## 4. 架构分层

```
┌─────────────────────────────────────────────┐
│         Layer 8: App Entry Layer            │
│  (main.cpp / QML 注册 / 命令连线 / 启动)      │
├─────────────────────────────────────────────┤
│           Layer 7: QML UI Layer             │
│  (表格编辑器/属性面板/段树/3D视口)            │
├─────────────────────────────────────────────┤
│           Layer 6: Application Layer        │
│  (文档管理/命令栈/选择管理/工作台系统)         │
├─────────────────────────────────────────────┤
│     Layer 5: Visualization Bridge           │
│  (ViewManager/VSG场景/VTK场景/拾取/LOD)      │
├─────────────────────────────────────────────┤
│     Layer 4: Pipeline Domain Engine         │
│  (几何推导/路由拓扑/约束求解/校验)            │
├─────────────────────────────────────────────┤
│     Layer 3: Document Model                 │
│  (PipePoint/PipeSpec/Segment/Route/         │
│   Accessory/Beam — 所有文档对象)             │
├─────────────────────────────────────────────┤
│     Layer 2: Geometry Kernel Wrapper        │
│  (OCCT封装: BRep/Mesh/STEP/Transform)       │
├─────────────────────────────────────────────┤
│     Layer 1: Foundation                     │
│  (类型系统/ID/数学工具/日志/配置)             │
└─────────────────────────────────────────────┘
```

Layer 3 是"文档模型层"，构件不持有几何生成逻辑；Layer 4 "管道领域引擎"负责从管点序列+PipeSpec推导几何。VTK 可视化以 Layer 5b 支线存在，由 `ViewManager` 在工作台切换时路由。

---

## 5. 构建系统

### 5.1 工具链

| 工具 | 作用 | 来源 |
|------|------|------|
| **pixi** | 包管理/环境隔离 | prefix.dev |
| **CMake 3.24+** | 构建配置 | pixi (conda-forge) |
| **Ninja** | 并行构建后端 | pixi (conda-forge) |

### 5.2 依赖管理

通用依赖通过 pixi (conda-forge) 安装。**OCCT 和 VSG 使用本地预编译库 (lib/)**，后续自建 conda recipe 迁入 pixi。Vulkan 依赖由系统提供。

| 包名 | 来源 | 备注 |
|------|------|------|
| `cmake` | pixi ✅ | 构建工具 |
| `ninja` | pixi ✅ | 构建后端 |
| `qt6-main` | pixi ✅ | Qt6 Quick/QML |
| `nlohmann_json` | pixi ✅ | JSON 序列化 |
| `gtest` | pixi ✅ | 单元测试 |
| `vtk` | pixi ✅ | CAE 分析可视化 |
| `glslang` + `spirv-tools` | pixi ✅ | SPIR-V 着色器编译 |
| Vulkan | 系统 RPM | vulkan-loader / vulkan-headers 由操作系统提供 |
| **`occt` (8.0.0)** | **本地 lib/occt/** | 后续迁入 pixi |
| **`vsg` (1.1.13)** | **本地 lib/vsg/** | 后续迁入 pixi |

### 5.3 pixi.toml

```toml
[workspace]
name = "qml-vsg-occt"
version = "0.1.0"
description = "Pipeline parametric modeling software"
channels = ["conda-forge"]
platforms = ["linux-64"]

[dependencies]
cmake = ">=3.24"
ninja = "*"
pkg-config = "*"
nlohmann_json = "*"
gtest = "*"
qt6-main = ">=6.5"
glslang = "*"
spirv-tools = "*"
vtk = ">=9.6"

# Vulkan 由系统提供 (vulkan-loader / vulkan-headers 在系统 RPM 中)
# OCCT & VSG — 使用本地 lib/, 后续迁入 pixi:
# occt = ">=8.0.0"
# vulkanscenegraph = ">=1.1.0"

[target.linux-64.dependencies]
xorg-libx11 = "*"
xorg-libxcb = "*"

[feature.dev.dependencies]
clang-format = "*"
ccache = "*"

[environments.default]
features = ["dev"]

[tasks]
configure-debug = { cmd = "cmake -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug ..." }
configure-release = { cmd = "cmake -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release ..." }
build-debug = { cmd = "cmake --build build/debug", depends-on = ["configure-debug"] }
build-release = { cmd = "cmake --build build/release", depends-on = ["configure-release"] }
test = { cmd = "ctest --test-dir build/debug --output-on-failure", depends-on = ["build-debug"] }
clean = { cmd = "rm -rf build/" }
```

### 5.4 CMakeLists.txt (根)

```cmake
cmake_minimum_required(VERSION 3.24)
project(qml-vsg-occt VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Ninja 下链接阶段串行，避免资源峰值
if(CMAKE_GENERATOR MATCHES "Ninja")
  set_property(GLOBAL PROPERTY JOB_POOLS link_pool=1)
  set(CMAKE_JOB_POOL_LINK link_pool)
endif()

# 本地预编译库 fallback (OCCT/VSG)
list(APPEND CMAKE_PREFIX_PATH
  ${PROJECT_SOURCE_DIR}/lib/occt
  ${PROJECT_SOURCE_DIR}/lib/vsg
)

# 第三方库
find_package(OpenCASCADE REQUIRED)
find_package(vsg REQUIRED)
find_package(Freetype REQUIRED)
find_package(VTK 9.6 COMPONENTS CommonCore CommonDataModel RenderingCore)
find_package(Qt6 REQUIRED COMPONENTS Quick Qml Test)
find_package(nlohmann_json REQUIRED)

# 源码
add_subdirectory(src)

# 测试
enable_testing()
add_subdirectory(tests)
```

### 5.5 CMake Target 结构

当前采用 `src/lib + src/apps` 双根拓扑，关键目标关系如下:

```
lib_base (INTERFACE)
  └─ lib_platform_occt (STATIC)
      └─ geometry (ALIAS)

pipecad_app_model (STATIC)
  └─ pipecad_app_engine (STATIC)
      └─ engine (ALIAS)

lib_platform_vtk (STATIC) + vtk_visualization (STATIC)
lib_platform_vsg (STATIC) ──depends on── engine + lib_platform_occt + vtk_visualization

lib_runtime (STATIC) ──depends on── engine + nlohmann_json
lib_framework (STATIC) ──depends on── lib_runtime + visualization(ALIAS)

pipecad_lib (INTERFACE) ──forwards── app(ALIAS -> lib_framework)
pipecad_app (STATIC) ──aggregates── model/engine/workbench/ui/resources
pipecad (EXE) ──links── pipecad_app
```

兼容目标 `foundation/geometry/model/engine/visualization/app/ui/command` 仍可链接，但仅作为迁移兼容壳；实际构建入口与新实现目录均在 `src/lib` 与 `src/apps`。

### 5.6 日常开发流程

```bash
pixi install                    # 安装所有依赖
pixi run configure-debug        # CMake配置(Debug)
pixi run build-debug            # 编译
pixi run test                   # 运行测试
pixi shell                      # 进入pixi环境
```

AI Agent 构建时需将输出重定向到临时日志文件，命令完成后再读取日志判断结果：

```bash
mkdir -p tmp_build_logs
pixi run build-debug > tmp_build_logs/build.log 2>&1
pixi run test > tmp_build_logs/test.log 2>&1
tail -50 tmp_build_logs/build.log
tail -50 tmp_build_logs/test.log
rm -rf tmp_build_logs
```

### 5.7 迁移路径

**已完成**: VTK 已从 `lib/vtk/` 迁入 pixi conda-forge (`vtk = ">=9.6"`)。

**待完成** (OCCT/VSG → pixi):

1. 编写 conda recipe (rattler-build)，从源码或预编译包构建
2. 发布到自定义 channel 或 conda-forge
3. pixi.toml 中取消注释 `occt`/`vulkanscenegraph` 依赖
4. CMakeLists.txt 删除 `lib/occt` `lib/vsg` 的 fallback
5. 删除 `lib/` 目录

---

## 6. 工作台系统

### 6.1 设计理念

类似 FreeCAD 的 Workbench 机制 — 每个工作台定义自己的工具栏、面板组合和 3D 渲染模式。切换工作台改变 UI 工具集、面板布局和视口引擎，但**文档模型(Layer 3)共享**。

### 6.2 工作台列表

| 内部类名 | 中文专业名 | 英文名 | `name()` 返回值 | 视口引擎 |
|----------|-----------|--------|-----------------|----------|
| `SpecWorkbench` | 管线规格管理 | Piping Specification | `"Specification"` | VSG (可选预览) |
| `DesignWorkbench` | 管线路由设计 | Piping Routing & Layout | `"Design"` | VSG (Vulkan) |
| `AnalysisWorkbench` | 管道应力分析 | Pipe Stress Analysis | `"Analysis"` | VTK |

**命名说明**:
- **Specification** 是管道行业通用术语（对应 ASME/EN 的 Piping Spec 概念），涵盖规范选择、参数管理、Pipe Class 定义
- **Design** 是主力建模工作台，"Piping Routing & Layout" 是行业标准说法
- **Pipe Stress Analysis** 是管道应力分析的行业标准叫法（CAESAR II、AutoPIPE、ROHR2 等均采用），管道行业做的是梁单元应力分析

### 6.3 核心抽象

```cpp
// 视口引擎类型
enum class ViewportType { Vsg, Vtk };

// 渲染模式（仅 AnalysisWorkbench 使用）
enum class RenderMode { Solid, Beam };

// Workbench (abstract, Layer 6: app/)
class Workbench {
    virtual string name() = 0;
    virtual void activate(Document&) = 0;
    virtual void deactivate(Document&) = 0;
    virtual vector<ToolbarAction> toolbarActions() = 0;
    virtual vector<string> panelIds() = 0;
    virtual ViewportType viewportType() = 0;  // VSG | VTK
};

// WorkbenchManager (Layer 6: app/)
class WorkbenchManager {
    void registerWorkbench(unique_ptr<Workbench>);
    bool switchWorkbench(string name);
    Workbench* activeWorkbench();
    vector<string> workbenchNames();
};
```

### 6.4 SpecWorkbench — 管线规格管理

管理行业规范、企业标准、自定义参数、管线等级(Pipe Class)定义。作为全系统的规则与参数源头。当前支持自定义参数，后续接入行业规范标准。

**工具栏**:

| ID | 标签 | 说明 |
|----|------|------|
| `new-spec` | 新建规格 | 创建新的 Piping Spec（管线等级） |
| `import-code` | 导入规范 | 从行业标准模板导入（预留） |
| `add-material` | 添加材料 | 管材、法兰、弯头等材料条目 |
| `add-component` | 添加元件 | 管件元件定义（壁厚、压力等级等） |
| `validate` | 校验 | 检查规格完整性与一致性 |

**面板**:
```
panelIds = { "SpecTree", "MaterialTable", "ComponentTable", "PropertyPanel" }
viewportType = Vsg  // 可选：无视口或简单预览
```

### 6.5 DesignWorkbench — 管线路由设计

基于已选规范与参数进行三维建模。这是主力建模工作台。

**工具栏**: 由 TopBar 区域显示，切换工作台时自动更换。

| ID | 标签 | 说明 |
|----|------|------|
| `new-segment` | 新建段 | 新建管线段 |
| `add-point` | 添加管点 | 添加路由控制点 |
| `measure` | 测量 | 距离/角度测量 |
| `export-step` | STEP 导出 | 几何导出 |

**面板**:
```
panelIds = { "DesignTree", "Viewport3D", "ComponentToolStrip", "ParameterPanel" }
viewportType = Vsg
```

**布局** (详见 §9.2):
```
┌────────────────────────────────────────────────────────────────────────────┐
│  TopBar (工作台切换 / 工具栏)                                               │
├─────────┬──────────────────────────────────────────┬───┬───┬──────────────┤
│         │                                          │   │   │              │
│  设计树  │                                          │ F │ A │  参数化面板   │
│ Design  │            3D 视口 (VSG)                  │ i │ c │  Parameter   │
│  Tree   │                                          │ t │ c │   Panel      │
│         │                                          │ t │ e │              │
│  ◄ 可   │                                          │ i │ s │    可 ►      │
│   折叠   │                                          │ n │ s │   折叠       │
│         │                                          │ g │ o │              │
│         │                                          │ s │ r │              │
│         │                                          │   │ y │              │
├─────────┴──────────────────────────────────────────┴───┴───┴──────────────┤
│  StatusBar                                                                │
└────────────────────────────────────────────────────────────────────────────┘
```

- **设计树 (DesignTree)**: 左侧，可折叠(←向左收起)，展示管线段、管点、管件、附属构件的层级关系
- **参数化面板 (ParameterPanel)**: 右侧固定，可折叠(→向右收起)，包含管点表格、属性编辑
- **元件插入工具条 (ComponentToolStrip)**: 贴在参数化面板内侧(靠近视口一侧)的双列图标条
- 设计树与参数化面板**独立折叠/展开**，折叠后视口自动填满

#### 6.5.1 ComponentToolStrip — 元件插入工具条

两列图标条，贴在参数化面板靠近视口一侧的边缘。每种元件一个独立的插入按钮。

**Fittings 列（管件）** — 紧贴视口侧:

| 图标 ID | 说明 |
|---------|------|
| `insert-pipe` | 插入直管段 |
| `insert-elbow` | 插入弯头 |
| `insert-tee` | 插入三通 |
| `insert-reducer` | 插入异径管（大小头） |
| `insert-valve` | 插入阀门 |
| `insert-flange` | 插入法兰 |

**Accessories 列（附属构件）** — 紧贴参数化面板内容区:

| 图标 ID | 说明 |
|---------|------|
| `insert-rigid-support` | 刚性支撑/锚固 |
| `insert-spring-hanger` | 弹簧支吊架 |
| `insert-guide` | 导向约束 |
| `insert-restraint` | 位移限位器 |
| `insert-beam` | 结构梁/支架 |

Fittings 列始终紧贴视口（插入管件频率更高），Accessories 列始终紧贴参数化面板内容区。

### 6.6 AnalysisWorkbench — 管道应力分析

在路由设计完成后进入，定义载荷、组合工况、执行分析与失效评估。使用 VTK 渲染引擎。

**工具栏**:

| ID | 标签 | 说明 |
|----|------|------|
| `toggle-render-mode` | 切换渲染模式 | 实体 ↔ 线条（梁单元） |
| `add-load` | 添加载荷 | 选择载荷类型并添加 |
| `manage-loadcase` | 工况管理 | 创建/编辑/删除工况组合 |
| `run-analysis` | 执行分析 | 运行当前工况计算（未来） |
| `show-results` | 查看结果 | 应力/位移云图（未来） |

**面板**:
```
panelIds = { "AnalysisTree", "VtkViewport", "LoadTable", "LoadCaseTable", "PropertyPanel" }
viewportType = Vtk
```

#### 6.6.1 渲染模式

| 模式 | 显示内容 | 技术实现 |
|------|---------|----------|
| **实体模式 (Solid)** | 与设计工作台相同的 3D 管件外观 | VTK `vtkPolyData` + Surface 渲染，数据来自 OCCT→VTK 网格转换 |
| **线条模式 (Beam)** | 管系中心线 + 节点（梁单元网格） | VTK `vtkPolyLine`，数据来自管路拓扑中心线提取 |

线条模式是管道应力分析的标准视图 — 每个管件对应一个或多个梁单元，节点在管件连接点上。

```
实体模式:
  OCCT TopoDS_Shape → ShapeMesher::mesh() → MeshData → vtkPolyData → vtkActor (Surface)

线条模式:
  管路拓扑 → 提取中心线端点 → vtkPoints + vtkCellArray(Lines) → vtkPolyData → vtkActor
  + 节点标记: vtkSphereSource / vtkGlyph3D → vtkActor
  + 约束/载荷符号: vtkGlyph3D 叠加显示
```

渲染模式切换通过 `vtkActor` 可见性控制，不需重建场景:
```cpp
class AnalysisWorkbench : public Workbench {
    void setRenderMode(RenderMode mode);
    RenderMode renderMode() const;
};
```

### 6.7 切换机制

- TopBar 显示工作台切换标签（Specification / Design / Analysis）
- 切换时: `WorkbenchManager::switchWorkbench()` → 旧工作台 `deactivate()` + 新工作台 `activate()` → `workbenchChanged` 信号 → QML 根据 `panelIds()` 动态加载/卸载面板，根据 `toolbarActions()` 更换工具栏
- Document 在工作台之间共享

### 6.8 工作台间数据契约

```
┌──────────────┐    PipingSpec     ┌──────────────┐   Topology+Geometry   ┌──────────────┐
│ Specification │ ───────────────→ │    Design     │ ────────────────────→ │   Analysis   │
│  Workbench    │ 材料/壁厚/压力等级│   Workbench   │ 管路拓扑+3D几何      │   Workbench  │
└──────────────┘                  └──────────────┘                       └──────────────┘
```

- **SpecWorkbench** 写入: `PipeSpec` 对象（材料、元件、压力等级表）
- **DesignWorkbench** 读取 PipeSpec，写入: `PipeSegment`/`PipePoint`/附属构件拓扑
- **AnalysisWorkbench** 读取拓扑+几何，写入: `Load`/`LoadCase`/`LoadCombination`

### 6.9 载荷与工况数据模型

载荷(Load)、基本工况(LoadCase)、组合工况(LoadCombination) 均为 `DocumentObject` 子类，完整纳入文档基础设施（UUID、属性系统、Undo/Redo、序列化、依赖图）。

#### 6.9.1 Load 基类

```cpp
class Load : public DocumentObject {
public:
    virtual std::string loadType() const = 0;          // 类型标识，用于序列化/UI
    virtual std::vector<UUID> affectedObjects() const;  // 作用到哪些管段/管点
    void addAffectedObject(const UUID& id);
    void removeAffectedObject(const UUID& id);
protected:
    std::vector<UUID> affectedObjectIds_;
};
```

#### 6.9.2 具体载荷类型

| 子类 | loadType | 特有参数 | 说明 |
|------|----------|---------|------|
| `DeadWeightLoad` | `"DeadWeight"` | _(无)_ | 自重由管件材料密度+壁厚自动计算 |
| `ThermalLoad` | `"Thermal"` | `installTemp`, `operatingTemp` (°C) | 热膨胀载荷 |
| `PressureLoad` | `"Pressure"` | `pressure` (MPa), `isExternal` (bool) | 内压/外压 |
| `WindLoad` | `"Wind"` | `speed` (m/s), `direction` (Vec3) | 风载荷 |
| `SeismicLoad` | `"Seismic"` | `acceleration` (g), `direction` (Vec3) | 地震载荷 |
| `DisplacementLoad` | `"Displacement"` | `translation` (mm), `rotation` (deg) (Vec3) | 锚点强制位移 |
| `UserDefinedLoad` | `"UserDefined"` | `force` (N), `moment` (N·mm) (Vec3) | 自定义集中力/力矩 |

#### 6.9.3 LoadCase（基本工况）

一个基本工况 = 一种**物理运行状态**下的载荷集合。每个基本工况对应求解器的一次独立运算。

```cpp
struct LoadEntry {
    UUID loadId;      // 指向 Load 文档对象
    double factor;    // 组合系数（通常 1.0）
};

class LoadCase : public DocumentObject {
    std::string caseName_;              // "W", "T1", "P1" 等
    std::vector<LoadEntry> entries_;
};
```

**示例**:

| 基本工况 | 包含载荷 |
|---------|---------|
| W（自重） | DeadWeightLoad × 1.0 |
| T1（热态1） | ThermalLoad(150°C) × 1.0 |
| P1（内压） | PressureLoad(2.5MPa) × 1.0 |
| WN（北风） | WindLoad(+X, 30m/s) × 1.0 |
| EQ（地震） | SeismicLoad(0.3g, +Z) × 1.0 |

#### 6.9.4 LoadCombination（组合工况）

组合工况 = 按**规范公式**将多个基本工况的结果进行代数运算，用于规范校核。

```cpp
enum class CombineMethod {
    Algebraic,   // 代数叠加: Σ(factor × result)
    Absolute,    // 绝对值叠加: Σ|factor × result|（保守）
    SRSS,        // 平方和开方: √Σ(result²)（不相关载荷，如地震）
    Envelope     // 包络: max/min across cases
};

enum class StressCategory {
    Sustained,    // 持续应力 (SUS) — 自重+内压，不得超过 Sh
    Expansion,    // 热膨胀应力 (EXP) — 热态-冷态，不得超过 Sa
    Occasional,   // 偶然应力 (OCC) — 持续+风/地震，不得超过 1.33Sh
    Operating,    // 操作工况 (OPE) — 检查位移和力
    Hydrotest     // 水压试验 (HYD)
};

struct CaseEntry {
    UUID caseId;     // 指向 LoadCase
    double factor;   // 组合系数
};

class LoadCombination : public DocumentObject {
    std::string comboName_;
    StressCategory category_;
    CombineMethod method_;
    std::vector<CaseEntry> caseEntries_;
};
```

**示例（B31.3 典型配置）**:

| 组合工况 | 类别 | 方法 | 公式 |
|---------|------|------|------|
| SUS | Sustained | Algebraic | W×1.0 + P1×1.0 |
| EXP | Expansion | Algebraic | T1×1.0 |
| OPE | Operating | Algebraic | W×1.0 + T1×1.0 + P1×1.0 |
| OCC1 | Occasional | Algebraic | W×1.0 + P1×1.0 + WN×1.0 |
| OCC2 | Occasional | SRSS | W×1.0 + P1×1.0 + EQ×1.0 |

#### 6.9.5 两层设计的理由

| 方面 | 说明 |
|------|------|
| **依赖关系** | 严格 DAG: LoadCombination → LoadCase → Load → 管段/管点 |
| **求解分离** | 基本工况 = 独立求解任务，组合工况 = 后处理代数运算 |
| **规范校核** | `StressCategory` 自然属于 Combination 层 |
| **组合方法** | Algebraic/SRSS/Envelope 只在 Combination 层，语义清晰 |
| **复用** | 一个 LoadCase 可被多个 Combination 引用 |

#### 6.9.6 AnalysisWorkbench 设计树结构

```
📁 载荷 (Loads)
  ├── 🏋 自重 (DeadWeight)
  ├── 🌡 热载荷-操作 (Thermal 150°C)
  ├── 📐 内压 (Pressure 2.5MPa)
  ├── 💨 风载荷-北风 (Wind +X)
  └── 🌊 地震 (Seismic 0.3g)

📁 基本工况 (Load Cases)
  ├── W  — 自重
  ├── T1 — 热态操作
  ├── P1 — 内压
  ├── WN — 北风
  └── EQ — 地震

📁 组合工况 (Combinations)
  ├── SUS — 持续应力 [W+P1]
  ├── EXP — 热膨胀应力 [T1]
  ├── OPE — 操作工况 [W+T1+P1]
  ├── OCC1 — 偶然-风 [W+P1+WN]
  └── OCC2 — 偶然-地震 [W+P1+EQ] (SRSS)
```

#### 6.9.7 依赖图集成

全部纳入现有 `DependencyGraph`，支持级联重算：

| 触发变更 | 标脏范围 |
|---------|---------|
| Load 参数修改 | 引用该 Load 的所有 LoadCase |
| LoadCase 修改 | 引用该 LoadCase 的所有 LoadCombination |
| 管段/管点坐标修改 | 作用于此管段的所有 Load |
| PipeSpec 修改 | 影响的管点 → 关联的 Load |

---

## 7. 命令栈与重算机制

### 7.1 设计原则

- **单步操作=单命令**: 每个用户编辑动作封装为一个 `Command`，对应一次 Undo/Redo 步骤
- **命令完成后统一重算**: `CommandStack` 在执行、撤销、重做后发射 affectedIds，主线程统一触发脏传播与重算
- **支持宏命令**: 复合操作通过 `MacroCommand` / `CommandStack::openMacro()` 聚合为单个历史项
- **单线程**: 所有文档变更与重算均在主线程执行
- **会话级**: 命令历史仅在内存中，关闭应用后丢失；保存状态由 `CommandStack::markClean()` 追踪

### 7.2 核心组件

```
┌──────────────────────────────────────────────┐
│       CommandStack (Layer 6: app/)           │
│  execute() / undo() / redo() / openMacro()   │
│  commandCompleted / commandUndone /          │
│  commandRedone / sceneRemoveRequested        │
├──────────────────────────────────────────────┤
│   CommandRegistry + CommandContext           │
│  工厂注册 / 序列化 / 运行时依赖注入          │
├──────────────────────────────────────────────┤
│      DependencyGraph (Layer 6: app/)         │
│  markDirty() / collectDirty() / clearDirty() │
├──────────────────────────────────────────────┤
│     RecomputeEngine (Layer 4: engine/)       │
│  收集脏对象 → 拓扑排序 → 批量重算Shape        │
│  → SceneManager/VtkSceneManager 更新场景      │
└──────────────────────────────────────────────┘
```

### 7.3 命令执行生命周期

```
1. UI 创建 `SetPropertyCommand` / `BatchSetPropertyCommand` / 结构命令
2. `Application::createCommandContext()` 构建 `CommandContext{document, dependencyGraph, topologyManager}`
3. `CommandStack::execute(cmd, ctx)` 执行命令，命令返回 `affectedIds` / `deletedIds`
4. `commandCompleted` 信号触发主线程重算处理器
5. 重算处理器执行：
  ├─ `DependencyGraph::markDirty(affectedIds)`
  ├─ `collectDirty()` 收集受影响对象
  ├─ `RecomputeEngine::recompute(dirtyIds)` 批量重算几何
  ├─ `SceneManager` / `VtkSceneManager` 更新节点
  └─ `clearDirty()` 清空脏标记
```

### 7.4 Undo/Redo

```
undo(): `CommandStack::undo(ctx)` → `commandUndone(affectedIds)` → 标脏 → recompute()
redo(): `CommandStack::redo(ctx)` → `commandRedone(affectedIds)` → 标脏 → recompute()
```

### 7.5 依赖关系

| 触发变更 | 标脏范围 |
|---------|---------|
| PipeSpec 字段修改 | 所有引用该 PipeSpec 的 PipePoint |
| PipePoint 坐标修改 | 前后相邻 PipePoint |
| PipePoint 类型修改 | 自身 + 前后相邻 PipePoint |
| PipePoint 增删 | 所在 Segment 重组 + 关联 Accessory/Beam |
| Accessory 参数修改 | 仅自身 |

---

## 8. 目录结构（T77 基线）

```
qml-vsg-occt/
├── CMakeLists.txt
├── pixi.toml
├── .github/                             # AI 辅助开发配置
│   ├── instructions/                    # VS Code Copilot 自动注入编码规范
│   └── prompts/                         # 任务启动 slash 命令
├── src/
│   ├── CMakeLists.txt
│   ├── lib/                            # 唯一架构构建入口
│   │   ├── base/
│   │   │   ├── foundation/
│   │   │   └── baseMod/
│   │   ├── platform/
│   │   │   ├── occt/geometry/
│   │   │   ├── vsg/visualization/
│   │   │   └── vtk/vtk-visualization/
│   │   ├── runtime/
│   │   │   ├── app/
│   │   │   ├── command/
│   │   │   ├── task/
│   │   │   └── runtimeMod/
│   │   └── framework/
│   │       ├── app/
│   │       └── frameworkMod/
│   ├── apps/                           # 唯一业务构建入口
│   │   ├── CMakeLists.txt              # PIPECAD_APP_LIST + 注册模板
│   │   └── pipecad/
│   │       ├── model/
│   │       ├── engine/
│   │       ├── workbench/
│   │       ├── ui/
│   │       ├── resources/
│   │       └── main.cpp
│   ├── foundation/                     # 迁移期历史镜像（非构建入口）
│   ├── geometry/                       # 迁移期历史镜像（非构建入口）
│   ├── model/                          # 迁移期历史镜像（非构建入口）
│   ├── engine/                         # 迁移期历史镜像（非构建入口）
│   ├── visualization/                  # 迁移期历史镜像（非构建入口）
│   ├── vtk-visualization/              # 迁移期历史镜像（非构建入口）
│   ├── app/                            # 迁移期历史镜像（非构建入口）
│   ├── command/                        # 迁移期历史镜像（非构建入口）
│   ├── ui/                             # 迁移期历史镜像（非构建入口）
│   └── main.cpp                        # 旧入口（保留，非构建入口）
├── docs/                               # 项目文档
│   ├── architecture.md                 # 本文件
│   ├── tasks/                          # 任务状态与交接
│   └── archive/                        # 已完成阶段归档
├── ui/main.qml                         # QML 入口
└── tests/                              # 全量测试（当前基线 46）
```

### 8.1 应用入口（T77）

1. 可执行入口为 `src/apps/pipecad/main.cpp`，构建产物为 `build/<type>/src/apps/pipecad/pipecad`
2. 启动阶段创建 `QGuiApplication`，注册 QML 类型并初始化 `Application`
3. 通过 `CommandRegistry` 注册命令工厂，连线 `CommandStack` 与异步重算回调
4. 配置 `ViewManager`，在 Design 工作台使用 VSG，在 Analysis 工作台切换 VTK
5. 加载 `ui/main.qml` 并进入事件循环

---

## 9. UI 设计

### 9.1 风格定位

现代 SaaS 风格(类似 Fusion 360 / Onshape) — 浅色主题, 简洁干净, 3D 视口为主体, 面板可收缩, 低密度留白。

### 9.2 主窗口布局

```
┌──────────────────────────────────────────────────────────────┐
│  [≡] 工程名称               [工具栏图标组]             [⚙]  │  ← TopBar (48px)
├────────┬─────────────────────────────┬───────────────────────┤
│        │                             │                       │
│ 结构树  │       3D 视口 (VSG)         │   管点表格 (可编辑)    │
│(可收缩) │                             │   (可收缩)            │
│        │                             │                       │
│ 240px  │     ← 视口主导最大面积 →     │   ────────────────    │
│        │                             │   属性面板 (动态)      │
│        │                             │   320px               │
├────────┴─────────────────────────────┴───────────────────────┤
│  A06 Bend (3500.0, 1200.0, 0.0)    [鼠标3D坐标]      [缩放] │  ← StatusBar
└──────────────────────────────────────────────────────────────┘
```

- **视口主导**: 3D 视口占中央最大面积
- **左右分割**: 左=3D视口, 右=管点表格+属性面板, 可拖动分割条
- **左侧结构树**: 可收缩, Route/Segment/PipeSpec/附属构件/梁树形展示
- **右侧上部**: 管点表格 (Name|X|Y|Z|Type|PipeSpec)，可直接编辑单元格
- **右侧下部**: 属性面板，根据选中对象类型动态显示不同属性
- **Bend 子点**: N/M/F 行灰色背景只读，标记(auto)

### 9.3 属性面板 (管点参数化面板)

右侧下部属性面板。根据选中对象类型动态渲染不同字段分组。

**触发方式**:
- 左键选中管点(3D/表格/树) → 面板内容切换
- 右键菜单→"查看管点属性" → 面板已展开时高亮闪烁动画提醒；面板已收缩时自动展开

**面板结构** (以 Bend 管点 A06 为例):

```
┌─ PropertyPanel ──────────────────┐
│  🔵 A06  Bend                    │  ← 标题: 名称+类型图标
│  ─────────────────────────────── │
│  ▸ 坐标                          │  ← 分组1, 可折叠
│    X  [  3500.0  ]  mm          │
│    Y  [  1200.0  ]  mm          │
│    Z  [     0.0  ]  mm          │
│  ▸ 管线特性                      │  ← 分组2
│    PipeSpec  [ 6inch-Sch40  ▾]  │
│    OD           168.3 mm        │    只读(来自PipeSpec)
│    壁厚           7.11 mm        │
│    材料        A106-B            │
│  ▸ Bend 参数                     │  ← 分组3, 仅Bend可见
│    弯曲倍数  [ 1.5D  ▾]         │
│    弯曲半径      252.45 mm      │    只读(=OD×倍数)
│    弯曲角度       90.0°          │    只读
│  ▸ 计算点                        │  ← 分组4, 仅Bend可见
│    A06N  (3247.6, 1200.0, 0.0)  │    只读
│    A06M  (3320.8, 1072.4, 0.0)  │
│    A06F  (3500.0,  947.6, 0.0)  │
│  ▸ 关联                          │  ← 分组5
│    所在段    Segment A           │    点击跳转
│    前管点    A05 (Run)           │
│    后管点    A07 (Run)           │
│    附属构件  Flange-01           │
├──────────────────────────────────┤
│  [🔒 只读模式] / [✏️ 编辑模式]  │  ← 模式切换按钮
└──────────────────────────────────┘
```

**各管点类型分组差异**:

| 类型 | 坐标 | 管线特性 | 类型参数 | 计算点 | 关联 |
|------|------|---------|---------|--------|------|
| Run | ✅编辑 | ✅PipeSpec选择 | _(无)_ | _(无)_ | ✅ |
| Bend | ✅编辑(交点) | ✅PipeSpec选择 | 弯曲倍数(下拉) | N/M/F只读 | ✅ |
| Reducer | ✅编辑 | ✅PipeSpec选择 | _(无)_ | 大端/小端OD只读 | ✅ |
| Tee | ✅编辑 | ✅PipeSpec选择 | 支管段引用(只读) | _(无)_ | ✅+分支段 |
| Valve | ✅编辑 | ✅PipeSpec选择 | 阀门类型(下拉)+参数 | _(无)_ | ✅ |
| FlexJoint | ✅编辑 | ✅PipeSpec选择 | 柔性参数 | _(无)_ | ✅ |

**编辑规则**:
- 参数化面板最下方有一个 **模式切换按钮**，在“🔒 只读模式”和“✏️ 编辑模式”之间切换
- **编辑模式**: 可编辑字段显示输入框/下拉菜单，修改即走命令（SetProperty/BatchSetProperty → CommandStack → recompute）
- **只读模式**: 所有字段均为只读显示，无输入框，仅供查看
- 右键菜单“修改”自动切换到编辑模式，“查看”自动切换到只读模式
- 默认打开时为编辑模式
- 只读字段(如计算值、自动推导值)在两种模式下始终显示为灰色 #888888 无边框，带(auto)标签
- 关联项点击跳转联动，不受模式影响

### 9.4 视觉规范

| 属性 | 值 |
|------|---|
| 主背景 | #F5F5F5 |
| 面板背景 | #FFFFFF (白色卡片) |
| 3D视口背景 | 渐变 #E8E8E8 → #D0D0D0 |
| 强调色 | #0078D4 (蓝色) |
| 文字主色 | #333333 |
| 文字次色 | #888888 (只读/计算值) |
| 分割线 | #E0E0E0 |
| 圆角 | 6px |
| 表格行高 | 36px |
| 图标 | 线性风格, 1.5px 笔触 |

### 9.5 交互联动

| 操作 | 响应 |
|------|------|
| 树节点选中 | 表格滚动到对应行 + 3D 视口高亮(管点:红色+, 管段:几何体高亮) |
| 表格行选中 | 树展开到对应节点 + 3D 视口高亮 |
| 3D 视口拾取(管点) | 树+表格同步选中该管点行，属性面板显示管点属性 |
| 3D 视口拾取(管段) | 树+表格同步选中该管段两端管点，属性面板显示管段信息 |
| 表格编辑坐标 | 命令执行→recompute→3D 视口实时更新 |
| 属性面板修改 PipeSpec | 命令执行→recompute→所有引用管点 3D 更新 |

### 9.6 TopBar / StatusBar

- **TopBar**: 左侧工作台切换标签(Specification|Design|Analysis)，当前活跃高亮。右侧工具栏图标组由活跃工作台的 `toolbarActions()` 决定，切换工作台时工具栏自动更换。文件操作(新建/打开/保存/撤销/重做)始终可见。
- **StatusBar**: 分三个区域——
  - **左区 (选中对象信息)**: 显示当前选中管件的 `名称 类型 (X, Y, Z)`，例如 `A06 Bend (3500.0, 1200.0, 0.0)`；无选中时显示 `管点数/段数/路由数` 统计信息
  - **中区 (鼠标3D坐标)**: 实时显示鼠标在3D视口中悬停位置的世界坐标
  - **右区 (缩放比例)**: 当前视口缩放级别

---

## 10. 3D 视口

### 10.1 鼠标交互

| 操作 | 功能 |
|------|------|
| **滚轮滚动** | 缩放 (以鼠标位置为中心) |
| **滚轮按下拖动** | 平移视图 (Pan) |
| **左键点击** | 选择对象 (管点选择 或 管段选择，见下方规则) |
| **左键拖动框选** | 框选 (方向决定模式，见 §10.1.2) |
| **Ctrl + 左键拖动** | 轨道旋转 (Orbit) |
| **右键点击** | 弹出上下文菜单 |

#### 10.1.1 选择模式—管点选择 vs 管段选择

左键点击拾取后，`PickHandler` 根据点击位置与最近管点的距离决定选择类型:

```
点击位置 P → LineSegmentIntersector → 命中几何体
  → 反查所属管件，找到该管件两端管点
  → 计算 P 到最近管点的屏幕距离 d
  → if d < 阈值 (e.g. 20px):  管点选择
     else:                     管段选择
```

| 选择类型 | 触发条件 | 高亮表现 | SelectionManager 存储 |
|---------|---------|---------|--------------------|
| **管点选择** | 点击位置距管点 < 阈值 | 管点处显示 **红色十字标记 (+)** | `{type: Point, id: pointUUID}` |
| **管段选择** | 点击位置距管点 ≥ 阈值 | 整个管段几何体高亮（发光/边框） | `{type: Segment, startId, endId}` |

**管点选择的红色十字标记**:
- 在管点 3D 坐标处叠加一个十字形 Glyph（两条正交短线），颜色 #FF0000
- 尺寸与管径成比例（约 0.8×OD），保证不同管径下视觉协调
- 始终面向相机（billboard），任何角度都清晰可见

**弯头管点的选择**: 弯头上的 4 个管点(A06/N/M/F)均可独立拾取。点击弯头几何体时，找最近的弯头管点；距离小于阈值则选中该管点，否则选中整个弯头管段。

```cpp
// SelectionManager 选择模型
enum class SelectionType { Point, Segment };

struct Selection {
    SelectionType type;
    UUID primaryId;     // 管点选择: 管点ID; 管段选择: 起始管点ID
    UUID secondaryId;   // 管段选择: 终止管点ID; 管点选择: 无效
};
```

#### 10.1.2 框选模式—方向决定选择逻辑

左键拖动（非 Ctrl）形成矩形框选区域，**拖动方向决定选择严格程度**：

| 拖动方向 | 框线样式 | 选择逻辑 | 行业术语 |
|---------|---------|---------|---------|
| **左上→右下**（正向） | 实线框 | 对象必须**完全在框内**才被选中 | Window 选择 |
| **右下→左上**（反向） | 虚线框 | 对象**部分在框内**即被选中 | Crossing 选择 |

```
正向框选 (Window):              反向框选 (Crossing):
┌─────────────┐                ╭┄┄┄┄┄┄┄┄┄┄┄┄┄╮
│  ○ 选中     │                ┆  ○ 选中     ┆
│  ● 选中     │  实线框        ┆  ●━━━选中   ┆  虚线框
│         ◆━━━│━ 不选中        ┆         ◆━━━┆━ 选中(部分在框内)
└─────────────┘                ╰┄┄┄┄┄┄┄┄┄┄┄┄┄╯
```

**判定规则**:
- Window: 管点的屏幕投影在框内 → 管点选中；管段两端管点均在框内 → 管段选中
- Crossing: 管点的屏幕投影在框内 → 管点选中；管段几何体的屏幕投影与框**相交或被包含** → 管段选中
- 框选结果为多选，所有命中对象同时高亮（管点显示红色+，管段显示高亮边框）
- 框选追加: Shift + 框选追加到已有选择集；不按 Shift 则替换

这与 AutoCAD/SolidWorks 的框选行为一致，管道设计师无学习成本。

### 10.2 视图快捷方式

3D 视口右上角视图切换按钮组，点击后相机平滑过渡(200ms ease-in-out):

| 视图 | 相机方向 | 快捷键 |
|------|---------|--------|
| 正视图 (Front) | -Y → +Y (XZ平面) | Numpad 1 |
| 右视图 (Right) | +X → -X (YZ平面) | Numpad 3 |
| 俯视图 (Top) | -Z → +Z (XY平面) | Numpad 7 |
| 轴测图 (Isometric) | (-1,-1,-1)→原点, 45°仰角 | Numpad 0 |
| Fit All | 自动包围全部对象 | F |

实现: `CameraController::setViewPreset(ViewPreset)` → 计算目标 LookAt → `vsg::LookAt` 插值动画

### 10.3 右键上下文菜单

选中管件或管点后右键弹出，菜单项：

| 菜单项 | 快捷键 | 可用条件 | 行为 |
|--------|--------|---------|------|
| **修改** | Enter | 有选中对象 | 参数化面板若已折叠则自动展开，属性面板进入编辑模式并聚焦到选中对象 |
| **查看** | — | 有选中对象 | 参数化面板若已折叠则自动展开，属性面板以只读模式显示选中对象信息 |
| **删除** | Delete | 有选中对象 | 弹出确认对话框，确认后执行删除命令 |

**参数化面板弹出逻辑**:
- 选择"修改"或"查看"时，如果右侧参数化面板已折叠 → 自动展开
- 如果参数化面板已展开 → 属性面板区域闪烁动画提醒用户注意
- 无选中对象时，三个菜单项均灰色禁用

### 10.4 ViewManager (视图管理)

`ViewManager` 是 Layer 5 的统一门面，协调 VSG 与 VTK 双视口的渲染与交互，上层 QML/工具栏只与 `ViewManager` 交互，无需感知底层渲染引擎。

**架构定位**:
```
WorkbenchManager ──切换通知──► ViewManager ──委派──► VSG Viewport (SceneManager + CameraController + SceneFurniture)
                                   │                  VTK Viewport (VtkSceneManager + VtkRenderer)
                                   │
                              QML / ToolBar
```

**管理状态**:
```cpp
class ViewManager {
public:
    // === 视口路由 ===
    enum class ActiveViewport { VSG, VTK };
    void setActiveViewport(ActiveViewport vp);  // 工作台切换时调用
    ActiveViewport activeViewport() const;

    // === 相机控制 (转发给当前活跃视口) ===
    void fitAll();
    void setViewPreset(ViewPreset preset);      // Front/Right/Top/Iso
    void saveViewState(const std::string& workbenchId);
    void restoreViewState(const std::string& workbenchId);

    // === 渲染模式 ===
    enum class RenderMode { Solid, Wireframe, SolidWithEdges, Beam };
    void setRenderMode(RenderMode mode);
    RenderMode renderMode() const;

    // === 可见性控制 ===
    enum class Category { PipePoints, Segments, Accessories, Supports, Beams,
                          Annotations, LoadArrows, StressContour };
    void setCategoryVisible(Category cat, bool visible);
    bool isCategoryVisible(Category cat) const;

    // === 场景装饰 ===
    void setGridVisible(bool visible);
    void setTriadVisible(bool visible);
    void setAnnotationsVisible(bool visible);

    // === LOD ===
    enum class LodLevel { Draft, Normal, Fine };
    void setLodLevel(LodLevel level);
    LodLevel lodLevel() const;

    // === StatusBar 数据 ===
    gp_Pnt currentMouseWorldPos() const;        // 鼠标 3D 坐标

    // === 截图 ===
    bool captureImage(const std::string& path); // 离屏渲染截图

private:
    ActiveViewport activeVp_ = ActiveViewport::VSG;
    RenderMode renderMode_ = RenderMode::Solid;
    LodLevel lodLevel_ = LodLevel::Normal;
    std::map<Category, bool> visibility_;
    std::map<std::string, CameraState> viewStateCache_;  // workbenchId → 相机状态

    // 非拥有指针
    SceneManager*      vsgScene_   = nullptr;
    CameraController*  vsgCamera_  = nullptr;
    SceneFurniture*    vsgFurni_   = nullptr;
    VtkSceneManager*   vtkScene_   = nullptr;
};
```

**工作台切换流程**:
```
WorkbenchManager::switchTo("Analysis")
  → ViewManager::saveViewState("Design")       // 保存当前相机
  → ViewManager::setActiveViewport(VTK)         // 切换活跃视口
  → ViewManager::restoreViewState("Analysis")   // 恢复目标相机
  → ViewManager::setRenderMode(Beam)            // Analysis 默认 Beam 模式
  → QML 层切换 Viewport3D.qml ↔ VtkViewport.qml
```

**与各底层类的分工**:

| 职责 | 负责类 | ViewManager 角色 |
|------|--------|------------------|
| 场景图节点增删 | `SceneManager` / `VtkSceneManager` | 不涉及 |
| 相机数学(lookAt/投影) | `CameraController` | 转发 fitAll/setViewPreset |
| 坐标轴/网格/背景 | `SceneFurniture` | 转发显隐开关 |
| LOD 网格精度 | `LodStrategy` | 设置全局精度档位 |
| 3D 拾取 | `PickHandler` | 不涉及 |
| VSG↔VTK 视口切换 | — | **ViewManager 独有** |
| 渲染模式/可见性 | — | **ViewManager 独有** |
| 视图状态缓存 | — | **ViewManager 独有** |
| 截图/离屏渲染 | — | **ViewManager 独有** |

### 10.5 场景基础设施

| 元素 | 描述 | 实现 |
|------|------|------|
| 坐标轴指示器 | 视口左下角 XYZ 小坐标轴 | vsg::MatrixTransform, 固定屏幕位置 |
| 地面网格 | XY 平面参考网格, 可开关 | vsg::Group + LineStrip |
| 渐变背景 | #E8E8E8 → #D0D0D0 | vsg::ClearValue / 全屏四边形 |
| Fit All | 自动调整相机包围全部对象 | ComputeBounds → 调整 vsg::LookAt |

---

## 11. 应用约定

- **单文档模式**: 一次只打开一个工程文件
- **窗口标题**: `{工程名} - PipeCAD` / `{工程名}* - PipeCAD` (未保存修改加 *)
- **键盘快捷键**: Ctrl+S 保存 / Ctrl+Z 撤销 / Ctrl+Y 重做 / Ctrl+N 新建 / Ctrl+O 打开 / Delete 删除选中

---

## 12. 实施步骤

### Phase 1: 基础设施 (Steps 1-3)

**Step 1 — 构建系统**
- `pixi.toml`: 声明所有依赖、平台、构建任务
- `CMakeLists.txt`(根): find_package, C++17, AUTOMOC/AUTORCC
- `src/CMakeLists.txt`: 各层子目录 target
- `tests/CMakeLists.txt`: GTest 测试
- `.gitignore`: build/, .pixi/
- 验证: `pixi install` + `pixi run configure-debug` + `pixi run build-debug` 通过

**Step 2 — Foundation 层**
- `Types.h`: UUID, Variant(double|int|string|enum), 单位枚举
- `Math.h`: 角度/弧度, 向量运算, 两线交点计算

**Step 3 — Geometry Kernel Wrapper**
- `ShapeBuilder`: BRepPrimAPI_MakeCylinder/MakeTorus/MakeCone, BRepOffsetAPI_MakePipeShell
- `ShapeMesher`: BRepMesh_IncrementalMesh → Poly_Triangulation
- `BooleanOps`: BRepAlgoAPI_Cut/Fuse
- `StepIO`: STEPControl_Reader/Writer
- `ShapeTransform`: gp_Trsf + BRepBuilderAPI_Transform

### Phase 2: 文档模型 (Steps 4-5, 依赖 Phase 1)

**Step 4 — 核心文档对象**
- `DocumentObject` / `SpatialObject` / `PropertyObject` / `ContainerObject`
- `PipePoint` / `PipeSpec` / `ProjectConfig` / `Segment` / `Route`

**Step 5 — 附属对象**
- `Accessory` 基类 + FixedPoint / Support / Flange / Gasket / SealRing
- `Beam`: 双端管点引用 + 截面参数

### Phase 3: 管道领域引擎 (Steps 6-9, 依赖 Phase 2)

**Step 6 — 弯头几何计算器 (核心算法)**
- `BendCalculator`: (A05, A06交点, A07, OD, bendMultiplier) → (N/M/F坐标 + 弯弧中心 + 起止角)

**Step 7 — 各类型几何生成器**
- RunBuilder / ReducerBuilder / TeeBuilder / ValveBuilder / FlexJointBuilder / BeamBuilder / AccessoryBuilder
- `GeometryDeriver`: 统一入口, 按管点类型分发

**Step 8 — 拓扑管理**
- `TopologyManager`: 段间关系, 管点增删时维护一致性

**Step 9 — 约束求解与校验**
- `ConstraintSolver`: 端口口径匹配, Bend 角度约束
- `PipelineValidator`: 干涉检测, 未连接端口警告

### Phase 4: 可视化桥接 (Steps 10-12, 可与 Phase 3 并行)

**Step 10 — OCCT→VSG 网格转换**
- `OcctToVsg`: TopoDS_Shape → BRepMesh → Poly_Triangulation → vsg::VertexIndexDraw

**Step 11 — 场景管理**
- `SceneManager`: 文档对象 ID→VSG 节点映射, 增量更新
- `PipePointNode` / `ComponentNode` / `LodStrategy`

**Step 12 — 3D 交互**
- `CameraController`: 自定义鼠标映射, Fit All, 视图预设
- `PickHandler`: 拾取→反查文档对象→SelectionManager
- `SelectionHighlight` / `SceneFurniture`

### Phase 5: QML 集成 (Steps 13-15, 依赖 Phase 4)

**Step 13 — VSG-QML 桥接**
- `VsgQuickItem`: QQuickFramebufferObject, VSG 离屏渲染→纹理→QML
- 鼠标/键盘事件转发

**Step 14 — 应用层**
- `Document` / `DependencyGraph` / `RecomputeEngine`
- `Command` / `CommandStack` / `CommandRegistry` / `CommandContext`
- `SelectionManager` / `Workbench` + `WorkbenchManager`
- `DesignWorkbench` (取代 CadWorkbench): 设计树+ComponentToolStrip+参数化面板
- `WorkbenchController`: C++→QML 桥接

**Step 15 — QML UI (DesignWorkbench)**
- `DesignTree.qml`: 左侧可折叠设计树
- `ParameterPanel.qml`: 右侧可折叠参数化面板(含管点表格+属性面板)
- `ComponentToolStrip.qml`: 双列元件插入图标条
- `Viewport3D.qml` / `ContextMenu.qml`

### Phase 6: 数据交换 (Steps 16-17, 依赖 Phase 3+5)

**Step 16 — STEP 导入/导出**
- 导出: 遍历管点→GeometryDeriver→STEPControl_Writer
- 侧车 JSON: 管点序列/PipeSpec/段结构/附属构件

**Step 17 — 管道校验**
- 干涉检测, 规格校验, 未连接端口

### Phase 7: 二期扩展 — 多工作台 + 载荷分析 (Steps 18-22, 依赖 Phase 5+6)

**Step 18 — SpecWorkbench (管线规格管理)**
- `SpecWorkbench.h/cpp`: 工具栏(新建规格/导入规范/添加材料/添加元件/校验)
- `SpecTree.qml` / `MaterialTable.qml` / `ComponentTable.qml`
- 自定义参数管理，预留行业规范接入接口

**Step 19 — 载荷与工况数据模型**
- `Load.h/cpp` 基类 + 7个子类(DeadWeight/Thermal/Pressure/Wind/Seismic/Displacement/UserDefined)
- `LoadCase.h/cpp`: 基本工况
- `LoadCombination.h/cpp`: 组合工况(CombineMethod + StressCategory)
- 序列化扩展: ProjectSerializer 支持载荷/工况的 JSON 读写
- 依赖图扩展: Load→LoadCase→LoadCombination 链

**Step 20 — VTK 可视化桥接**
- `OcctToVtk.h/cpp`: OCCT TopoDS_Shape → vtkPolyData
- `BeamMeshBuilder.h/cpp`: 管路拓扑中心线 → 梁单元线模型
- `VtkSceneManager.h/cpp`: VTK 场景管理, 实体/线条双模式 Actor 管理
- `VtkViewport.h/cpp`: VTK 渲染器 + QML 集成 (QVTKOpenGLNativeWidget 或 QQuickFramebufferObject)

**Step 21 — AnalysisWorkbench (管道应力分析)**
- `AnalysisWorkbench.h/cpp`: 工具栏(渲染模式切换/添加载荷/工况管理)
- `RenderMode::Solid` ↔ `RenderMode::Beam` 切换
- `AnalysisTree.qml`: 三级树结构(载荷/基本工况/组合工况)
- `LoadTable.qml` / `LoadCaseTable.qml` / `VtkViewport.qml`

**Step 22 — 集成验证**
- Spec→Design→Analysis 全流程端到端测试
- 载荷/工况 Undo/Redo 和 JSON round-trip 验证
- VTK 实体/线条模式切换验证

### Phase 8: 三期扩展 — 命令模式 (T0-T10, 已完成)

**Step T0-T3 — 命令基础设施**
- `Variant` 扩展 `bool` / `Vec3`
- `DocumentObject` 增加 `setProperty()` / `getProperty()` 虚接口
- `Command` / `MacroCommand` / `PropertyApplier` / `CommandStack`

**Step T4-T7 — 属性命令与 UI 迁移**
- `SetPropertyCommand` / `BatchSetPropertyCommand`
- `CommandRegistry` 统一工厂与 JSON 序列化
- `Application` 与 `main.cpp` 接入命令信号
- `AppController` / `PipePointTableModel` / `PipeSpecModel` 改为通过命令驱动编辑

**Step T8-T10 — 结构命令与清理**
- `CreatePipePointCommand` / `DeletePipePointCommand` / `InsertComponentCommand`
- 删除 `TransactionManager`
- `src/command/` 源文件并入 `app` 静态库，消除循环依赖

---

## 13. 关键技术文件

### OCCT (Layer 2)
- `BRepPrimAPI_MakeCylinder` / `MakeTorus` / `MakeCone` → 管/弯头/异径
- `BRepAlgoAPI_Cut` / `Fuse` → 管壳/三通
- `BRepOffsetAPI_MakePipeShell` → 梁扫掠
- `BRepMesh_IncrementalMesh` + `Poly_Triangulation` + `BRep_Tool::Triangulation()` → 三角化
- `STEPControl_Writer` / `Reader` → STEP
- `gp_Trsf` + `BRepBuilderAPI_Transform` → 变换

### VSG (Layer 5a)
- `vsg::VertexIndexDraw` → 渲染 OCCT 网格
- `vsg::MatrixTransform` / `Group` / `StateGroup` → 场景组织
- `vsg::Builder::createBox` → 管点正方体
- `vsg::LOD` → 距离 LOD
- `vsg::LineSegmentIntersector` → 拾取

### VTK (Layer 5b, AnalysisWorkbench)
- `vtkPolyData` → 实体模式网格 (来自 OCCT 转换)
- `vtkPolyLine` + `vtkCellArray` → 线条模式梁单元
- `vtkGlyph3D` + `vtkSphereSource` → 节点标记
- `vtkActor` 可见性切换 → 渲染模式切换
- `vtkRenderer` / `vtkRenderWindow` → VTK 渲染管线

### Qt/QML (Layer 7)
- `QQuickFramebufferObject` → VSG 嵌入 QML
- `QAbstractTableModel` → 管点表格
- `QAbstractItemModel` → 段树

---

## 14. 验证计划

1. **Phase 1**: test_geometry — 创建圆柱/圆环/锥 → 三角化 → 验证顶点数 > 0
2. **Phase 2**: test_model — 创建 PipePoint/PipeSpec/Segment → 验证引用关系/查询
3. **Phase 3**: test_engine — BendCalculator 验证 N/M/F 坐标；RunBuilder 验证 Shape 非空
4. **Phase 4**: test_visualization — Shape→VSG 节点→验证顶点/索引非空
5. **Phase 5**: 手动 — QML 启动→表格输入管点→3D 渲染→参数修改→几何更新
6. **Phase 6**: 导出 STEP→FreeCAD 验证
7. **Phase 7**: test_loads — 载荷/工况 CRUD + 依赖图 + 序列化 round-trip; VTK 渲染模式切换; 端到端多工作台流程
8. **Phase 8**: test_command_base / test_command_stack / test_command_registry / test_property_commands / test_structural_commands / test_insert_component — 验证命令执行、撤销重做、序列化、结构编辑与 UI 迁移链路

---

## 15. 关键决策记录

| 决策 | 说明 |
|------|------|
| 管点为中心 | 管件不是独立对象，是管点类型+推导规则的结果 |
| PipeSpec 可扩展字段 | map<string,Variant> 字典，预留标准数据库接口 |
| 管点级别引用 PipeSpec | 异径接头自然由前后管点引用不同 PipeSpec 产生 |
| Bend 倍数存管点上 | 每个 Bend 可独立指定 1.5D/2D/5D |
| 仅法兰连接 | — |
| 工程文件=JSON | nlohmann/json 序列化参数，3D 几何不存储，打开时重建 |
| STEP 仅导出 | 用于与其他 CAD 软件交换最终几何 |
| 表格输入为主交互 | 管点通过表格输入坐标和参数 |
| 命令栈驱动编辑 | `CommandStack` / `CommandRegistry` 取代 `TransactionManager`，命令完成/撤销/重做后统一重算 |
| VTK 用于分析工作台 | 管道应力分析采用 VTK 渲染，支持实体/线条双模式 |
| 三工作台架构 | Specification(规格管理) → Design(路由设计) → Analysis(应力分析) |
| 载荷两层模型 | Load→LoadCase(基本工况,独立求解) → LoadCombination(组合工况,后处理代数运算) |
| 参数化构件模板库 | 形状拓扑固定+尺寸按OD缩放，不用草图/STEP文件库，ComponentCatalog代码模板 |
| 工作台系统 | FreeCAD 风格，CAD/CAE 两个工作台，文档模型共享 |
| 初期全公制 | 内部统一公制存储，单位转换后续增加 |
| pixi + CMake + Ninja | 通用依赖 pixi 管理，OCCT/VSG 本地 lib/ fallback |
| command 兼容别名 | `src/command` target 指向 `app`，避免 app↔command 循环依赖 |
| 单文档模式 | 一次只打开一个工程 |
| 3D 鼠标映射 | 滚轮=缩放, 滚轮拖=平移, Ctrl+左键拖=旋转, 左键=选择, 右键=菜单 |
| ViewManager 统一门面 | 视口路由+渲染状态+交互协调，QML 只与 ViewManager 交互，不感知 VSG/VTK |
| Application 中央单例 | 所有管理器由 Application 单例持有，使用 `std::call_once` 保证线程安全初始化，并提供 `createCommandContext()` 供 UI/命令调用 |

---

## 16. 进一步考虑

1. **VSG-QML Vulkan 互操作**: 初期方案 A(离屏渲染→CPU 拷贝→QML 纹理)快速原型；30k 构件帧率不足时升级方案 B(VK_KHR_external_memory 共享)

2. **Bend N/M/F 点用户可见性**: N/M/F 点是计算生成的，用户在表格中只输入交点坐标，N/M/F 自动计算显示。UI 区分"用户输入管点"和"自动计算管点"。

3. **大规模性能**: 实例化渲染(相同 PipeSpec+类型共享几何)、LOD、异步三角化、场景分区按段/路由分组可选加载。

4. **工程文件打开重建流程**: JSON→反序列化→全量依赖图构建→全量 Recompute→SceneManager 构建 VSG 场景。30k 管点可按 Segment 分批异步重建。

5. **颜色/显示模式**: 支持多种着色模式(按段/按 PipeSpec/按管点类型/自定义颜色)。

6. **测量工具**: 基础测量(两点距离/管段长度/路由总长)基于管点坐标计算，初期文本输出，后续 3D 标注。
