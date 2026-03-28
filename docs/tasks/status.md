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
| T09 | 管件几何 (Valve/Flex/Beam) | `done` | T03, T05 | Sonnet | 2026-06-03 |
| T10 | 拓扑管理与约束 | `done` | T05 | Sonnet | 2026-03-28 |
| T11 | OCCT→VSG 网格转换 | `ready` | T04 | Sonnet | |
| T12 | VSG 场景管理 | `pending` | T11 | Sonnet | |
| T13 | 相机控制与场景基础设施 | `pending` | T12 | Sonnet | |
| T14 | 3D 拾取与高亮 | `pending` | T12 | Sonnet | |
| T15 | VSG-QML 桥接 | `pending` | T12, T13 | **Opus** | |
| T16 | 应用层核心 | `ready` | T05, T07 | **Opus** | |
| T17 | 工作台 + QML 桥接 | `pending` | T16 | Sonnet | |
| T18 | QML 表格模型层 | `pending` | T17 | Sonnet | |
| T19 | QML UI 面板 | `pending` | T18 | Sonnet | |
| T20 | JSON 序列化 | `ready` | T05, T06 | Sonnet | |
| T21 | STEP 导出 | `ready` | T08, T09, T04 | Sonnet | |
| T25 | 集成测试 | `pending` | 全部 | **Opus** | |

---

## 完成记录

> 每个任务完成时，AI 在此追加一条记录，包含：产出文件、关键接口、后续任务需要知道的信息。

<!-- === COMPLETION LOG START === -->
### T01 — 构建系统搭建 (2026-03-28)

**产出文件**:
- `pixi.toml`
- `CMakeLists.txt` (根)
- `src/CMakeLists.txt`
- `src/foundation/CMakeLists.txt` + `placeholder.cpp`
- `src/geometry/CMakeLists.txt` + `placeholder.cpp`
- `src/model/CMakeLists.txt` + `placeholder.cpp`
- `src/engine/CMakeLists.txt` + `placeholder.cpp`
- `src/visualization/CMakeLists.txt` + `placeholder.cpp`
- `src/app/CMakeLists.txt` + `placeholder.cpp`
- `src/ui/CMakeLists.txt` + `placeholder.cpp`
- `tests/CMakeLists.txt`
- `tests/test_build_system.cpp`
- `.gitignore`

**关键接口** (后续任务需要知道的):
```cmake
# 7 个 static library target:
# foundation / geometry / model / engine / visualization / app / ui
# 各层 include 路径统一为 ${CMAKE_SOURCE_DIR}/src，使用 "layer/Header.h" 方式包含

# OCCT 包含路径: ${OpenCASCADE_INCLUDE_DIR}
# OCCT 库变量:   ${OpenCASCADE_FoundationClasses_LIBRARIES}
#               ${OpenCASCADE_ModelingData_LIBRARIES}
#               ${OpenCASCADE_ModelingAlgorithms_LIBRARIES}
#               ${OpenCASCADE_DataExchange_LIBRARIES}
# VSG target:   vsg::vsg
# Qt6 targets:  Qt6::Quick  Qt6::Qml
# JSON target:  nlohmann_json::nlohmann_json
# GTest:        GTest::GTest  GTest::Main
```

**设计决策**:
- `vulkan-loader` / `vulkan-headers` 在 conda-forge linux-64 不可用，注释掉，由系统 RPM 包提供
- OCCT 已替换为 Debug 库，不需要 `CMAKE_MAP_IMPORTED_CONFIG_DEBUG` 配置映射
- pixi.toml 使用旧式 `[project]` 字段（有废弃警告但仍可用），可后续改为 `[workspace]`
- `configure-debug` task 需要传入 `-DCMAKE_PREFIX_PATH=$CONDA_PREFIX` 让 CMake 找到 pixi 安装的包

**已知限制**:
- pixi.toml 有两个警告：`[project]` 已废弃（建议改 `[workspace]`）、`[feature.dev]` 未被任何 env 使用
- `glslang` 和 `spirv-tools` 已加入 pixi 依赖（VSG 的传递依赖需要）

**后续任务注意**:
- T02/T03/T04 可以并行开始（均 `ready`）
- 新增 test executable 时，在 `tests/CMakeLists.txt` 加 `add_executable` + `target_link_libraries` + `add_test`
- 每层的 `.cpp` placeholder 文件在实现该层时替换（不需要保留）
### T02 — Foundation 层 (2026-03-28)

**产出文件**:
- `src/foundation/Types.h`
- `src/foundation/Math.h`
- `src/foundation/Signal.h`
- `src/foundation/Log.h`
- `src/foundation/placeholder.cpp` (更新为引用头文件)
- `tests/test_foundation.cpp`

