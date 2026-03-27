# 管道系统参数化建模软件 — 架构设计文档

> **版本**: 0.1.0 | **日期**: 2026-03-27 | **状态**: 初始设计

---

## 1. 概述

基于 **OCCT**(几何内核) + **VSG**(Vulkan 渲染) + **QML**(UI 框架) 构建海上油气/风电平台管道系统参数化建模软件。

**核心设计理念**: 以 **管点(PipePoint)** 为中心的数据模型 — 管点是带坐标和类型的文档对象，管件几何由管点序列 + 管线特性(PipeSpec) 推导生成。管点按段(Segment)组织，段按树状结构组成路由(Route)。

采用 **7 层架构**，QML 通过 FBO 嵌入 VSG 3D 视口，仅法兰连接，STEP 格式数据交换。

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
| **VTK** | 架构预留 CAE 后处理，初期不集成 |
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
  │      ├── Accessory       附属构件基类 (关联管点引用 + offset)
  │      │    ├── FixedPoint 固定点
  │      │    ├── Support    支架
  │      │    ├── Flange     法兰
  │      │    ├── Gasket     垫片
  │      │    └── SealRing   密封圈
  │      └── Beam            梁 (双端管点引用 + 截面参数)
  │
  ├─── PropertyObject (无坐标基类: 属性/配置文档)
  │      ├── PipeSpec        管线特性 (可扩展字段: OD/WT/材料...)
  │      ├── ProjectConfig   工程配置 (名称/作者/标准/单位制/重力方向)
  │      └── (后续扩展: 载荷定义/材料库/标准规格...)
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
| **Valve** | 参数化阀体, 端口OD=PipeSpec.OD, 阀体尺寸由valveType+OD决定 |
| **FlexJoint** | 波纹管壳, OD=PipeSpec.OD, 长度=两点距离 |

---

## 4. 架构分层

```
┌─────────────────────────────────────────────┐
│           Layer 7: QML UI Layer             │
│  (表格编辑器/属性面板/段树/3D视口)            │
├─────────────────────────────────────────────┤
│           Layer 6: Application Layer        │
│  (文档管理/Undo-Redo/选择管理/工作台系统)      │
├─────────────────────────────────────────────┤
│     Layer 5: Visualization Bridge           │
│  (OCCT Mesh→VSG SceneGraph / 拾取 / LOD)    │
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

Layer 3 是"文档模型层"，构件不持有几何生成逻辑；Layer 4 "管道领域引擎"负责从管点序列+PipeSpec推导几何。

---

## 5. 构建系统

### 5.1 工具链

| 工具 | 作用 | 来源 |
|------|------|------|
| **pixi** | 包管理/环境隔离 | prefix.dev |
| **CMake 3.24+** | 构建配置 | pixi (conda-forge) |
| **Ninja** | 并行构建后端 | pixi (conda-forge) |

### 5.2 依赖管理

通用依赖通过 pixi (conda-forge) 安装。**OCCT 和 VSG 初期使用本地预编译库 (lib/)**，后续自建 conda recipe 迁入 pixi。

| 包名 | 来源 | 备注 |
|------|------|------|
| `cmake` | pixi ✅ | 构建工具 |
| `ninja` | pixi ✅ | 构建后端 |
| `qt6-main` | pixi ✅ | Qt6 Quick/QML |
| `nlohmann_json` | pixi ✅ | JSON 序列化 |
| `gtest` | pixi ✅ | 单元测试 |
| `vulkan-headers` + `vulkan-loader` | pixi ✅ | Vulkan 依赖 |
| **`occt` (8.0.0)** | **本地 lib/occt/** | 后续迁入 pixi |
| **`vsg` (1.1.13)** | **本地 lib/vsg/** | 后续迁入 pixi |
| `vtk` | pixi (延迟) | CAE 工作台，初期不装 |

### 5.3 pixi.toml

```toml
[project]
name = "qml-vsg-occt"
version = "0.1.0"
description = "Pipeline parametric modeling software"
channels = ["conda-forge"]
platforms = ["linux-64", "win-64"]

