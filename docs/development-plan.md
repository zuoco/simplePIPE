# 开发计划 — 任务分解与分配指南

> **版本**: 0.2.0 | **日期**: 2026-03-29  
> **参考**: [architecture.md](architecture.md) — 架构设计文档

---

## 总览

项目共分 **6 个 Phase、17 个 Step**，拆分为 **25 个可独立分配的任务(Task)**。每个 Task 有明确的输入、输出、验收标准，可交给不同开发者或 AI Agent 并行执行。

### 依赖关系图

```
Phase 1: 基础设施
  T01 ──→ T02 ──→ T03
              ──→ T04

Phase 2: 文档模型 (依赖 T02)
  T05 ──→ T06

Phase 3: 管道领域引擎 (依赖 T05)          Phase 4: 可视化桥接 (依赖 T04)
  T07 ──→ T08                               T11 ──→ T12 ──→ T13
  T07 ──→ T09                                         ──→ T14
  T05 ──→ T10

Phase 5: QML集成 (依赖 Phase 3 + Phase 4)
  T15 ──→ T16 ──→ T17 ──→ T18 ──→ T19

Phase 6: 数据交换 (依赖 Phase 3 + Phase 5)
  T20 ──→ T21

横切任务 (贯穿全程):
  T22  JSON序列化
  T23  事务管理
  T24  选择管理
  T25  集成测试与调试
```

### 并行分配建议

| 并行组 | 可同时进行的任务 | 所需技能 |
|--------|----------------|---------|
| **A 组** | T02 + T03 + T04 | C++ 基础 / OCCT |
| **B 组** | T05 + T06 (Phase 1 完成后) | C++ 数据建模 |
| **C 组 (引擎)** | T07 + T08 + T09 + T10 | OCCT 几何算法 |
| **D 组 (可视化)** | T11 + T12 + T13 + T14 | VSG / Vulkan |
| **E 组 (UI)** | T17 + T18 + T19 | Qt/QML |
| **F 组 (应用)** | T15 + T16 + T22 + T23 + T24 | C++ 架构 |

---

## Phase 1: 基础设施

### T01 — 构建系统搭建

| 属性 | 值 |
|------|---|
| **对应** | Step 1 |
| **预计规模** | ~10 文件 |
| **前置依赖** | 无 |
| **所需技能** | CMake, pixi |
| **可分配给** | 任何开发者 / AI Agent |

**输入**: 架构文档 §5 (构建系统)

**交付物**:
- `pixi.toml` — 完整依赖声明 + 构建任务
- `CMakeLists.txt` (根) — find_package, C++17, AUTOMOC/AUTORCC, 本地 lib/ fallback
- `src/CMakeLists.txt` — 7 层 static library target 骨架 (空 .cpp 占位)
- `tests/CMakeLists.txt` — GTest 框架接入
- `.gitignore` — build/, .pixi/, *.pyc

**验收标准**:
- [ ] `pixi install` 成功
- [ ] `pixi run configure-debug` 成功 (CMake 配置通过)
- [ ] `pixi run build-debug` 成功 (空项目编译通过)
- [ ] `pixi run test` 通过 (GTest 空测试)
- [ ] 7 个 static library target 均存在 (foundation/geometry/model/engine/visualization/app/ui)

---

### T02 — Foundation 层

| 属性 | 值 |
|------|---|
| **对应** | Step 2 |
| **预计规模** | ~4 文件 |
| **前置依赖** | T01 |
| **所需技能** | C++ 基础 |

**交付物**:
- `src/foundation/Types.h` — UUID 生成 (随机 v4), Variant (double/int/string/enum 联合类型), 单位枚举
- `src/foundation/Math.h` — deg↔rad 转换, 3D 向量运算 (长度/归一化/点积/叉积/夹角), 两线交点计算
- `src/foundation/Signal.h` — 轻量信号/槽 (属性变更通知, 不依赖 Qt)
- `src/foundation/Log.h` — 日志宏 (stdout, 可后续替换)

**验收标准**:
- [ ] UUID 生成唯一且格式正确
- [ ] Variant 可存取 double/int/string 且类型安全
- [ ] Math: 两线交点计算误差 < 1e-6 mm
- [ ] Math: 角度/弧度转换正确
- [ ] 单元测试全部通过 (`tests/test_foundation.cpp`)

---

### T03 — OCCT 几何封装 (ShapeBuilder + BooleanOps + ShapeTransform)

| 属性 | 值 |
|------|---|
| **对应** | Step 3 (部分) |
| **预计规模** | ~6 文件 |
| **前置依赖** | T01 |
| **所需技能** | C++, OCCT BRep API |

**交付物**:
- `src/geometry/OcctTypes.h` — Handle 别名, 前置声明
- `src/geometry/ShapeBuilder.h/cpp` — 封装:
  - `makeCylinder(radius, height)` → TopoDS_Shape
  - `makeTorus(majorR, minorR, angle)` → TopoDS_Shape
  - `makeCone(r1, r2, height)` → TopoDS_Shape
  - `makePipeShell(wire, radius)` → TopoDS_Shape
- `src/geometry/BooleanOps.h/cpp` — `cut(s1,s2)`, `fuse(s1,s2)` → TopoDS_Shape
- `src/geometry/ShapeTransform.h/cpp` — `translate(shape, vec)`, `rotate(shape, axis, angle)`, `transform(shape, trsf)`

**验收标准**:
- [ ] makeCylinder/makeTorus/makeCone 返回非空 Shape
- [ ] cut(外柱-内柱) 创建管壳
- [ ] fuse(主管+支管) 创建三通
- [ ] transform 后 Shape 位置正确
- [ ] 单元测试通过 (`tests/test_geometry.cpp`)

---

### T04 — OCCT 网格化 + STEP I/O

| 属性 | 值 |
|------|---|
| **对应** | Step 3 (部分) |
| **预计规模** | ~4 文件 |
| **前置依赖** | T01 |
| **所需技能** | C++, OCCT Mesh/STEP API |
| **可与 T03 并行** | ✅ |

**交付物**:
- `src/geometry/ShapeMesher.h/cpp`:
  - `mesh(shape, deflection)` — BRepMesh_IncrementalMesh 三角化
  - `extractTriangulation(shape)` → struct MeshData { vertices, normals, indices }
- `src/geometry/StepIO.h/cpp`:
  - `exportStep(shapes, filePath)` — STEPControl_Writer
  - `importStep(filePath)` → vector<TopoDS_Shape> — STEPControl_Reader
- `src/geometry/ShapeProperties.h/cpp`:
  - `volume(shape)`, `surfaceArea(shape)` — BRepGProp

**验收标准**:
- [ ] 圆柱三角化后 vertices.size() > 0 且 indices.size() > 0
- [ ] 导出 STEP 文件可被 FreeCAD 打开
- [ ] 导入再导出的 Shape 体积误差 < 1%
- [ ] 单元测试通过

---

## Phase 2: 文档模型