**关键接口** (后续任务需要知道的):
```cpp
// foundation/Types.h
namespace foundation {
    struct UUID { uint8_t data[16]; static UUID generate(); std::string toString(); bool isNull(); };
    using Variant = std::variant<double, int, std::string>;
    double variantToDouble(const Variant&);
    int    variantToInt(const Variant&);
    const std::string& variantToString(const Variant&);
    enum class UnitSystem { SI, Imperial };
}

// foundation/Math.h
namespace foundation::math {
    constexpr double PI;
    double degToRad(double), radToDeg(double);
    struct Vec3 { double x,y,z; /* +,-,*,/ ops */ };
    double length(Vec3), dot(Vec3,Vec3), angleBetween(Vec3,Vec3);
    Vec3   normalize(Vec3), cross(Vec3,Vec3);
    std::optional<Vec3> lineLineIntersect(Vec3 p1, Vec3 d1, Vec3 p2, Vec3 d2);
}

// foundation/Signal.h
namespace foundation {
    template<typename... Args>
    class Signal { SlotId connect(Slot); void disconnect(SlotId); void emit(Args...); };
    using ChangeSignal = Signal<>;
}

// foundation/Log.h — 宏接口
LOG_DEBUG(msg); LOG_INFO(msg); LOG_WARN(msg); LOG_ERROR(msg);
```

**设计决策**:
- Foundation 层全为 header-only，placeholder.cpp 仅作为库非空占位
- `Vec3` 是轻量 3D 向量（不依赖 OCCT），供 T07 BendCalculator 使用
- `lineLineIntersect` 返回两最近点中点（真相交时即为交点），平行时返回 nullopt
- `Variant = std::variant<double,int,std::string>`，enum 值用 int 存储
- Signal 实现拷贝槽列表再触发，避免回调中 connect/disconnect 的迭代器失效

**后续任务注意**:
- T05 DocumentObject 使用 `foundation::UUID` 作为对象 ID
- T05 PipeSpec 的 fields 使用 `map<string, foundation::Variant>`
- T07 BendCalculator 使用 `foundation::math::Vec3` 和 `lineLineIntersect`
- 所有层 include: `#include "foundation/Types.h"` 等（相对于 `${CMAKE_SOURCE_DIR}/src`）
### T03 — OCCT 几何封装 (2026-03-28)

**产出文件**:
- `src/geometry/OcctTypes.h`
- `src/geometry/ShapeBuilder.h`
- `src/geometry/ShapeBuilder.cpp`
- `src/geometry/BooleanOps.h`
- `src/geometry/BooleanOps.cpp`
- `src/geometry/ShapeTransform.h`
- `src/geometry/ShapeTransform.cpp`
- `src/geometry/CMakeLists.txt` (更新，移除 placeholder.cpp)
- `tests/test_geometry.cpp`
- `tests/CMakeLists.txt` (更新，添加 test_geometry)

**关键接口** (后续任务需要知道的):
```cpp
// geometry/OcctTypes.h
namespace geometry { using OcctShape = TopoDS_Shape; }
// 包含: TopoDS_Shape.hxx, gp_Pnt/Vec/Dir/Ax1/Trsf.hxx, Standard_Handle.hxx

// geometry/ShapeBuilder.h
namespace geometry {
class ShapeBuilder {
    static TopoDS_Shape makeCylinder(double radius, double height);
    static TopoDS_Shape makeTorus(double majorR, double minorR, double angle = 2*PI);
    static TopoDS_Shape makeCone(double r1, double r2, double height);
    static TopoDS_Shape makePipeShell(const TopoDS_Wire& spine, double radius);
};
}

// geometry/BooleanOps.h
namespace geometry {
class BooleanOps {
    static TopoDS_Shape cut(const TopoDS_Shape& s1, const TopoDS_Shape& s2);  // s1-s2
    static TopoDS_Shape fuse(const TopoDS_Shape& s1, const TopoDS_Shape& s2); // s1∪s2
};
}

// geometry/ShapeTransform.h
namespace geometry {
class ShapeTransform {
    static TopoDS_Shape translate(const TopoDS_Shape& shape, const gp_Vec& vec);
    static TopoDS_Shape rotate(const TopoDS_Shape& shape, const gp_Ax1& axis, double angle);
    static TopoDS_Shape transform(const TopoDS_Shape& shape, const gp_Trsf& trsf);
};
}
```

**设计决策**:
- `ShapeBuilder::makePipeShell` 使用 `BRepOffsetAPI_MakePipeShell` + `Geom_Circle` 圆截面轮廓
- Boolean ops (`BRepAlgoAPI_Cut/Fuse`) 失败时返回 `TopoDS_Shape()`（空形体），调用方可用 `IsNull()` 检查
- `ShapeTransform::transform` 使用 `copy=true` 保证原始形体不被修改
- geometry 库链接 OCCT 四个分组：FoundationClasses, ModelingData, ModelingAlgorithms, DataExchange