[dependencies]
cmake = ">=3.24"
ninja = "*"
pkg-config = "*"
nlohmann_json = "*"
gtest = "*"
vulkan-headers = "*"
vulkan-loader = "*"
qt6-main = ">=6.5"

# OCCT & VSG — 初期使用本地 lib/, 后续迁入 pixi:
# occt = ">=8.0.0"
# vulkanscenegraph = ">=1.1.0"

# VTK (CAE工作台，初期不启用)
# vtk = ">=9.6"

[target.linux-64.dependencies]
xorg-libx11 = "*"
xorg-libxcb = "*"

[feature.dev.dependencies]
clang-format = "*"
ccache = "*"

[tasks]
configure-debug = { cmd = "cmake -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON" }
configure-release = { cmd = "cmake -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release" }
build-debug = { cmd = "cmake --build build/debug", depends-on = ["configure-debug"] }
build-release = { cmd = "cmake --build build/release", depends-on = ["configure-release"] }
test = { cmd = "ctest --test-dir build/debug --output-on-failure", depends-on = ["build-debug"] }
clean = { cmd = "rm -rf build/" }
```

### 5.4 CMakeLists.txt (根)

```cmake
cmake_minimum_required(VERSION 3.24)
project(qml-vsg-occt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 本地预编译库 fallback (OCCT/VSG)
# 迁入 pixi 后删除这两行
list(APPEND CMAKE_PREFIX_PATH
  ${PROJECT_SOURCE_DIR}/lib/occt
  ${PROJECT_SOURCE_DIR}/lib/vsg
)

# 第三方库
find_package(OpenCASCADE REQUIRED)
find_package(vsg REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Quick Qml)
find_package(nlohmann_json REQUIRED)

# 源码
add_subdirectory(src)

# 测试
enable_testing()
add_subdirectory(tests)
```

### 5.5 CMake Target 结构

每层一个 static library target，层间显式依赖:

```
foundation          (无外部依赖)
geometry            → foundation + OpenCASCADE
model               → foundation
engine              → model + geometry
visualization       → engine + geometry + vsg
app                 → engine + visualization + nlohmann_json
ui                  → app + Qt6::Quick + Qt6::Qml
qml-vsg-occt (exe)  → ui (应用入口)
```

### 5.6 日常开发流程

```bash
pixi install                    # 安装所有依赖
pixi run configure-debug        # CMake配置(Debug)
pixi run build-debug            # 编译
pixi run test                   # 运行测试
pixi shell                      # 进入pixi环境
```

### 5.7 迁移路径 (OCCT/VSG → pixi)

1. 编写 conda recipe (rattler-build)，从源码或预编译包构建
2. 发布到自定义 channel 或 conda-forge
3. pixi.toml 中取消注释 `occt`/`vulkanscenegraph` 依赖
4. CMakeLists.txt 删除 `lib/occt` `lib/vsg` 的 fallback
5. 删除 `lib/` 目录

---

## 6. 工作台系统

### 6.1 设计理念

类似 FreeCAD 的 Workbench 机制 — 每个工作台定义自己的工具栏、面板组合和 3D 渲染模式。切换工作台改变 UI 工具集和面板布局，但**文档模型(Layer 3)共享**。

### 6.2 工作台列表

| 工作台 | 状态 | 职责 | 视口引擎 |
|--------|------|------|----------|
| **CAD** | 当前开发 | 管道系统参数化建模 | VSG (Vulkan) |
| **CAE** | 未来阶段 | 有限元分析后处理 | VTK |

### 6.3 核心抽象

```cpp
// Workbench (abstract, Layer 6: app/)
class Workbench {
    virtual string name() = 0;
    virtual void activate(Document&, AppContext&) = 0;
    virtual void deactivate() = 0;
    virtual vector<Action> toolbarActions() = 0;
    virtual vector<string> panelIds() = 0;
    virtual ViewportType viewportType() = 0;  // VSG | VTK
};

// WorkbenchManager (Layer 6: app/)
class WorkbenchManager {
    void registerWorkbench(unique_ptr<Workbench>);
    void switchWorkbench(string name);
    Workbench* activeWorkbench();
    vector<string> workbenchNames();
};

// CadWorkbench : Workbench
//   panelIds = {StructureTree, Viewport3D, PipePointTable, PropertyPanel, PipeSpecEditor}
//   toolbarActions = {新建段/添加管点/添加附属/测量/STEP导出}
//   viewportType = VSG

// CaeWorkbench : Workbench (未来)
//   panelIds = {ResultTree, VtkViewport, ColorMapPanel, CutPlanePanel}
//   viewportType = VTK
```

### 6.4 切换机制

- TopBar 显示工作台切换标签（CAD / CAE）
- 切换时: `WorkbenchManager::switchWorkbench()` → 旧工作台 `deactivate()` + 新工作台 `activate()` → `workbenchChanged` 信号 → QML 根据 `panelIds()` 动态加载/卸载面板
- Document 在工作台之间共享

---

## 7. 事务管理机制

### 7.1 设计原则

- **事务内不触发重算**: 属性变更只记录到事务日志并标脏，不立即触发级联更新
- **提交后统一重算**: commit 后走依赖图拓扑排序，批量重算受影响对象
- **单步操作=单事务**: 每个用户操作对应一个事务，即一次 Undo 步骤
- **单线程**: 所有文档变更在主线程执行
- **会话级**: 事务历史仅在内存中，关闭应用后丢失

### 7.2 核心组件

```
┌──────────────────────────────────────────────┐
│     TransactionManager (Layer 6: app/)       │
│  open(desc) / commit() / abort()             │
│  undo() / redo()                             │
│  事务日志: vector<PropertyChange{obj,key,old,new}> │
├──────────────────────────────────────────────┤
│     DependencyGraph (Layer 6: app/)          │
│  PipeSpec ──→ PipePoint[]                    │
│  PipePoint ──→ 相邻PipePoint[]               │
│  PipePoint ──→ Accessory[] / Beam[]          │
├──────────────────────────────────────────────┤
│     RecomputeEngine (Layer 4: engine/)       │
│  收集脏对象 → 拓扑排序 → 批量重算Shape        │
│  → SceneManager批量更新VSG → UI信号通知       │
└──────────────────────────────────────────────┘
```

### 7.3 事务生命周期

```
1. TransactionManager::open("修改OD")
2. pipeSpec->setField("OD", 219.1)
   → 记录日志: {pipeSpec, "OD", old:168.3, new:219.1}
   → 标记脏: pipeSpec + 所有引用它的PipePoint
3. TransactionManager::commit()
4. RecomputeEngine::recompute()
   ├─ 收集脏对象 → 依赖图拓扑排序
   ├─ 对每个脏PipePoint: GeometryDeriver重算Shape
   ├─ 对每个变更的Shape: SceneManager批量更新VSG节点
   └─ 发出 documentChanged 信号 → UI批量刷新
```

### 7.4 Undo/Redo

```
undo(): 取最近事务日志 → 反向回放(恢复旧值) → 标脏 → recompute()
redo(): 取下一个事务日志 → 正向回放(恢复新值) → 标脏 → recompute()
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

## 8. 目录结构

```
qml-vsg-occt/
├── pixi.toml                       # pixi包管理配置
├── CMakeLists.txt                   # 根CMake
├── .gitignore                       # build/, .pixi/
├── README.md
├── docs/                            # 文档
│   └── architecture.md              # 本文档
├── lib/                             # 本地预编译库, 迁入pixi后删除
│   ├── occt/                        # OCCT 8.0.0
│   ├── vsg/                         # VSG 1.1.13
│   └── vtk/                         # VTK 9.6.0 (CAE预留)
├── src/
│   ├── CMakeLists.txt
│   ├── main.cpp                     # 应用入口
│   ├── foundation/                  # Layer 1
│   │   ├── Types.h                  # UUID, Variant, 基础类型
│   │   ├── Math.h                   # 角度/方向/坐标系数学工具
│   │   ├── Signal.h                 # 参数变更通知
│   │   └── Log.h
│   ├── geometry/                    # Layer 2 - OCCT封装
│   │   ├── OcctTypes.h              # Handle别名/前置声明
│   │   ├── ShapeBuilder.h/cpp       # BRep封装
│   │   ├── ShapeMesher.h/cpp        # BRepMesh → Poly_Triangulation
│   │   ├── ShapeTransform.h/cpp     # gp_Trsf封装
│   │   ├── BooleanOps.h/cpp         # 布尔运算封装
│   │   ├── StepIO.h/cpp             # STEP导出封装
│   │   └── ShapeProperties.h/cpp    # BRepGProp封装
│   ├── model/                       # Layer 3 - 文档模型
│   │   ├── DocumentObject.h/cpp     # 基类(ID+name)
│   │   ├── SpatialObject.h/cpp      # 有坐标基类
│   │   ├── PropertyObject.h/cpp     # 无坐标属性基类
│   │   ├── ContainerObject.h/cpp    # 组织容器基类
│   │   ├── PipePoint.h/cpp          # 管点
│   │   ├── PipeSpec.h/cpp           # 管线特性
│   │   ├── ProjectConfig.h/cpp      # 工程配置
│   │   ├── Segment.h/cpp            # 段
│   │   ├── Route.h/cpp              # 路由
│   │   ├── Beam.h/cpp               # 梁
│   │   ├── Accessory.h/cpp          # 附属构件基类
│   │   ├── FixedPoint.h/cpp         # 固定点
│   │   ├── Support.h/cpp            # 支架
│   │   ├── Flange.h/cpp             # 法兰
│   │   ├── Gasket.h/cpp             # 垫片
│   │   └── SealRing.h/cpp           # 密封圈
│   ├── engine/                      # Layer 4 - 管道领域引擎
│   │   ├── GeometryDeriver.h/cpp    # 统一几何推导入口
│   │   ├── BendCalculator.h/cpp     # 弯头几何计算
│   │   ├── RunBuilder.h/cpp         # 管段几何
│   │   ├── ReducerBuilder.h/cpp     # 异径接头几何
│   │   ├── TeeBuilder.h/cpp         # 三通几何
│   │   ├── ValveBuilder.h/cpp       # 阀门几何
│   │   ├── FlexJointBuilder.h/cpp   # 柔性接头几何
│   │   ├── BeamBuilder.h/cpp        # 梁几何
│   │   ├── AccessoryBuilder.h/cpp   # 附属构件几何
│   │   ├── TopologyManager.h/cpp    # 路由拓扑管理
│   │   ├── ConstraintSolver.h/cpp   # 约束求解
│   │   ├── PipelineValidator.h/cpp  # 干涉检测/校验
│   │   └── RecomputeEngine.h/cpp    # 批量重算引擎
│   ├── visualization/               # Layer 5
│   │   ├── OcctToVsg.h/cpp          # OCCT→VSG顶点数据
│   │   ├── SceneManager.h/cpp       # VSG场景图管理
│   │   ├── PipePointNode.h/cpp      # 管点渲染
│   │   ├── ComponentNode.h/cpp      # 管件3D节点
│   │   ├── SelectionHighlight.h/cpp # 选中高亮
│   │   ├── LodStrategy.h/cpp        # LOD策略
│   │   ├── VsgViewport.h/cpp        # VSG渲染器
│   │   ├── CameraController.h/cpp   # 相机控制
│   │   ├── SceneFurniture.h/cpp     # 坐标轴/网格/背景
│   │   └── PickHandler.h/cpp        # 3D拾取
│   ├── app/                         # Layer 6
│   │   ├── Document.h/cpp           # 文档容器
│   │   ├── TransactionManager.h/cpp # 事务管理
│   │   ├── DependencyGraph.h/cpp    # 依赖图
│   │   ├── ProjectSerializer.h/cpp  # JSON序列化
│   │   ├── SelectionManager.h/cpp   # 选择管理
│   │   ├── StepExporter.h/cpp       # STEP导出
│   │   ├── Workbench.h              # 工作台基类
│   │   ├── WorkbenchManager.h/cpp   # 工作台管理
│   │   └── CadWorkbench.h/cpp       # CAD工作台
│   └── ui/                          # Layer 7 - QML Bridge
│       ├── VsgQuickItem.h/cpp       # QQuickFramebufferObject
│       ├── AppController.h/cpp      # QML控制器
│       ├── WorkbenchController.h/cpp# 工作台桥接
│       ├── PipePointTableModel.h/cpp# 管点表格Model
│       ├── SegmentTreeModel.h/cpp   # 段树Model
│       ├── PropertyModel.h/cpp      # 属性面板Model
│       └── PipeSpecModel.h/cpp      # 管线特性Model
├── ui/
│   ├── main.qml                     # 主窗口布局
│   ├── style/
│   │   └── Theme.qml                # 颜色/字体常量
│   ├── components/                  # 可复用QML组件
│   │   ├── CollapsiblePanel.qml
│   │   ├── SplitView.qml
│   │   ├── IconButton.qml
│   │   ├── EditableCell.qml
│   │   └── ContextMenu.qml
│   ├── panels/
│   │   ├── TopBar.qml
│   │   ├── StructureTree.qml
│   │   ├── Viewport3D.qml
│   │   ├── PipePointTable.qml
│   │   ├── PropertyPanel.qml
│   │   ├── PipeSpecEditor.qml
│   │   └── StatusBar.qml
│   └── dialogs/
│       ├── NewProjectDialog.qml
│       ├── OpenProjectDialog.qml
│       └── ProjectConfigDialog.qml
└── tests/
    ├── test_model.cpp
    ├── test_geometry.cpp
    ├── test_engine.cpp
    ├── test_pipeline.cpp
    └── test_visualization.cpp
```

### 8.1 应用入口 (main.cpp)

1. 创建 `QGuiApplication`
2. 注册 C++ 类型到 QML (`qmlRegisterType<VsgQuickItem>`, `qmlRegisterSingletonType<AppController>` 等)
3. 创建 `Document`, `TransactionManager`, `DependencyGraph`, `SelectionManager`
4. 创建 `WorkbenchManager`, 注册 `CadWorkbench`
5. 创建 `QQmlApplicationEngine`, 设置根上下文属性
6. 加载 `ui/main.qml`
7. 进入事件循环

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
│  [状态信息]              [坐标]                     [缩放]   │  ← StatusBar
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

**编辑规则**: 所有可编辑字段修改即走事务(open→set→commit→recompute)；只读字段灰色 #888888 无边框，带(auto)标签；关联项点击跳转联动。

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
| 树节点选中 | 表格滚动到对应行 + 3D 视口高亮 |
| 表格行选中 | 树展开到对应节点 + 3D 视口高亮 |
| 3D 视口拾取 | 树+表格同步选中 |
| 表格编辑坐标 | 事务→recompute→3D 视口实时更新 |
| 属性面板修改 PipeSpec | 事务→recompute→所有引用管点 3D 更新 |

### 9.6 TopBar / StatusBar

- **TopBar**: 极简图标工具栏(新建/打开/保存/撤销/重做/选择/测量)，无文字标签，悬停提示。左侧工作台切换标签(CAD|CAE)，当前活跃高亮。工具栏内容由活跃工作台的 `toolbarActions()` 决定。
- **StatusBar**: 管点数/段数/路由数 | 鼠标处 3D 坐标 | 缩放比例

---

## 10. 3D 视口

### 10.1 鼠标交互

| 操作 | 功能 |
|------|------|
| **滚轮滚动** | 缩放 (以鼠标位置为中心) |
| **滚轮按下拖动** | 平移视图 (Pan) |
| **左键点击** | 选择对象 (LineSegmentIntersector → SelectionManager) |
| **Ctrl + 左键拖动** | 轨道旋转 (Orbit) |
| **右键点击** | 弹出上下文菜单 |

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

**初期功能 (v0.1)**:
- **查看/编辑管点属性** — 拾取到管点或管件时可用；引导至右侧属性面板(已展开→闪烁动画, 已收缩→自动展开)

**后续扩展**: 删除管点/添加管点、添加附属构件、隐藏/显示、Fit to selection、测量

### 10.4 场景基础设施

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
- `Document` / `TransactionManager` / `DependencyGraph` / `RecomputeEngine`
- `SelectionManager` / `Workbench` + `WorkbenchManager` / `CadWorkbench`
- `WorkbenchController`: C++→QML 桥接

**Step 15 — QML UI**
- `PipePointTable.qml`: 核心交互 — 管点表格
- `SegmentTree.qml` / `PipeSpecEditor.qml` / `PropertyPanel.qml`
- `Viewport3D.qml` / `ContextMenu.qml`

### Phase 6: 数据交换 (Steps 16-17, 依赖 Phase 3+5)

**Step 16 — STEP 导入/导出**
- 导出: 遍历管点→GeometryDeriver→STEPControl_Writer
- 侧车 JSON: 管点序列/PipeSpec/段结构/附属构件

**Step 17 — 管道校验**
- 干涉检测, 规格校验, 未连接端口

---

## 13. 关键技术文件

### OCCT (Layer 2)
- `BRepPrimAPI_MakeCylinder` / `MakeTorus` / `MakeCone` → 管/弯头/异径
- `BRepAlgoAPI_Cut` / `Fuse` → 管壳/三通
- `BRepOffsetAPI_MakePipeShell` → 梁扫掠
- `BRepMesh_IncrementalMesh` + `Poly_Triangulation` + `BRep_Tool::Triangulation()` → 三角化
- `STEPControl_Writer` / `Reader` → STEP
- `gp_Trsf` + `BRepBuilderAPI_Transform` → 变换

### VSG (Layer 5)
- `vsg::VertexIndexDraw` → 渲染 OCCT 网格
- `vsg::MatrixTransform` / `Group` / `StateGroup` → 场景组织
- `vsg::Builder::createBox` → 管点正方体
- `vsg::LOD` → 距离 LOD
- `vsg::LineSegmentIntersector` → 拾取

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
| FreeCAD 风格事务 | 事务内不触发重算，commit 后统一 recompute |
| VTK 延迟 | 预留接口，初期不集成 |
| 工作台系统 | FreeCAD 风格，CAD/CAE 两个工作台，文档模型共享 |
| 初期全公制 | 内部统一公制存储，单位转换后续增加 |
| pixi + CMake + Ninja | 通用依赖 pixi 管理，OCCT/VSG 本地 lib/ fallback |
| 单文档模式 | 一次只打开一个工程 |
| 3D 鼠标映射 | 滚轮=缩放, 滚轮拖=平移, Ctrl+左键拖=旋转, 左键=选择, 右键=菜单 |

---

## 16. 进一步考虑

1. **VSG-QML Vulkan 互操作**: 初期方案 A(离屏渲染→CPU 拷贝→QML 纹理)快速原型；30k 构件帧率不足时升级方案 B(VK_KHR_external_memory 共享)

2. **Bend N/M/F 点用户可见性**: N/M/F 点是计算生成的，用户在表格中只输入交点坐标，N/M/F 自动计算显示。UI 区分"用户输入管点"和"自动计算管点"。

3. **大规模性能**: 实例化渲染(相同 PipeSpec+类型共享几何)、LOD、异步三角化、场景分区按段/路由分组可选加载。

4. **工程文件打开重建流程**: JSON→反序列化→全量依赖图构建→全量 Recompute→SceneManager 构建 VSG 场景。30k 管点可按 Segment 分批异步重建。

5. **颜色/显示模式**: 支持多种着色模式(按段/按 PipeSpec/按管点类型/自定义颜色)。

6. **测量工具**: 基础测量(两点距离/管段长度/路由总长)基于管点坐标计算，初期文本输出，后续 3D 标注。