### T05 — 核心文档对象

| 属性 | 值 |
|------|---|
| **对应** | Step 4 |
| **预计规模** | ~18 文件 |
| **前置依赖** | T02 (Types.h, Math.h) |
| **所需技能** | C++ 面向对象设计 |

**交付物**:

| 文件 | 类 | 说明 |
|------|---|------|
| `DocumentObject.h/cpp` | `DocumentObject` | 基类: UUID + name, 属性变更信号 |
| `SpatialObject.h/cpp` | `SpatialObject` | +position (gp_Pnt) |
| `PropertyObject.h/cpp` | `PropertyObject` | 无坐标属性基类 |
| `ContainerObject.h/cpp` | `ContainerObject` | 容器基类, 子对象管理 |
| `PipePoint.h/cpp` | `PipePoint : SpatialObject` | type 枚举 + PipeSpec 引用 + 类型参数 map |
| `PipeSpec.h/cpp` | `PipeSpec : PropertyObject` | map<string,Variant> 可扩展字段 |
| `ProjectConfig.h/cpp` | `ProjectConfig : PropertyObject` | 工程名/作者/标准/单位制 |
| `Segment.h/cpp` | `Segment : ContainerObject` | 有序管点列表 + 段 ID |
| `Route.h/cpp` | `Route : ContainerObject` | 段的树状集合 |

**验收标准**:
- [ ] PipePoint 创建后 UUID 唯一, name/type/position 可读写
- [ ] PipeSpec 字段 set/get 正确, 支持 double/string 类型
- [ ] Segment 管点增删有序, Route 包含多个 Segment
- [ ] PipePoint 引用 PipeSpec 后可读取 OD/WT
- [ ] 继承体系: dynamic_cast 可正确识别各子类型
- [ ] 属性变更时发出信号
- [ ] 单元测试通过 (`tests/test_model.cpp`)

---

### T06 — 附属对象与梁

| 属性 | 值 |
|------|---|
| **对应** | Step 5 |
| **预计规模** | ~12 文件 |
| **前置依赖** | T05 |
| **所需技能** | C++ |

**交付物**:
- `Accessory.h/cpp` — 基类 (SpatialObject + 关联管点引用 + offset)
- `FixedPoint.h/cpp` — 固定点 (固定约束标记)
- `Support.h/cpp` — 支架 (类型 + 承载方向)
- `Flange.h/cpp` — 法兰 (等级/面型/螺栓孔数)
- `Gasket.h/cpp` — 垫片 (材料/厚度)
- `SealRing.h/cpp` — 密封圈 (材料/截面尺寸)
- `Beam.h/cpp` — 梁 (双端管点引用 + 截面类型 + 截面尺寸)

**验收标准**:
- [ ] Accessory 关联管点后可反查
- [ ] Beam 双端引用正确, 长度 = 两端管点距离
- [ ] PipePoint 可查询关联的 Accessory 列表
- [ ] 单元测试通过

---

## Phase 3: 管道领域引擎

### T07 — 弯头几何计算器 (核心算法)

| 属性 | 值 |
|------|---|
| **对应** | Step 6 |
| **预计规模** | ~2 文件 |
| **前置依赖** | T05 (PipePoint), T02 (Math) |
| **所需技能** | 3D 几何算法, OCCT |
| **优先级** | **最高** — 整个管道系统的核心 |

**交付物**:
- `src/engine/BendCalculator.h/cpp`

**接口**:
```cpp
struct BendResult {
    gp_Pnt nearPoint;   // N点
    gp_Pnt midPoint;    // M点
    gp_Pnt farPoint;    // F点
    gp_Pnt arcCenter;   // 弯弧圆心
    double bendAngle;   // 弯曲角度 (rad)
    double bendRadius;  // 弯曲半径 (mm)
};

BendResult calculateBend(
    const gp_Pnt& prevPoint,      // A05
    const gp_Pnt& intersectPoint, // A06 (交点)
    const gp_Pnt& nextPoint,      // A07
    double outerDiameter,          // OD (mm)
    double bendMultiplier          // 1.5 | 2.0 | 5.0
);
```

**算法要点**:
1. 弯曲半径 $R = OD \times multiplier$
2. 弯曲角度 $\theta = \pi - \angle(A05 \to A06, A06 \to A07)$
3. 入射方向 $\vec{d_1} = normalize(A06 - A05)$, 出射方向 $\vec{d_2} = normalize(A07 - A06)$
4. $N = A06 - \vec{d_1} \times R \times \tan(\theta/2)$
5. $F = A06 + \vec{d_2} \times R \times \tan(\theta/2)$
6. $M$ = 弧中点

**验收标准**:
- [ ] 90° 弯头: N/F 到圆心距离 = R, 弯曲角度 = 90°
- [ ] 45° 弯头: 角度计算正确
- [ ] 180° 弯头 (U型): 退化情况正确处理
- [ ] 接近 0° (直线): 返回错误/标记
- [ ] N/M/F 点均在弯弧上
- [ ] 精度: 误差 < 1e-6 mm
- [ ] 单元测试覆盖 ≥ 5 种角度场景 (`tests/test_engine.cpp`)

---

### T08 — 管件几何生成器 (Run/Reducer/Tee)

| 属性 | 值 |
|------|---|
| **对应** | Step 7 (部分) |
| **预计规模** | ~8 文件 |
| **前置依赖** | T07, T03 (ShapeBuilder, BooleanOps) |
| **所需技能** | OCCT BRep |
| **可与 T09 并行** | ✅ |

**交付物**:
- `RunBuilder.h/cpp` — 两管点 → 圆柱壳 (外柱 cut 内柱)
- `ReducerBuilder.h/cpp` — 两管点 + 两端 PipeSpec → 锥壳
- `TeeBuilder.h/cpp` — 三通管点 + 主/支管参数 → 主管 fuse 支管
- `GeometryDeriver.h/cpp` — 统一入口, 按 type 枚举分发

**验收标准**:
- [ ] RunBuilder: 两点(0,0,0)→(1000,0,0), OD=168.3, WT=7.11 → 管壳 Shape 非空, 长度正确
- [ ] ReducerBuilder: 大端 OD=168.3, 小端 OD=114.3 → 锥壳非空
- [ ] TeeBuilder: 主管+支管 fuse → 非空, 体积 > 单管
- [ ] GeometryDeriver 正确分发
- [ ] 单元测试通过

---

### T09 — 管件几何生成器 (Valve/FlexJoint/Beam/Accessory)

| 属性 | 值 |
|------|---|
| **对应** | Step 7 (部分) |
| **预计规模** | ~8 文件 |
| **前置依赖** | T03, T05 |
| **所需技能** | OCCT BRep |
| **可与 T08 并行** | ✅ |