**已知限制**:
- 目前无异常处理；OCCT 内部错误会直接抛出 `Standard_Failure`
- `makePipeShell` 仅支持圆截面，异形截面需额外实现

**后续任务注意**:
- T04 (OCCT 网格化 + STEP I/O) 可直接使用 `geometry` 库的 `TopoDS_Shape`
- T08/T09 管件几何需要使用 `ShapeBuilder` + `BooleanOps` + `ShapeTransform`
- 包含 geometry 头文件时需要 `${OpenCASCADE_INCLUDE_DIR}` in include path

### T04 — OCCT 网格化 + STEP I/O (2026-03-28)

**产出文件**:
- `src/geometry/ShapeMesher.h`
- `src/geometry/ShapeMesher.cpp`
- `src/geometry/StepIO.h`
- `src/geometry/StepIO.cpp`
- `src/geometry/ShapeProperties.h`
- `src/geometry/ShapeProperties.cpp`
- `src/geometry/CMakeLists.txt` (更新，添加 3 个新源文件)
- `tests/test_mesh_step.cpp`
- `tests/CMakeLists.txt` (更新，添加 test_mesh_step)

**关键接口** (后续任务需要知道的):
```cpp
// geometry/ShapeMesher.h
namespace geometry {
struct MeshData {
    std::vector<std::array<float,3>> vertices; // XYZ 顶点（世界坐标）
    std::vector<std::array<float,3>> normals;  // 归一化法线（与 vertices 一一对应）
    std::vector<uint32_t>            indices;  // 三角面索引（0-based，每组3个）
};
class ShapeMesher {
    static MeshData mesh(const TopoDS_Shape& shape, double deflection = 0.1);
};
}

// geometry/StepIO.h
namespace geometry {
class StepIO {
    static bool exportStep(const std::vector<TopoDS_Shape>& shapes, const std::string& filePath);
    static std::vector<TopoDS_Shape> importStep(const std::string& filePath);
};
}

// geometry/ShapeProperties.h
namespace geometry {
class ShapeProperties {
    static double volume(const TopoDS_Shape& shape);
    static double surfaceArea(const TopoDS_Shape& shape);
};
}
```

**设计决策**:
- `ShapeMesher::mesh` 使用 `BRepMesh_IncrementalMesh(shape, deflection, isRelative=false, angle=0.5)`
- 法线通过三角面叉积计算，累加到顶点后归一化（平均法线）
- 面法线方向由 TopoDS_Face orientation 决定（TopAbs_REVERSED → swap n2/n3）
- `TopLoc_Location::Transformation()` 返回 `const gp_Trsf&`（OCCT 8.0 直接提供）
- STEP export 使用 `STEPControl_AsIs` 模式，用 `STEPControl_Writer::Transfer` + `Write`
- STEP import 用 `TransferRoots()` 然后 `NbShapes()` + `Shape(i)` 枚举

**已知限制**:
- STEP 写入时会向 stdout 打印统计信息（OCCT 默认行为，无法静默）
- `ShapeMesher` 法线是平均法线，对于尖锐几何效果有限

**后续任务注意**:
- T11 (OCCT→VSG 网格转换) 直接使用 `ShapeMesher::mesh()` 获取 `MeshData`
- T21 (STEP 导出) 使用 `StepIO::exportStep()` 作为底层接口
- `MeshData.indices` 是 0-based，与 VSG vsg::uintArray 兼容

### T05 — 核心文档对象 (2026-03-28)

**产出文件**:
- `src/model/DocumentObject.h`
- `src/model/SpatialObject.h`
- `src/model/PropertyObject.h`
- `src/model/ContainerObject.h`
- `src/model/PipeSpec.h`
- `src/model/PipePoint.h`
- `src/model/ProjectConfig.h`
- `src/model/Segment.h`
- `src/model/Route.h`
- `src/model/placeholder.cpp` (更新，include 所有头文件)
- `tests/test_model.cpp`
- `tests/CMakeLists.txt` (更新，添加 test_model)

**关键接口** (后续任务需要知道的):
```cpp
// model/DocumentObject.h — 所有文档对象的基类
namespace model {
class DocumentObject {
    foundation::UUID id() const;      // 自动生成 v4 UUID
    std::string name() const;
    void setName(const std::string&); // 值变化时 emit changed
    foundation::ChangeSignal changed; // 变更信号
    virtual ~DocumentObject();        // 虚析构
};
}

// model/SpatialObject.h — 带 3D 坐标的对象
class SpatialObject : public DocumentObject {
    gp_Pnt position() const;
    void setPosition(const gp_Pnt&);  // gp_Pnt::IsEqual(pos, 1e-12) 去重
};

// model/PropertyObject.h — 带可扩展字段字典的对象
class PropertyObject : public DocumentObject {
    bool hasField(const std::string& key) const;
    foundation::Variant field(const std::string& key) const;
    void setField(const std::string& key, const foundation::Variant& value);
    void removeField(const std::string& key);
    std::map<std::string, foundation::Variant> fields() const;
};

// model/ContainerObject.h — 管理子对象集合
class ContainerObject : public DocumentObject {
    void addChild(std::shared_ptr<DocumentObject> child);
    bool removeChild(const foundation::UUID& id);
    std::shared_ptr<DocumentObject> findChild(const foundation::UUID& id) const;
    size_t childCount() const;
};

// model/PipePoint.h — 管道拓扑节点（核心领域对象）
enum class PipePointType { Run, Bend, Reducer, Tee, Valve, FlexJoint };
class PipePoint : public SpatialObject {
    PipePointType type() const;
    void setType(PipePointType);
    std::shared_ptr<PipeSpec> pipeSpec() const;
    void setPipeSpec(std::shared_ptr<PipeSpec>);
    std::map<std::string, foundation::Variant> typeParams() const;
    void setTypeParam(const std::string& key, const foundation::Variant& value);
};

// model/PipeSpec.h — 管道规格
class PipeSpec : public PropertyObject {
    double od() const;            void setOd(double);
    double wallThickness() const; void setWallThickness(double);
    std::string material() const; void setMaterial(const std::string&);
};

// model/Segment.h — 有序 PipePoint 序列
class Segment : public ContainerObject {
    void addPoint(std::shared_ptr<PipePoint>);
    void insertPoint(size_t index, std::shared_ptr<PipePoint>);
    bool removePoint(const foundation::UUID& id);
    std::shared_ptr<PipePoint> pointAt(size_t index) const;
    size_t pointCount() const;
};

// model/Route.h — Segment 集合
class Route : public ContainerObject {
    void addSegment(std::shared_ptr<Segment>);
    bool removeSegment(const foundation::UUID& id);
    std::shared_ptr<Segment> segmentAt(size_t index) const;
    size_t segmentCount() const;
};

// model/ProjectConfig.h — 项目配置
class ProjectConfig : public PropertyObject {
    std::string projectName() const; void setProjectName(const std::string&);
    std::string author() const;      void setAuthor(const std::string&);
    std::string standard() const;    void setStandard(const std::string&);
    foundation::UnitSystem unitSystem() const;
    void setUnitSystem(foundation::UnitSystem);
};
```

**设计决策**:
- 全部 header-only 实现，`placeholder.cpp` 仅作为编译验证
- 继承层次: DocumentObject → SpatialObject / PropertyObject / ContainerObject → 领域类
- PipePoint 使用 `map<string, Variant> typeParams_` 存储类型特定参数（如 bendMultiplier, valveType）
- Signal 只在值真正变化时才触发（setName/setPosition/setType 均有去重判断）
- PipePoint 持有 `shared_ptr<PipeSpec>` 引用，多个 PipePoint 可共享同一规格
- 拷贝构造函数已删除（DocumentObject 不可复制）

**已知限制**:
- ContainerObject 的子对象管理是简单 vector，大量子对象时 removeChild 为 O(n)
- 目前无序列化支持（T20 JSON 序列化将添加）
- PipePoint.typeParams 无 schema 校验（依赖调用方正确设置）

**后续任务注意**:
- T06 (附属对象与梁) 需继承 DocumentObject/SpatialObject 体系
- T07 (弯头几何计算器) 需读取 PipePoint 的 type()==Bend 及 typeParams["bendMultiplier"]
- T09 (Valve/Flex/Beam) 需读取 PipePoint 的 type() 和对应 typeParams
- T10 (拓扑管理) 需操作 Segment/Route 的增删改查接口
- T16 (应用层核心) 需使用 ProjectConfig + Route + Segment + PipePoint 构建文档模型
- T20 (JSON 序列化) 需遍历所有对象的 fields/typeParams/children

### T07 — 弯头几何计算器 (2026-03-28)

**产出文件**:
- `src/engine/BendCalculator.h`
- `src/engine/BendCalculator.cpp`
- `src/engine/CMakeLists.txt` (更新，placeholder.cpp→BendCalculator.cpp)
- `tests/test_engine.cpp`
- `tests/CMakeLists.txt` (更新，添加 test_engine)