**交付物**:
- `ValveBuilder.h/cpp` — 参数化阀体 (管端+阀体+法兰端)
- `FlexJointBuilder.h/cpp` — 波纹管 (多段锥壳拼接)
- `BeamBuilder.h/cpp` — 截面沿路径扫掠 (BRepOffsetAPI_MakePipeShell)
- `AccessoryBuilder.h/cpp` — 法兰/支架/固定点简化几何

**验收标准**:
- [ ] ValveBuilder: 指定 OD + 阀门类型 → 非空 Shape
- [ ] BeamBuilder: H型/矩形截面 → 非空 Shape, 长度正确
- [ ] 单元测试通过

---

### T10 — 拓扑管理与约束

| 属性 | 值 |
|------|---|
| **对应** | Steps 8-9 |
| **预计规模** | ~4 文件 |
| **前置依赖** | T05 |
| **所需技能** | 图算法, C++ |

**交付物**:
- `TopologyManager.h/cpp`:
  - 维护段间关系 (三通处分叉图)
  - 管点增删时自动维护 Segment/Route 一致性
  - 三通: 创建分支 Segment, 更新 Route 树
- `ConstraintSolver.h/cpp`:
  - 端口口径匹配检查 (相邻管点 PipeSpec.OD 一致或有 Reducer)
  - Bend 角度范围约束 (0° < θ < 180°)
- `PipelineValidator.h/cpp`:
  - 干涉检测 (BRepExtrema_DistShapeShape)
  - 未连接端口警告

**验收标准**:
- [ ] 三通处自动分叉: 添加 Tee 管点 → 新 Segment 创建
- [ ] 删除管点 → Segment 重新排列
- [ ] 口径不匹配时返回错误列表
- [ ] 单元测试通过

---

## Phase 4: 可视化桥接

### T11 — OCCT→VSG 网格转换

| 属性 | 值 |
|------|---|
| **对应** | Step 10 |
| **预计规模** | ~2 文件 |
| **前置依赖** | T04 (ShapeMesher) |
| **所需技能** | OCCT Mesh, VSG 基础 |

**交付物**:
- `src/visualization/OcctToVsg.h/cpp`:
  - `toVsgGeometry(shape, deflection)` → vsg::ref_ptr<vsg::VertexIndexDraw>
  - 内部: ShapeMesher → MeshData → vsg::vec3Array (顶点/法线) + vsg::uintArray (索引)

**验收标准**:
- [ ] 圆柱 Shape → VSG 几何, 顶点数 > 0
- [ ] 法线方向正确 (指向外侧)
- [ ] 索引三角形面数合理
- [ ] 单元测试通过 (`tests/test_visualization.cpp`)

---

### T12 — VSG 场景管理

| 属性 | 值 |
|------|---|
| **对应** | Step 11 |
| **预计规模** | ~8 文件 |
| **前置依赖** | T11 |
| **所需技能** | VSG 场景图 |

**交付物**:
- `SceneManager.h/cpp`:
  - 文档对象 UUID → vsg::Node 映射 (unordered_map)
  - `addObject(docObj)` → 创建 VSG 节点加入场景
  - `removeObject(uuid)` → 移除节点
  - `updateObject(uuid)` → 重新网格化 + 替换节点
  - `batchUpdate(uuids)` → 批量更新 (recompute 后调用)
- `PipePointNode.h/cpp` — 管点渲染为小正方体 (vsg::Builder::createBox)
- `ComponentNode.h/cpp` — 管件 3D 模型 (MatrixTransform → StateGroup → VertexIndexDraw)
- `LodStrategy.h/cpp` — vsg::LOD 距离分级策略

**验收标准**:
- [ ] addObject 后场景图包含新节点
- [ ] updateObject 后几何更新
- [ ] removeObject 后节点移除
- [ ] LOD: 远距离显示简化几何
- [ ] 单元测试通过

---

### T13 — 相机控制与场景基础设施

| 属性 | 值 |
|------|---|
| **对应** | Step 12 (部分) |
| **预计规模** | ~4 文件 |
| **前置依赖** | T12 |
| **所需技能** | VSG, 3D 数学 |

**交付物**:
- `CameraController.h/cpp`:
  - 滚轮 → 缩放 (以鼠标位置为中心)
  - 中键拖动 → 平移
  - Ctrl+左键拖动 → 轨道旋转
  - `setViewPreset(Front/Right/Top/Isometric)` → 200ms 平滑相机过渡
  - `fitAll()` → ComputeBounds → 调整 LookAt