**关键接口** (后续任务需要知道的):
```cpp
// engine/BendCalculator.h
namespace engine {

struct BendResult {
    gp_Pnt nearPoint;   // N点 — 弯弧起点
    gp_Pnt midPoint;    // M点 — 弯弧中点
    gp_Pnt farPoint;    // F点 — 弯弧终点
    gp_Pnt arcCenter;   // 弯弧圆心
    double bendAngle;   // 弯曲角度 (rad), 范围 (0, π)
    double bendRadius;  // 弯曲半径 (mm) = OD × multiplier
};

class BendCalculator {
    static std::optional<BendResult> calculateBend(
        const gp_Pnt& prevPoint,       // A05
        const gp_Pnt& intersectPoint,  // A06 (交点)
        const gp_Pnt& nextPoint,       // A07
        double outerDiameter,          // OD (mm)
        double bendMultiplier          // 1.5 | 2.0 | 5.0
    );
};

} // namespace engine
```

**设计决策**:
- θ = angle(d1, d2) 其中 d1=normalize(A06-A05), d2=normalize(A07-A06)；等价于 π - 顶点内角
- tangentLength = R × tan(θ/2)；N = A06 - d1×tanLen, F = A06 + d2×tanLen
- 弧圆心通过弯弧平面法线(d1×d2)和 d1 的垂直方向计算：C = N + cross(normal, d1) × R
- 弧中点 M = C + normalize(A06 - C) × R（利用交点在弧角平分方向上的几何性质）
- θ < 1e-6 rad（直线）或 θ > π-1e-6 rad（U-turn）或输入点重合时返回 nullopt
- 使用 OCCT gp_Pnt/gp_Vec 直接计算，不依赖 foundation::math::Vec3

**已知限制**:
- 不支持 θ = 0（直线）和 θ = π（U-turn）退化情况
- 不生成 OCCT BRep 几何（弯头 3D 实体由 T08 实现）
- 未考虑管壁厚度对弯弧偏移的影响

**后续任务注意**:
- T08 (Run/Reducer/Tee) 使用 BendResult.nearPoint/farPoint 确定直管段端点
- T08 需要 BendResult 来生成弯头 BRep（TopoDS_Shape）：用 arcCenter + R + bendAngle 构建 torus 截段
- T16 (应用层核心) 调用 BendCalculator::calculateBend() 进行几何推导
- BendResult 的 N/F 点是管段中心线上的切点，管件实体的实际端面由 OD/wallThickness 决定

### T08 — 管件几何生成器 Run/Reducer/Tee (2026-03-28)

**产出文件**:
- `src/engine/RunBuilder.h/.cpp`
- `src/engine/BendBuilder.h/.cpp`
- `src/engine/ReducerBuilder.h/.cpp`
- `src/engine/TeeBuilder.h/.cpp`
- `src/engine/GeometryDeriver.h/.cpp`
- `src/engine/CMakeLists.txt` (更新，添加 5 个 .cpp)
- `tests/test_pipe_geometry.cpp`
- `tests/CMakeLists.txt` (更新，添加 test_pipe_geometry)

**关键接口** (后续任务需要知道的):
```cpp
// engine/RunBuilder.h
namespace engine {
class RunBuilder {
    static TopoDS_Shape build(
        const gp_Pnt& startPoint,
        const gp_Pnt& endPoint,
        double outerDiameter,  // mm
        double wallThickness); // mm
};
}

// engine/BendBuilder.h
namespace engine {
class BendBuilder {
    static TopoDS_Shape build(
        const BendResult& bendResult,
        double outerDiameter,
        double wallThickness);
};
}

// engine/ReducerBuilder.h
namespace engine {
class ReducerBuilder {
    static TopoDS_Shape build(
        const gp_Pnt& startPoint,
        const gp_Pnt& endPoint,
        double startOD, double endOD,
        double wallThickness);
};
}

// engine/TeeBuilder.h
namespace engine {
class TeeBuilder {
    static TopoDS_Shape build(
        const gp_Pnt& mainStart, const gp_Pnt& mainEnd,
        const gp_Pnt& branchPoint, const gp_Pnt& branchEnd,
        double mainOD, double branchOD,
        double wallThickness);
};
}

// engine/GeometryDeriver.h
namespace engine {
class GeometryDeriver {
    static TopoDS_Shape deriveGeometry(
        const gp_Pnt& prevPoint,
        const std::shared_ptr<model::PipePoint>& current,
        const gp_Pnt& nextPoint);
};
}
```

**设计决策**:
- RunBuilder: 用 `BRepPrimAPI_MakeCylinder(ax, r, length)` 直接在方向轴上构建，避免变换开销
- BendBuilder: 使用 `BRepPrimAPI_MakeTorus(ax, majorR, minorR, angle)` 截段，轴坐标系中 X 轴对齐 arcCenter→nearPoint
- ReducerBuilder: 用 `BRepPrimAPI_MakeCone(ax, r1, r2, length)` 构建；内外锥 cut 得到锥壳
- TeeBuilder: 先 fuse 实心外圆柱，再 fuse 实心内圆柱，最后 cut — 避免直接 fuse 薄壳导致 OCCT 计算超时
- GeometryDeriver: 按 `PipePoint::type()` 分发；Reducer 的 endOD 从 typeParams["endOD"] 读取；Tee 的支管端点从 typeParams["branchEndX/Y/Z"] 读取

**已知限制**:
- TeeBuilder 的 boolean fuse 在 Debug 构建中每次约 1s（Release 会快很多）
- GeometryDeriver 对 Valve/FlexJoint 类型返回空 Shape（由 T09 实现）
- TeeBuilder 中 branchPoint 应在主管中心线上，否则几何可能不正确
- 测试用较小几何尺寸(60mm OD)以保证 Debug 下 boolean 操作可接受耗时

**后续任务注意**:
- T09 (Valve/Flex/Beam) 在 GeometryDeriver.cpp 的 switch 中添加 Valve/FlexJoint/Beam case
- T21 (STEP 导出) 调用上述 builder 得到 TopoDS_Shape，再用 StepIO::exportStep()
- T16 (应用层核心) 通过 GeometryDeriver::deriveGeometry() 为每个 PipePoint 生成几何体

### T09 — 管件几何 (Valve/FlexJoint/Beam/Accessory) (2026-06-03)

**产出文件**:
- `src/engine/ValveBuilder.h/.cpp`
- `src/engine/FlexJointBuilder.h/.cpp`
- `src/engine/BeamBuilder.h/.cpp`
- `src/engine/AccessoryBuilder.h/.cpp`
- `src/engine/GeometryDeriver.cpp` (更新，添加 Valve/FlexJoint case)
- `src/engine/CMakeLists.txt` (更新，添加 4 个新 .cpp)
- `tests/test_valve_flex_beam.cpp` (19 个测试)
- `tests/CMakeLists.txt` (更新，添加 test_valve_flex_beam)

**关键接口** (后续任务需要知道的):
```cpp
// engine/ValveBuilder.h
namespace engine {
class ValveBuilder {
    static TopoDS_Shape build(
        const gp_Pnt& startPoint,
        const gp_Pnt& endPoint,
        double outerDiameter,
        double wallThickness,
        const std::string& valveType = "gate"); // "gate"/"ball"/"check"
};
}

// engine/FlexJointBuilder.h
namespace engine {
class FlexJointBuilder {
    static TopoDS_Shape build(
        const gp_Pnt& startPoint,
        const gp_Pnt& endPoint,
        double outerDiameter,
        double wallThickness,
        int segmentCount = 3); // 波纹段数
};
}

// engine/BeamBuilder.h
namespace engine {
class BeamBuilder {
    enum class SectionType { Rectangular, HSection };
    static TopoDS_Shape build(
        const gp_Pnt& startPoint,
        const gp_Pnt& endPoint,
        SectionType sectionType,
        double width,
        double height);
};
}

// engine/AccessoryBuilder.h
namespace engine {
class AccessoryBuilder {
    static TopoDS_Shape buildFlange(
        const gp_Pnt& center, const gp_Dir& normal,
        double pipeDiameter, double thickness);
    static TopoDS_Shape buildBracket(
        const gp_Pnt& base, const gp_Pnt& top, double width);
};
}
```

**设计决策**:
- ValveBuilder: 3 段实心圆柱（前短截+阀体+后短截）fuse 后 cut 内腔；gate 阀体直径 1.8×OD，ball/check 为 1.6×OD
- FlexJointBuilder: 交替扩缩锥体（BRepPrimAPI_MakeCone）fuse 生成波纹外形，再 cut 内径圆柱；膨胀比 1.5×OD
- BeamBuilder: 用 BRepBuilderAPI_MakeFace + BRepPrimAPI_MakePrism 挤出截面;H 型截面由 12 点闭合 Wire 生成
- AccessoryBuilder: 法兰 = BRepPrimAPI_MakeCylinder 平面圆盘；支架 = 正方形截面挤压
- GeometryDeriver: Valve 从 typeParams["valveType"] 读取阀型，FlexJoint 从 typeParams["segmentCount"] 读取段数
- BeamBuilder 和 AccessoryBuilder 作为独立工具类，不通过 GeometryDeriver 分发

**已知限制**:
- ValveBuilder 3 段 fuse 在法兰边界处有共面拓扑；OCCT fuse 通常能处理，但复杂情况可能有微小间隙
- FlexJointBuilder 波纹数 >5 时 Debug 下 fuse 耗时增加（每段约 80-100ms）
- BeamBuilder 仅支持直梁（BRepPrimAPI_MakePrism 为平移挤出，不支持弯曲梁）