- `SceneFurniture.h/cpp`:
  - 坐标轴指示器 (左下角固定)
  - 地面网格 (XY 平面, 可开关)
  - 渐变背景 (#E8E8E8 → #D0D0D0)

**验收标准**:
- [ ] 鼠标交互: 缩放/平移/旋转响应正确
- [ ] 视图预设: Front/Right/Top/Iso 方向正确
- [ ] Fit All: 全部对象可见
- [ ] 坐标轴/网格/背景正确渲染

---

### T14 — 3D 拾取与高亮

| 属性 | 值 |
|------|---|
| **对应** | Step 12 (部分) |
| **预计规模** | ~4 文件 |
| **前置依赖** | T12 |
| **所需技能** | VSG Intersector |
| **可与 T13 并行** | ✅ |

**交付物**:
- `PickHandler.h/cpp`:
  - 左键点击 → vsg::LineSegmentIntersector → 反查 UUID → 通知 SelectionManager
  - 右键点击 → 传递拾取位置给 ContextMenu
- `SelectionHighlight.h/cpp`:
  - 选中对象 → 材质替换为高亮色 (#0078D4)
  - 取消选中 → 恢复原始材质

**验收标准**:
- [ ] 点击管件 → 正确返回对应文档对象 UUID
- [ ] 点击空白 → 清空选择
- [ ] 选中后高亮颜色正确

---

## Phase 5: QML 集成

### T15 — VSG-QML 桥接

| 属性 | 值 |
|------|---|
| **对应** | Step 13 |
| **预计规模** | ~2 文件 |
| **前置依赖** | T12 (SceneManager), T13 (CameraController) |
| **所需技能** | Qt Quick, OpenGL/Vulkan FBO |
| **关键风险** | Vulkan→QML 纹理传递性能 |

**交付物**:
- `src/ui/VsgQuickItem.h/cpp`:
  - 继承 `QQuickFramebufferObject`
  - `createRenderer()` → VSG 离屏渲染到 FBO → QML 显示
  - 鼠标事件 (`mousePressEvent` 等) 转发给 CameraController / PickHandler
  - 键盘事件转发 (Numpad 视图切换, F=FitAll, Delete=删除)
  - `requestUpdate()` → 触发重绘

**验收标准**:
- [ ] QML 窗口中显示 VSG 渲染的 3D 场景
- [ ] 鼠标交互正常 (缩放/平移/旋转/拾取)
- [ ] 键盘快捷键生效
- [ ] 窗口 resize 后正确重绘

---

### T16 — 应用层核心 (Document + Transaction + DependencyGraph)

| 属性 | 值 |
|------|---|
| **对应** | Step 14 (部分) |
| **预计规模** | ~8 文件 |
| **前置依赖** | T05, T07 |
| **所需技能** | C++ 架构, 图算法 |

**交付物**:
- `src/app/Document.h/cpp`:
  - 所有文档对象容器 (unordered_map<UUID, unique_ptr<DocumentObject>>)
  - `addObject()` / `removeObject()` / `findObject(uuid)` / `findByType<T>()`
  - `allPipePoints()` / `allSegments()` 便捷查询
- `src/app/TransactionManager.h/cpp`:
  - `open(description)` / `commit()` / `abort()`
  - `undo()` / `redo()`
  - 事务日志: vector\<PropertyChange{obj, key, oldValue, newValue}\>
  - commit 后触发 RecomputeEngine
- `src/app/DependencyGraph.h/cpp`:
  - 对象间依赖边 (PipeSpec→PipePoint[], PipePoint→相邻PipePoint[])
  - `markDirty(uuid)` → 沿依赖边传播脏标记
  - `collectDirty()` → 拓扑排序返回待重算对象列表
- `src/engine/RecomputeEngine.h/cpp`:
  - `recompute(dirtyObjects)` → 逐个调用 GeometryDeriver → 通知 SceneManager

**验收标准**:
- [ ] Document 增删查改正确
- [ ] 事务: open→修改→commit → 属性变更生效
- [ ] 事务: open→修改→abort → 属性不变
- [ ] undo: 恢复上一步状态
- [ ] redo: 恢复 undo 前状态
- [ ] 依赖传播: 修改 PipeSpec.OD → 引用它的 PipePoint 被标脏
- [ ] recompute: 脏对象的 Shape 正确重建
- [ ] 单元测试通过

---

### T17 — 工作台系统 + C++→QML 桥接

| 属性 | 值 |
|------|---|
| **对应** | Step 14 (部分) |
| **预计规模** | ~8 文件 |
| **前置依赖** | T16 |
| **所需技能** | Qt/QML, C++ |

**交付物**:
- `src/app/Workbench.h` — 工作台抽象基类
- `src/app/WorkbenchManager.h/cpp` — 注册/切换工作台
- `src/app/CadWorkbench.h/cpp` — CAD 工作台 (面板/工具定义)
- `src/app/SelectionManager.h/cpp` — 选择集管理 + selectionChanged 信号
- `src/ui/AppController.h/cpp` — QML 根控制器 (暴露 Document/Transaction/Selection)
- `src/ui/WorkbenchController.h/cpp` — 暴露 activePanels/toolbarActions 给 QML
- `src/main.cpp` — 应用入口 (创建所有管理器, 注册 QML 类型, 加载 main.qml)

**验收标准**:
- [ ] QML 引擎启动, main.qml 加载成功
- [ ] C++ 对象在 QML 中可访问
- [ ] 工作台切换发出信号, QML 响应
- [ ] SelectionManager 选中/取消选中正确通知

---

### T18 — QML 表格模型层

| 属性 | 值 |
|------|---|
| **对应** | Step 15 (部分) |
| **预计规模** | ~6 文件 |
| **前置依赖** | T17 |
| **所需技能** | Qt Model/View, QML |

**交付物**:
- `src/ui/PipePointTableModel.h/cpp` — QAbstractTableModel
  - 列: Name | X | Y | Z | Type | PipeSpec | bendMultiplier
  - Bend N/M/F 行: 灰色背景, 只读
  - setData → TransactionManager (open→set→commit)
- `src/ui/SegmentTreeModel.h/cpp` — QAbstractItemModel
  - 树结构: Route → Segment → PipePoint
  - 选中节点 → SelectionManager
- `src/ui/PropertyModel.h/cpp` — 属性面板数据模型
  - 根据选中对象类型返回不同分组字段
- `src/ui/PipeSpecModel.h/cpp` — PipeSpec 字段编辑模型

**验收标准**:
- [ ] 管点表格正确显示所有管点
- [ ] 编辑单元格 → 事务提交 → 模型数据更新
- [ ] Bend N/M/F 行只读
- [ ] 树形模型正确展示 Route→Segment→PipePoint
- [ ] 选中联动: 表格↔树↔3D

---

### T19 — QML UI 面板

| 属性 | 值 |
|------|---|
| **对应** | Step 15 (部分) |
| **预计规模** | ~15 文件 |
| **前置依赖** | T18 |
| **所需技能** | QML, UI 设计 |

**交付物**:

**样式**:
- `ui/style/Theme.qml` — 颜色/字体/间距常量

**可复用组件**:
- `ui/components/CollapsiblePanel.qml` — 可收缩面板
- `ui/components/SplitView.qml` — 可拖动分割视图
- `ui/components/IconButton.qml` — 图标按钮 (悬停提示)
- `ui/components/EditableCell.qml` — 可编辑表格单元格
- `ui/components/ContextMenu.qml` — 右键菜单

**面板**:
- `ui/main.qml` — 主窗口布局 (TopBar + StructureTree + Viewport3D + PipePointTable + PropertyPanel + StatusBar)
- `ui/panels/TopBar.qml` — 顶部栏 (工作台标签 + 图标工具栏)
- `ui/panels/StructureTree.qml` — 左侧结构树
- `ui/panels/Viewport3D.qml` — 3D 视口 (嵌入 VsgQuickItem + 视图按钮组)
- `ui/panels/PipePointTable.qml` — 管点表格
- `ui/panels/PropertyPanel.qml` — 属性面板 (5 分组, 根据类型动态渲染)
- `ui/panels/StatusBar.qml` — 状态栏

**对话框**:
- `ui/dialogs/NewProjectDialog.qml`
- `ui/dialogs/OpenProjectDialog.qml`

**验收标准**:
- [ ] 主窗口布局符合架构文档 §9.2 wireframe
- [ ] 面板可收缩/展开
- [ ] 分割条可拖动
- [ ] 属性面板按管点类型显示不同分组 (Run/Bend/Reducer/Tee/Valve/FlexJoint)
- [ ] 右键菜单弹出, "查看属性" 引导至属性面板
- [ ] 闪烁动画: 属性面板已展开时边框闪烁 (#0078D4, 300ms)
- [ ] 视觉风格符合 §9.4 规范
- [ ] 键盘快捷键: Ctrl+S/Z/Y/N/O, Delete

---

## Phase 6: 数据交换

### T20 — JSON 工程文件序列化

| 属性 | 值 |
|------|---|
| **对应** | 横切任务 T22 |
| **预计规模** | ~2 文件 |
| **前置依赖** | T05 + T06 |
| **所需技能** | C++, nlohmann/json |

**交付物**:
- `src/app/ProjectSerializer.h/cpp`:
  - `save(document, filePath)` → JSON 文件
  - `load(filePath)` → Document (反序列化所有对象 → 全量 recompute 重建几何)

**JSON 结构**:
```json
{
  "version": "0.1.0",
  "projectConfig": { "name": "...", "author": "...", ... },
  "pipeSpecs": [ { "id": "...", "name": "...", "fields": {...} } ],
  "routes": [
    {
      "id": "...", "name": "...",
      "segments": [
        {
          "id": "...", "segmentId": "A",
          "pipePoints": [
            { "id": "...", "name": "A00", "type": "Run", "position": [0,0,0], "pipeSpecId": "..." }
          ]
        }
      ]
    }
  ],
  "accessories": [ ... ],
  "beams": [ ... ]
}
```

**验收标准**:
- [ ] save → load → save → 两次 JSON 文件内容一致
- [ ] load 后全部对象引用关系正确 (PipePoint→PipeSpec, Accessory→PipePoint)
- [ ] load 后 recompute 能正确重建所有几何
- [ ] 文件大小合理 (30k 管点 < 50MB)

---

### T21 — STEP 导出

| 属性 | 值 |
|------|---|
| **对应** | Step 16 |
| **预计规模** | ~2 文件 |
| **前置依赖** | T08, T09, T04 (StepIO) |
| **所需技能** | OCCT STEP |

**交付物**:
- `src/app/StepExporter.h/cpp`:
  - `exportAll(document, filePath)`:
    遍历所有管点 → GeometryDeriver 生成 Shape → STEPControl_Writer 装配导出
  - 层级: Route → Segment → Component (STEP 产品层次)

**验收标准**:
- [ ] 导出 STEP → FreeCAD 打开, 几何正确
- [ ] 产品层次: Route/Segment 结构可见
- [ ] 10 个管点的管道段导出 < 5s

---

## 横切任务

以下任务贯穿多个 Phase，需要在各 Phase 中逐步完善。

### T22 — JSON 序列化 (= T20)

见 T20。随着 T05/T06 的对象增加逐步扩展序列化支持。

### T23 — 事务管理 (包含在 T16)

见 T16。需与 T18 (QML 表格模型) 集成——表格编辑触发事务。

### T24 — 选择管理 (包含在 T17)

见 T17。需与 T14 (拾取)、T18 (表格)、T19 (UI 面板) 集成——三方联动。

### T25 — 集成测试与端到端验证

| 属性 | 值 |
|------|---|
| **预计规模** | ~5 文件 |
| **前置依赖** | 全部任务 |
| **所需技能** | 全栈 |

**验收场景**:

1. **新建工程**: 启动应用 → 新建工程 → 工程名显示在标题栏
2. **管点输入**: 表格中输入管点 A00(0,0,0)→A01(1000,0,0)→A02(1000,1000,0) → 3D 视口实时渲染管段
3. **弯头**: 标记 A01 为 Bend → N/M/F 自动计算 → 3D 视口显示弯头
4. **PipeSpec 修改**: 修改 OD → 所有引用管点 3D 更新
5. **Undo/Redo**: Ctrl+Z → 回退 → Ctrl+Y → 重做
6. **保存/加载**: Ctrl+S → 关闭 → 重新打开 → 全部恢复
7. **STEP 导出**: 导出 → FreeCAD 打开确认

---

## 任务总表

| ID | 任务名 | Phase | 依赖 | 规模 | 核心技能 |
|----|--------|-------|------|------|---------|
| T01 | 构建系统搭建 | 1 | — | ~10 文件 | CMake, pixi |
| T02 | Foundation 层 | 1 | T01 | ~4 文件 | C++ |
| T03 | OCCT 几何封装 | 1 | T01 | ~6 文件 | OCCT BRep |
| T04 | OCCT 网格化 + STEP I/O | 1 | T01 | ~4 文件 | OCCT Mesh |
| T05 | 核心文档对象 | 2 | T02 | ~18 文件 | C++ OOP |
| T06 | 附属对象与梁 | 2 | T05 | ~12 文件 | C++ |
| T07 | 弯头几何计算器 | 3 | T05, T02 | ~2 文件 | 3D 几何算法 |
| T08 | 管件几何 (Run/Reducer/Tee) | 3 | T07, T03 | ~8 文件 | OCCT BRep |
| T09 | 管件几何 (Valve/Flex/Beam) | 3 | T03, T05 | ~8 文件 | OCCT BRep |
| T10 | 拓扑管理与约束 | 3 | T05 | ~4 文件 | 图算法 |
| T11 | OCCT→VSG 网格转换 | 4 | T04 | ~2 文件 | OCCT + VSG |
| T12 | VSG 场景管理 | 4 | T11 | ~8 文件 | VSG |
| T13 | 相机控制与场景基础设施 | 4 | T12 | ~4 文件 | VSG, 3D 数学 |
| T14 | 3D 拾取与高亮 | 4 | T12 | ~4 文件 | VSG |
| T15 | VSG-QML 桥接 | 5 | T12, T13 | ~2 文件 | Qt Quick, Vulkan |
| T16 | 应用层核心 | 5 | T05, T07 | ~8 文件 | C++ 架构 |
| T17 | 工作台 + QML 桥接 | 5 | T16 | ~8 文件 | Qt/QML |
| T18 | QML 表格模型层 | 5 | T17 | ~6 文件 | Qt Model/View |
| T19 | QML UI 面板 | 5 | T18 | ~15 文件 | QML |
| T20 | JSON 序列化 | 6 | T05, T06 | ~2 文件 | nlohmann/json |
| T21 | STEP 导出 | 6 | T08, T09, T04 | ~2 文件 | OCCT STEP |
| T25 | 集成测试 | — | 全部 | ~5 文件 | 全栈 |

---

## 建议执行顺序

```
Week 1:   T01 (构建系统)
Week 2:   T02 + T03 + T04 (并行: Foundation + OCCT封装)
Week 3:   T05 (核心文档对象)
Week 4:   T06 + T07 (并行: 附属对象 + 弯头计算器)
Week 5:   T08 + T09 + T10 + T11 (并行: 几何生成器 + 网格转换)
Week 6:   T12 + T16 (并行: 场景管理 + 应用层核心)
Week 7:   T13 + T14 + T20 (并行: 相机/拾取 + JSON序列化)
Week 8:   T15 + T17 (VSG-QML桥接 + 工作台)
Week 9:   T18 + T21 (表格模型 + STEP导出)
Week 10:  T19 (QML UI面板)
Week 11:  T25 (集成测试 + Bug修复)
```

---

## AI Agent 分配指南

每个 Task 可直接作为 AI Agent 的 prompt 上下文。分配时提供:

1. **本文档对应 Task 章节** — 交付物 + 验收标准
2. **架构文档对应章节** — 数据模型 + 接口约定
3. **前置 Task 的交付代码** — 已实现的头文件作为接口参考
4. **lib/ 下的 AGENTS.md** — 对应库的 API 使用指南

**示例 prompt 结构**:
```
你是一个C++开发者。请实现 T07 — 弯头几何计算器。

## 上下文
- 架构文档: [粘贴 §3.3 Bend 4管点模型]
- 依赖接口: [粘贴 foundation/Math.h 和 model/PipePoint.h 头文件]
- OCCT API指南: [粘贴 lib/occt/AGENTS.md]

## 交付
- src/engine/BendCalculator.h
- src/engine/BendCalculator.cpp
- tests/test_bend_calculator.cpp

## 验收标准
[粘贴 T07 验收标准列表]
```

---
---

# 二期开发计划 (Phase 2)

> **版本**: 0.2.0 | **日期**: 2026-03-29
>
> 一期 (T01-T25) 已全部完成。二期聚焦：参数化构件模板、ViewManager、三工作台拆分、载荷分析、VTK 集成。
>
> **编号即执行顺序**：严格从 T30 → T31 → ... → T45 依次执行，每个任务只依赖编号更小的任务。

---

## 二期执行顺序与依赖

```
执行顺序    任务                              依赖（均为编号更小的任务）
────────    ────                              ──────────────────────
T30         ViewManager 视图管理器             —（一期已完成）
T31         ComponentCatalog 参数化构件模板     —
T32         Load 载荷数据模型                  —
T33         LoadCase 与 LoadCombination        T32
T34         DesignWorkbench 工作台             T31
T35         SpecWorkbench 工作台               T31
T36         DesignTree + ParameterPanel 重构   T34
T37         OCCT→VTK 网格转换                  T32
T38         VTK 场景管理                       T37
T39         工作台切换 + QML 面板动态加载        T34, T35
T40         StatusBar + 右键菜单 + 框选         T36
T41         ComponentToolStrip 元件插入         T31, T36
T42         VTK-QML 桥接                       T38
T43         序列化扩展 (Load/LoadCase)          T33
T44         AnalysisWorkbench 工作台            T33, T39, T42
T45         端到端集成测试                      T41, T43, T44
```

---

## T30 — ViewManager 视图管理器

**Phase**: 8 — 核心重构  
**依赖**: T12(场景管理), T13(相机控制) — 已完成  
**推荐模型**: Sonnet  
**预计规模**: ~300 行  

**参考**: architecture.md §10.4

### 交付物
- `src/visualization/ViewManager.h`
- `src/visualization/ViewManager.cpp`
- `tests/test_view_manager.cpp`

### 接口定义
```cpp
class ViewManager {
    enum class ActiveViewport { VSG, VTK };
    enum class RenderMode { Solid, Wireframe, SolidWithEdges, Beam };
    enum class Category { PipePoints, Segments, Accessories, Supports, Beams, Annotations, LoadArrows, StressContour };
    enum class LodLevel { Draft, Normal, Fine };

    void setActiveViewport(ActiveViewport vp);
    void fitAll();
    void setViewPreset(ViewPreset preset);
    void saveViewState(const std::string& workbenchId);
    void restoreViewState(const std::string& workbenchId);
    void setRenderMode(RenderMode mode);
    void setCategoryVisible(Category cat, bool visible);
    void setGridVisible(bool visible);
    void setTriadVisible(bool visible);
    void setLodLevel(LodLevel level);
    gp_Pnt currentMouseWorldPos() const;
    bool captureImage(const std::string& path);
};
```

### 验收标准
1. 构造 ViewManager，注入 SceneManager + CameraController
2. setActiveViewport 切换活跃视口
3. fitAll/setViewPreset 转发给当前活跃视口的 CameraController
4. saveViewState/restoreViewState 正确缓存和恢复相机状态
5. setRenderMode/setCategoryVisible 状态管理正确
6. 单元测试全部通过

---

## T31 — ComponentCatalog 参数化构件模板库

**Phase**: 8 — 核心重构  
**依赖**: T08(管件几何), T09(阀门/附件几何) — 已完成  
**推荐模型**: **Opus**  
**预计规模**: ~800 行  

**参考**: architecture.md §3.7

### 交付物
- `src/engine/ComponentTemplate.h` — 模板基类
- `src/engine/ComponentCatalog.h` / `ComponentCatalog.cpp` — 注册表单例
- `src/engine/templates/PipeTemplate.h/cpp`
- `src/engine/templates/ElbowTemplate.h/cpp`
- `src/engine/templates/TeeTemplate.h/cpp`
- `src/engine/templates/ReducerTemplate.h/cpp`
- `src/engine/templates/GateValveTemplate.h/cpp`
- `src/engine/templates/WeldNeckFlangeTemplate.h/cpp`
- `src/engine/templates/RigidSupportTemplate.h/cpp`
- `src/engine/templates/SpringHangerTemplate.h/cpp`
- 更新 `GeometryDeriver` 改为通过 Catalog 查表
- `tests/test_component_catalog.cpp`

### 接口定义
```cpp
class ComponentTemplate {
    virtual std::string templateId() const = 0;
    virtual ComponentParams deriveParams(double od, double wt) const = 0;
    virtual TopoDS_Shape buildShape(const ComponentParams& p) const = 0;
};

class ComponentCatalog {
    static ComponentCatalog& instance();
    void registerTemplate(std::unique_ptr<ComponentTemplate> tpl);
    ComponentTemplate* getTemplate(const std::string& templateId) const;
    std::vector<std::string> allTemplateIds() const;
};
```

### 验收标准
1. 至少 8 种模板注册成功
2. 每种模板通过 OD=168.3mm 和 OD=323.8mm 两组参数验证几何生成
3. GeometryDeriver 通过 Catalog 查表生成几何，不再硬编码分发
4. 单元测试: 每种模板 buildShape 返回非空 Shape，体积 > 0

---

## T32 — Load 载荷数据模型

**Phase**: 9 — 载荷数据模型  
**依赖**: T05(核心文档对象) — 已完成  
**推荐模型**: Sonnet  
**预计规模**: ~500 行  

**参考**: architecture.md §6.9.1, §6.9.2

### 交付物
- `src/model/Load.h` / `Load.cpp` — 载荷基类
- `src/model/DeadWeightLoad.h/cpp`
- `src/model/ThermalLoad.h/cpp`
- `src/model/PressureLoad.h/cpp`
- `src/model/WindLoad.h/cpp`
- `src/model/SeismicLoad.h/cpp`
- `src/model/DisplacementLoad.h/cpp`
- `src/model/UserDefinedLoad.h/cpp`
- `tests/test_load_model.cpp`

### 接口定义
```cpp
class Load : public DocumentObject {
    virtual std::string loadType() const = 0;
    virtual std::vector<UUID> affectedObjects() const;
    void addAffectedObject(const UUID& id);
    void removeAffectedObject(const UUID& id);
};

class ThermalLoad : public Load {
    double installTemp() const;
    double operatingTemp() const;
    void setInstallTemp(double t);
    void setOperatingTemp(double t);
};
// ... 其他 6 种子类类似
```

### 验收标准
1. 7 种载荷子类均可构造，loadType() 返回正确字符串
2. affectedObjects 增删正确
3. 每种载荷的特有参数 getter/setter 正常
4. 所有载荷继承 DocumentObject 的 UUID 和 name
5. 单元测试全部通过

---

## T33 — LoadCase 与 LoadCombination

**Phase**: 9 — 载荷数据模型  
**依赖**: T32(Load 数据模型)  
**推荐模型**: Sonnet  
**预计规模**: ~400 行  

**参考**: architecture.md §6.9.3, §6.9.4, §6.9.5

### 交付物
- `src/model/LoadCase.h` / `LoadCase.cpp`
- `src/model/LoadCombination.h` / `LoadCombination.cpp`
- `tests/test_loadcase.cpp`

### 接口定义
```cpp
struct LoadEntry { UUID loadId; double factor; };

class LoadCase : public DocumentObject {
    const std::string& caseName() const;
    void addEntry(const LoadEntry& entry);
    void removeEntry(const UUID& loadId);
    const std::vector<LoadEntry>& entries() const;
};

enum class CombineMethod { Algebraic, Absolute, SRSS, Envelope };
enum class StressCategory { Sustained, Expansion, Occasional, Operating, Hydrotest };

struct CaseEntry { UUID caseId; double factor; };

class LoadCombination : public DocumentObject {
    StressCategory category() const;
    CombineMethod method() const;
    void addCaseEntry(const CaseEntry& entry);
    const std::vector<CaseEntry>& caseEntries() const;
};
```

### 验收标准
1. LoadCase 可添加/删除 LoadEntry，entries 正确
2. LoadCombination 可设置 CombineMethod 和 StressCategory
3. 支持 B31.3 典型组合配置 (SUS/EXP/OPE/OCC)
4. 依赖关系: LoadCombination → LoadCase → Load 严格 DAG
5. 单元测试全部通过

---

## T34 — DesignWorkbench 工作台实现

**Phase**: 10 — 工作台拆分  
**依赖**: T31(ComponentCatalog), T17(工作台+QML桥接) — 已完成  
**推荐模型**: Sonnet  
**预计规模**: ~300 行  

**参考**: architecture.md §6.5

### 交付物
- `src/app/DesignWorkbench.h` / `DesignWorkbench.cpp` — 替代 CadWorkbench
- 更新 `main.cpp` 注册 DesignWorkbench
- `tests/test_design_workbench.cpp`

### 接口定义
```cpp
class DesignWorkbench : public Workbench {
    std::string name() override;            // "Design"
    void activate(Document&) override;
    void deactivate(Document&) override;
    std::vector<ToolbarAction> toolbarActions() override;  // new-segment, add-point, measure, export-step
    std::vector<std::string> panelIds() override;          // DesignTree, Viewport3D, ComponentToolStrip, ParameterPanel
    ViewportType viewportType() override;   // Vsg
};
```

### 验收标准
1. DesignWorkbench 正确替代 CadWorkbench
2. toolbarActions 返回正确的 4 项
3. panelIds 返回 4 个面板 ID
4. viewportType 返回 Vsg
5. activate/deactivate 生命周期正确

---

## T35 — SpecWorkbench 工作台实现

**Phase**: 10 — 工作台拆分  
**依赖**: T31(ComponentCatalog), T17(工作台+QML桥接) — 已完成  
**推荐模型**: Sonnet  
**预计规模**: ~250 行  

**参考**: architecture.md §6.4

### 交付物
- `src/app/SpecWorkbench.h` / `SpecWorkbench.cpp`
- `tests/test_spec_workbench.cpp`

### 接口定义
```cpp
class SpecWorkbench : public Workbench {
    std::string name() override;            // "Specification"
    void activate(Document&) override;
    void deactivate(Document&) override;
    std::vector<ToolbarAction> toolbarActions() override;  // new-spec, import-code, add-material, add-component, validate
    std::vector<std::string> panelIds() override;          // SpecTree, MaterialTable, ComponentTable, PropertyPanel
    ViewportType viewportType() override;   // Vsg
};
```

### 验收标准
1. toolbarActions 返回 5 项
2. panelIds 返回 4 个面板 ID
3. activate/deactivate 生命周期正确

---

## T36 — DesignTree + ParameterPanel QML 重构

**Phase**: 13 — UI 完善  
**依赖**: T34(DesignWorkbench)  
**推荐模型**: Sonnet  
**预计规模**: ~500 行  

**参考**: architecture.md §9.2, §9.3

### 交付物
- `ui/panels/DesignTree.qml` — 左侧可折叠设计树
- `ui/panels/ParameterPanel.qml` — 右侧可折叠，含管点表格+属性面板+模式切换按钮
- `ui/panels/PropertyPanel.qml` — 动态属性面板（分组+编辑/只读模式切换）
- `ui/panels/PipePointTable.qml` — 管点表格（嵌入 ParameterPanel）
- 更新 `ui/main.qml` — DesignWorkbench 布局适配

### 验收标准
1. 设计树可折叠/展开，层级显示 Route→Segment→PipePoint→Accessory
2. 参数化面板可折叠/展开，独立于设计树
3. 属性面板支持编辑模式/只读模式切换（底部按钮）
4. 选中管点时属性面板动态切换内容
5. 管点表格可直接编辑坐标值

---

## T37 — OCCT→VTK 网格转换

**Phase**: 11 — VTK 可视化  
**依赖**: T04(OCCT 网格化) — 已完成, T32(Load 模型,用于 VTK 中显示载荷箭头)  
**推荐模型**: Sonnet  
**预计规模**: ~400 行  

**参考**: architecture.md §12 Step 20, lib/vtk/AGENTS.md

### 交付物
- `src/vtk-visualization/OcctToVtk.h` / `OcctToVtk.cpp`
- `src/vtk-visualization/BeamMeshBuilder.h` / `BeamMeshBuilder.cpp`
- `src/vtk-visualization/CMakeLists.txt`
- `tests/test_occt_to_vtk.cpp`

### 接口定义
```cpp
namespace vtk_vis {
    vtkSmartPointer<vtkPolyData> toVtkPolyData(const TopoDS_Shape& shape);
    vtkSmartPointer<vtkPolyData> buildBeamMesh(const std::vector<gp_Pnt>& centerline);
}
```

### 验收标准
1. TopoDS_Shape (圆柱/弯管) → vtkPolyData 转换成功，点数/面数 > 0
2. BeamMeshBuilder 从管路中心线生成 vtkPolyLine
3. 单元测试通过，不依赖 GPU

---

## T38 — VTK 场景管理

**Phase**: 11 — VTK 可视化  
**依赖**: T37(OCCT→VTK)  
**推荐模型**: Sonnet  
**预计规模**: ~400 行  

**参考**: architecture.md §8 vtk-visualization/

### 交付物
- `src/vtk-visualization/VtkSceneManager.h` / `VtkSceneManager.cpp`
- `tests/test_vtk_scene.cpp`

### 接口定义
```cpp
class VtkSceneManager {
    void addActor(const std::string& uuid, vtkSmartPointer<vtkActor> actor);
    void removeActor(const std::string& uuid);
    void updateActor(const std::string& uuid, vtkSmartPointer<vtkActor> actor);
    void setRenderMode(RenderMode mode);  // Solid ↔ Beam 切换 Actor 可见性
    vtkSmartPointer<vtkRenderer> renderer() const;
};
```

### 验收标准
1. 增删 Actor 正常工作
2. Solid/Beam 模式切换通过 Actor 可见性控制
3. renderer() 返回有效 vtkRenderer

---

## T39 — 工作台切换 + QML 面板动态加载

**Phase**: 10 — 工作台拆分  
**依赖**: T34(DesignWorkbench), T35(SpecWorkbench)  
**推荐模型**: **Opus**  
**预计规模**: ~500 行  

**参考**: architecture.md §6.7, §9.6

### 交付物
- 更新 `ui/panels/TopBar.qml` — 三工作台切换标签
- 更新 `ui/main.qml` — 根据 panelIds 动态加载/卸载面板
- 更新 `src/ui/WorkbenchController.h/cpp` — 桥接三工作台
- `tests/test_workbench_switch.cpp`

### 验收标准
1. TopBar 显示 Specification / Design / Analysis 三个标签
2. 切换工作台时工具栏自动更换
3. 切换工作台时面板动态加载/卸载（不崩溃）
4. Design → Analysis 切换时视口引擎从 VSG 切到 VTK（通过 ViewManager）
5. 文档数据在切换后保持不变

---

## T40 — StatusBar + 右键菜单 + 框选

**Phase**: 13 — UI 完善  
**依赖**: T36(DesignTree+ParameterPanel)  
**推荐模型**: Sonnet  
**预计规模**: ~400 行  

**参考**: architecture.md §9.6, §10.1.2, §10.3

### 交付物
- `ui/panels/StatusBar.qml` — 三区状态栏（选中对象信息 | 鼠标3D坐标 | 缩放）
- `ui/components/ContextMenu.qml` — 右键菜单（修改/查看/删除）
- 更新 `src/visualization/PickHandler` — 管点/管段选择 + 框选逻辑

### 验收标准
1. StatusBar 左区显示选中管件名称、类型、坐标
2. StatusBar 中区显示鼠标 3D 世界坐标
3. 右键菜单显示修改/查看/删除三个选项
4. 修改 → 参数化面板编辑模式，查看 → 只读模式
5. 左拖框选: 左→右 = Window，右→左 = Crossing

---

## T41 — ComponentToolStrip 元件插入工具条

**Phase**: 13 — UI 完善  
**依赖**: T31(ComponentCatalog), T36(DesignTree+ParameterPanel)  
**推荐模型**: Sonnet  
**预计规模**: ~300 行  

**参考**: architecture.md §6.5.1

### 交付物
- `ui/components/ComponentToolStrip.qml` — 双列图标条
- 更新 `src/ui/AppController` — 处理元件插入动作

### 验收标准
1. Fittings 列 6 个图标 + Accessories 列 5 个图标
2. 点击图标触发对应的管件/附件插入流程
3. Fittings 列贴近视口，Accessories 列贴近参数面板

---

## T42 — VTK-QML 桥接

**Phase**: 11 — VTK 可视化  
**依赖**: T38(VTK 场景管理)  
**推荐模型**: **Opus**  
**预计规模**: ~400 行  

**参考**: architecture.md §8, lib/vtk/AGENTS.md

### 交付物
- `src/vtk-visualization/VtkViewport.h` / `VtkViewport.cpp`
- `ui/panels/VtkViewport.qml`
- `tests/test_vtk_qml.cpp`

### 验收标准
1. VTK 渲染嵌入 QML 窗口，画面正确
2. 鼠标交互（旋转/平移/缩放）正常
3. AnalysisWorkbench 切换时 VTK 视口正确加载

---

## T43 — 序列化扩展 (Load/LoadCase)

**Phase**: 14 — 集成 & 序列化  
**依赖**: T33(LoadCase/Combo), T20(JSON序列化) — 已完成  
**推荐模型**: Sonnet  
**预计规模**: ~400 行  

**参考**: architecture.md §6.9

### 交付物
- 更新 `src/app/ProjectSerializer.h/cpp` — 支持 Load/LoadCase/LoadCombination 的 JSON 读写
- 更新 `src/app/DependencyGraph` — Load→LoadCase→LoadCombination 依赖链
- `tests/test_load_serialization.cpp`

### 验收标准
1. 7 种 Load 子类均可序列化/反序列化
2. LoadCase + LoadCombination round-trip 正确
3. 依赖图扩展: Load 变更 → 标脏 LoadCase → 标脏 LoadCombination
4. Undo/Redo 对载荷操作正确

---

## T44 — AnalysisWorkbench 工作台实现

**Phase**: 12 — AnalysisWorkbench  
**依赖**: T39(工作台切换), T42(VTK-QML桥接), T33(LoadCase/Combo)  
**推荐模型**: **Opus**  
**预计规模**: ~600 行  

**参考**: architecture.md §6.6

### 交付物
- `src/app/AnalysisWorkbench.h` / `AnalysisWorkbench.cpp`
- `ui/panels/AnalysisTree.qml`
- `ui/panels/LoadTable.qml`
- `ui/panels/LoadCaseTable.qml`
- `tests/test_analysis_workbench.cpp`

### 接口定义
```cpp
class AnalysisWorkbench : public Workbench {
    std::string name() override;            // "Analysis"
    ViewportType viewportType() override;   // Vtk
    void setRenderMode(RenderMode mode);
    RenderMode renderMode() const;
};
```

### 验收标准
1. AnalysisWorkbench 注册并可激活
2. toolbarActions 包含 toggle-render-mode, add-load, manage-loadcase 等
3. 分析树 (AnalysisTree) 显示 Loads/LoadCases/Combinations 三级树
4. LoadTable 和 LoadCaseTable 可编辑
5. Solid/Beam 渲染模式切换正常
6. 从 Design 切换到 Analysis 不崩溃

---

## T45 — 端到端集成测试

**Phase**: 14 — 集成 & 序列化  
**依赖**: T44(AnalysisWorkbench), T41(ComponentToolStrip), T43(序列化扩展)  
**推荐模型**: **Opus**  
**预计规模**: ~600 行  

### 交付物
- `tests/test_phase2_integration.cpp`

### 验收标准
1. **Spec→Design→Analysis 全流程**: 创建 PipeSpec → 设计管路(多个管点+弯头) → 添加载荷 → 创建工况组合
2. **工作台切换**: Design ↔ Analysis 来回切换，数据和视口状态正确恢复
3. **序列化 round-trip**: 保存→加载→所有对象(含载荷/工况)完整恢复
4. **Undo/Redo**: 跨工作台操作的事务回退正确
5. **渲染模式切换**: Solid ↔ Beam 切换不崩溃
6. **编译零错误零警告**