**后续任务注意**:
- T21 (STEP 导出) 现在可以使用所有 9 种管件 builder
- T06 (附属对象) 如需要几何表示，可调用 AccessoryBuilder::buildFlange/buildBracket
- GeometryDeriver 已完整覆盖所有 PipePointType（Run/Bend/Reducer/Tee/Valve/FlexJoint）

### T06 — 附属对象与梁 (2026-03-28)

**产出文件**:
- `src/model/Accessory.h`
- `src/model/FixedPoint.h`
- `src/model/Support.h`
- `src/model/Flange.h`
- `src/model/Gasket.h`
- `src/model/SealRing.h`
- `src/model/Beam.h`
- `src/model/PipePoint.h` (更新，添加 accessory 列表管理)
- `src/model/placeholder.cpp` (更新，include 新头文件)
- `tests/test_accessory_beam.cpp` (36 个测试)
- `tests/CMakeLists.txt` (更新，添加 test_accessory_beam)

**关键接口** (后续任务需要知道的):
```cpp
// model/Accessory.h — 附属构件基类
namespace model {
class Accessory : public SpatialObject {
    std::shared_ptr<PipePoint> pipePoint() const; // 关联的管点 (weak_ptr)
    void attachTo(std::shared_ptr<PipePoint> pt);
    void detach();
    const gp_Vec& offset() const;
    void setOffset(const gp_Vec& offset);
};
}

// model/FixedPoint.h — 固定点
class FixedPoint : public Accessory { bool isFixed() const; /* always true */ };

// model/Support.h — 支架
enum class SupportType { Rod, Spring, Rigid, Guide };
class Support : public Accessory {
    SupportType supportType() const; void setSupportType(SupportType);
    const gp_Dir& loadDirection() const; void setLoadDirection(const gp_Dir&);
};

// model/Flange.h — 法兰
class Flange : public Accessory {
    const std::string& rating() const;   void setRating(const std::string&);
    const std::string& faceType() const; void setFaceType(const std::string&);
    int boltHoleCount() const;           void setBoltHoleCount(int);
};

// model/Gasket.h — 垫片
class Gasket : public Accessory {
    const std::string& gasketMaterial() const; void setGasketMaterial(const std::string&);
    double thickness() const;                  void setThickness(double);
};

// model/SealRing.h — 密封圈
class SealRing : public Accessory {
    const std::string& sealMaterial() const;  void setSealMaterial(const std::string&);
    double crossSectionDiameter() const;      void setCrossSectionDiameter(double);
};

// model/Beam.h — 梁
enum class BeamSectionType { Rectangular, HSection };
class Beam : public SpatialObject {
    std::shared_ptr<PipePoint> startPoint() const; void setStartPoint(std::shared_ptr<PipePoint>);
    std::shared_ptr<PipePoint> endPoint() const;   void setEndPoint(std::shared_ptr<PipePoint>);
    BeamSectionType sectionType() const;           void setSectionType(BeamSectionType);
    double width() const;  void setWidth(double);
    double height() const; void setHeight(double);
    double length() const; // computed = startPoint.Distance(endPoint)
};

// model/PipePoint.h — 新增 accessory 管理
class PipePoint : public SpatialObject {
    // ... existing ...
    void addAccessory(std::shared_ptr<DocumentObject> acc);
    bool removeAccessory(const foundation::UUID& accId);
    const std::vector<std::shared_ptr<DocumentObject>>& accessories() const;
    size_t accessoryCount() const;
};
```

**设计决策**:
- 全部 header-only 实现，遵循 T05 既有模式
- Accessory 使用 weak_ptr<PipePoint> 引用管点，避免循环引用
- Beam 使用 weak_ptr<PipePoint> 引用双端管点，length() 实时计算距离
- PipePoint 的 accessory 列表存储为 vector<shared_ptr<DocumentObject>>，避免 Accessory.h ↔ PipePoint.h 循环 include
- 所有 setter 在值未变化时不触发 changed 信号（与 T05 一致）
- Beam 默认截面：Rectangular, 100mm 宽 × 200mm 高

**已知限制**:
- PipePoint 的 accessory 列表存储 DocumentObject 而非 Accessory 类型（caller 需 dynamic_cast）
- Accessory.attachTo() 不自动调用 PipePoint.addAccessory()，需调用方手动管理双向关联
- Beam.position 与端点位置独立，不自动更新

**后续任务注意**:
- T20 (JSON 序列化) 需遍历新增对象的字段：Accessory/FixedPoint/Support/Flange/Gasket/SealRing/Beam
- T09 的 engine/AccessoryBuilder 和 engine/BeamBuilder 已提供几何生成，可与这些 model 对象配合使用
- 所有新类继承 DocumentObject，有唯一 UUID 和 changed 信号

### T10 — 拓扑管理与约束 (2026-03-28)

**产出文件**:
- `src/engine/TopologyManager.h`
- `src/engine/TopologyManager.cpp`
- `src/engine/ConstraintSolver.h`
- `src/engine/ConstraintSolver.cpp`
- `src/engine/PipelineValidator.h`
- `src/engine/PipelineValidator.cpp`
- `src/engine/CMakeLists.txt` (更新：添加 3 个新 .cpp，添加 OCCT include 路径，链接 ModelingAlgorithms)
- `tests/test_topology.cpp` (22 个测试)
- `tests/CMakeLists.txt` (更新，添加 test_topology)

**关键接口** (后续任务需要知道的):
```cpp
// engine/TopologyManager.h
namespace engine {
class TopologyManager {
    // 追加管点；Tee 类型自动创建分支 Segment 并返回其 shared_ptr
    std::shared_ptr<model::Segment> appendPoint(
        model::Route&, model::Segment&, std::shared_ptr<model::PipePoint>);
    // 插入管点；Tee 同上
    std::shared_ptr<model::Segment> insertPoint(
        model::Route&, model::Segment&, std::size_t index, std::shared_ptr<model::PipePoint>);
    // 删除管点；Tee 同时删除其分支 Segment；空 Segment 自动清理（保留≥1个）
    bool removePoint(model::Route&, const foundation::UUID& pointId);
    // 查询包含该管点的所有 Segment（原始指针）
    std::vector<model::Segment*> segmentsContaining(const model::Route&, const foundation::UUID&) const;
    // 查询 Tee 对应分支 Segment 的 ID 字符串
    std::string branchSegmentId(const foundation::UUID& teeId) const;
};
}

// engine/ConstraintSolver.h
namespace engine {
struct ConstraintError { std::string pointId; std::string message; };
class ConstraintSolver {
    // 检查段内相邻非 Reducer 点的 OD 一致性
    std::vector<ConstraintError> checkDiameterConsistency(const model::Segment&) const;
    // 检查段内 Bend 点的角度合法性 (0° < bendAngle < 180°)
    std::vector<ConstraintError> checkBendAngles(const model::Segment&) const;
    // 对整条管路运行所有约束检查
    std::vector<ConstraintError> checkAll(const model::Route&) const;
};
}

// engine/PipelineValidator.h
namespace engine {
struct ValidationWarning {
    enum class Severity { Warning, Error };
    Severity severity; std::string objectId; std::string message;
};
class PipelineValidator {
    // 管段少于 2 个管点时报 Warning
    std::vector<ValidationWarning> checkUnconnectedPorts(const model::Route&) const;
    // 逐对检查形体最小距离，< tolerance 时报 Error（使用 BRepExtrema_DistShapeShape）
    std::vector<ValidationWarning> checkInterference(
        const std::vector<TopoDS_Shape>&, const std::vector<std::string>& objectIds,
        double tolerance = 1e-3) const;
    // 运行所有结构校验（不含干涉检测）
    std::vector<ValidationWarning> validateAll(const model::Route&) const;
};
}
```

**设计决策**:
- TopologyManager 内部用 `unordered_map<string, string>` 维护 Tee UUID→分支 Segment UUID 映射
- 创建分支 Segment 时命名为 "Branch_<TeeName>"，空分支（无管点）由调用方按需填充
- ConstraintSolver 用 `gp_Vec` 计算三点夹角，bendAngle ≠ theta（夹角）：bendAngle = π - theta
- 相邻点含 Reducer 时跳过 OD 检查（Reducer 负责异径过渡），未设置 PipeSpec 时亦跳过
- PipelineValidator 干涉检测需要预计算形体列表，结构校验（checkUnconnectedPorts）无需 OCCT
- engine CMakeLists.txt 现在 PUBLIC 暴露 `${OpenCASCADE_INCLUDE_DIR}`，并 PRIVATE 链接 ModelingAlgorithms

**已知限制**:
- TopologyManager 不维护 Segment→Tee 的反向映射，如需查询分支父节点需全量扫描
- 删除 Tee 时同时删除整个分支 Segment（含其下所有管点），不可部分保留
- ConstraintSolver 弯角检测依赖管点坐标，坐标未更新时结果可能无效

**后续任务注意**:
- T16 (应用层核心) 可直接使用 `TopologyManager` 作为文档操作的底层，管理 Route/Segment 一致性
- T21 (STEP 导出) 生成形体列表后可调用 `PipelineValidator::checkInterference()` 做校验
- `ConstraintSolver::checkAll()` 是管道设计校验的主要入口，T16 可在提交时调用
- engine 的 CMakeLists 现已添加 OCCT 包含路径（PUBLIC），T21/T16 等的 engine 下游无需再次添加

<!-- === COMPLETION LOG END === -->
