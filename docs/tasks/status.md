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

### T11 — OCCT→VSG 网格转换 (2026-03-28)

**产出文件**:
- `src/visualization/OcctToVsg.h`
- `src/visualization/OcctToVsg.cpp`
- `src/visualization/CMakeLists.txt` (更新：新增 OcctToVsg.cpp)
- `tests/test_visualization.cpp` (6 个测试全部通过)
- `tests/CMakeLists.txt` (更新：新增 test_visualization)

**关键接口** (后续任务需要知道的):
```cpp
namespace visualization {

/// OCCT Shape → VSG VertexIndexDraw（顶点 + 法线 + 索引）
/// binding 0 = vsg::vec3Array（顶点坐标）
/// binding 1 = vsg::vec3Array（法线向量）
/// 索引类型: vsg::uintArray（uint32_t）
/// 空形体或三角化失败返回 nullptr
vsg::ref_ptr<vsg::VertexIndexDraw> toVsgGeometry(const TopoDS_Shape& shape,
                                                  double deflection = 0.1);

} // namespace visualization
```

**设计决策**:
- MeshData → vsg::vec3Array (顶点/法线) + vsg::uintArray (索引)，直接调用 ShapeMesher::mesh()
- 使用 `assignArrays({vertices, normals})` 和 `assignIndices(indices)` 组装 VertexIndexDraw
- 索引使用 uint32_t（vsg::uintArray），支持超过 65535 顶点的大网格
- 空形体判断：mesh.vertices.empty() || mesh.indices.empty() 时返回 nullptr

**已知限制**:
- VertexIndexDraw 不含材质/着色器绑定，T12 需要在外层 StateGroup 提供 Pipeline 和 DescriptorSet
- 本阶段仅做 CPU 端数据组装，GPU buffer 上传在 compile() 时发生（需要 Vulkan Context）

**后续任务注意**:
- T12 (VSG 场景管理) 使用 `toVsgGeometry()` 创建几何节点，表面材质需在外层 StateGroup 设置
- visualization 库链接：`target_link_libraries(your_target visualization)` 即可，已传递 vsg::vsg + geometry
- `vid->arrays[0]->data` 是顶点 vec3Array, `vid->arrays[1]->data` 是法线 vec3Array

### T12 — VSG 场景管理 (2026-03-28)

**产出文件**:
- `src/visualization/PipePointNode.h`
- `src/visualization/PipePointNode.cpp`
- `src/visualization/ComponentNode.h`
- `src/visualization/ComponentNode.cpp`
- `src/visualization/LodStrategy.h`
- `src/visualization/LodStrategy.cpp`
- `src/visualization/SceneManager.h`
- `src/visualization/SceneManager.cpp`
- `src/visualization/CMakeLists.txt` (更新：新增 4 个 .cpp)
- `tests/test_scene_manager.cpp` (29 个测试)
- `tests/CMakeLists.txt` (更新：新增 test_scene_manager)

**关键接口** (后续任务需要知道的):
```cpp
// visualization/PipePointNode.h
namespace visualization {
// 创建管点 VSG 节点（小正方体，手动构建 VertexIndexDraw，无 GPU 依赖）
vsg::ref_ptr<vsg::MatrixTransform> createPipePointNode(
    double x, double y, double z,
    float size = 20.0f,
    const vsg::vec4& color = {0.9f, 0.5f, 0.1f, 1.0f});
}

// visualization/ComponentNode.h
namespace visualization {
// 创建管件 VSG 节点：MatrixTransform → StateGroup(空) → VertexIndexDraw
vsg::ref_ptr<vsg::MatrixTransform> createComponentNode(
    vsg::ref_ptr<vsg::VertexIndexDraw> vid,
    const vsg::dmat4& matrix = vsg::dmat4{});
}

// visualization/LodStrategy.h
namespace visualization {
struct LodLevels { double highDetailRatio = 0.05; double lowDetailRatio = 0.0; };
// 创建两级 VSG LOD 节点，以包围球中心+半径做视锥剔除
vsg::ref_ptr<vsg::LOD> createLodNode(
    vsg::ref_ptr<vsg::Node> highDetail,
    vsg::ref_ptr<vsg::Node> lowDetail,
    const vsg::dvec3& center, double radius,
    const LodLevels& levels = {});
}

// visualization/SceneManager.h
namespace visualization {
class SceneManager {
    SceneManager();
    vsg::ref_ptr<vsg::Group> root() const;    // 场景根节点
    void addNode(const std::string& uuid, vsg::ref_ptr<vsg::Node> node);
    bool removeNode(const std::string& uuid);
    bool updateNode(const std::string& uuid, vsg::ref_ptr<vsg::Node> newNode);
    void batchUpdate(const std::vector<std::pair<std::string, vsg::ref_ptr<vsg::Node>>>& updates);
    vsg::ref_ptr<vsg::Node> findNode(const std::string& uuid) const;
    bool hasNode(const std::string& uuid) const;
    size_t nodeCount() const;
};
}
```

**设计决策**:
- `PipePointNode` 手动构建 24 顶点 / 36 索引的正方体 VertexIndexDraw（不使用 vsg::Builder，避免 Vulkan context 依赖）
- `ComponentNode` 的 StateGroup 初始为空（stateCommands 留给 T13/T15 渲染层填充 Pipeline）
- `SceneManager` 维护 `unordered_map<string, ref_ptr<Node>>` 映射 + `root_->children` 同步，addNode 对重复 UUID 静默忽略
- LOD 使用 VSG 的 `minimumScreenHeightRatio` 机制（非距离阈值），默认高精度层 0.05、低精度层 0.0（始终可见）
- 所有节点均为 CPU 端数据结构，GPU 上传在 Viewer compile() 时发生

**已知限制**:
- StateGroup 无 Pipeline 绑定，在 T15 桥接前无法实际渲染
- PipePointNode 正方体无纹理/颜色绑定（颜色参数预留但未传入 StateGroup）
- SceneManager::removeNode 在 children 中的查找是 O(n)，大场景可优化为 index map

**后续任务注意**:
- T13 (相机控制) 可直接使用 `SceneManager::root()` 作为 Viewer 场景数据
- T14 (拾取) 可遍历 SceneManager 的 nodeMap_ 反查 UUID（或扩展 SceneManager 提供 UUID 查询接口）
- T15 (VSG-QML 桥接) 在 compile 前需为 ComponentNode 的 StateGroup 填充 GraphicsPipeline
- `visualization` 库现已包含所有 T12 产出，下游只需 `target_link_libraries(xxx visualization)` 即可

### T13 — 相机控制与场景基础设施 (2026-03-28)

**产出文件**:
- `src/visualization/CameraController.h`
- `src/visualization/CameraController.cpp`
- `src/visualization/SceneFurniture.h`
- `src/visualization/SceneFurniture.cpp`
- `tests/test_camera_furniture.cpp` — 27 个测试全部通过

**关键接口** (后续任务需要知道的):
```cpp
// visualization/CameraController.h
namespace visualization {
enum class ViewPreset { Front, Right, Top, Isometric };

class CameraController {
    CameraController(vsg::ref_ptr<vsg::Camera> camera,
                     vsg::ref_ptr<vsg::Trackball> trackball);
    vsg::ref_ptr<vsg::Trackball> trackball() const;
    vsg::ref_ptr<vsg::Camera>    camera()    const;
    void setViewPreset(ViewPreset preset, double duration = 0.2);
    void fitAll(vsg::ref_ptr<vsg::Node> sceneRoot);
    void fitAll(const vsg::dbox& bounds);
    vsg::ref_ptr<vsg::LookAt> lookAt() const;
    static vsg::ref_ptr<vsg::LookAt> computePresetLookAt(
        ViewPreset preset, const vsg::dvec3& center, double distance);
};
}

// visualization/SceneFurniture.h
namespace visualization {
class SceneFurniture {
    SceneFurniture(double axisLength=200.0, double gridSize=5000.0, uint32_t divisions=20);
    vsg::ref_ptr<vsg::Group>  axisNode()   const;  // X/Y/Z 三轴线段
    vsg::ref_ptr<vsg::Group>  gridNode()   const;  // 地面网格内容
    vsg::ref_ptr<vsg::Switch> gridSwitch() const;  // 可见性控制节点（添加到场景用这个）
    void setGridVisible(bool visible);
    bool isGridVisible() const;
    static constexpr float backgroundTopR/G/B(); // #E8E8E8
    static constexpr float backgroundBotR/G/B(); // #D0D0D0
};
}
```

**设计决策**:
- `CameraController` 是对 `vsg::Trackball` 的轻量封装，不派生 Visitor，而是持有 `ref_ptr<Trackball>` 委托事件处理
- `setViewPreset` 调用 `trackball->setViewpoint(lookAt, duration)` 实现平滑过渡（VSG 内置插值动画）
- `fitAll` 使用 `vsg::ComputeBounds` 遍历场景图，计算包围盒后设置新 LookAt
- `SceneFurniture` 轴线和网格均使用 `vsg::VertexDraw`（轻量级，仅存储顶点，不依赖 Vulkan context）
- 地面网格可见性通过 `vsg::Switch` 节点控制（`setAllChildren(bool)`）
- 背景颜色以 constexpr 常量提供，实际 Vulkan clearColor 设置留给 T15 桥接层

**已知限制**:
- `CameraController` 未实现自定义鼠标按键映射（使用 Trackball 默认映射），T15 桥接时可通过 QML 事件转发覆盖
- 坐标轴几何无颜色绑定（颜色 X=红/Y=绿/Z=蓝 由 Pipeline 的顶点着色器负责，T15 时填充）
- 渐变背景未实现为 VSG 节点（仅提供颜色常量供 T15 配置 Vulkan clearColor）

**后续任务注意**:
- T14 (拾取): `CameraController::camera()` 可用于构建 Intersector 的投影矩阵
- T15 (VSG-QML 桥接): 从 `ctrl.trackball()` 获取 Trackball，注册到 Viewer 事件处理链；
  `SceneFurniture::axisNode()` 和 `gridSwitch()` 加入场景根节点即可
- 典型场景组装:
  ```cpp
  SceneManager mgr;
  SceneFurniture furniture;
  mgr.root()->addChild(furniture.axisNode());
  mgr.root()->addChild(furniture.gridSwitch());
  auto ctrl = CameraController(camera, trackball);
  viewer->addEventHandler(ctrl.trackball());
  ```

### T15 — VSG-QML 桥接 (2026-03-28)

**产出文件**:
- `src/ui/VsgQuickItem.h`
- `src/ui/VsgQuickItem.cpp`
- `src/ui/CMakeLists.txt` (更新，添加 VsgQuickItem.cpp)
- `tests/test_vsg_qml_bridge.cpp` (28 个测试)
- `tests/CMakeLists.txt` (更新，添加 test_vsg_qml_bridge)
- `CMakeLists.txt` (更新，添加 Qt6::Test 组件)

**关键接口** (后续任务需要知道的):
```cpp
// ui/VsgQuickItem.h
namespace ui {
class VsgQuickItem : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(bool gridVisible READ isGridVisible WRITE setGridVisible NOTIFY gridVisibleChanged)
public:
    explicit VsgQuickItem(QQuickItem* parent = nullptr);

    // 场景管理（非拥有指针）
    void setSceneManager(visualization::SceneManager* mgr);
    void setCameraController(visualization::CameraController* ctrl);
    void setSceneFurniture(visualization::SceneFurniture* furniture);
    visualization::SceneManager* sceneManager() const;
    visualization::CameraController* cameraController() const;
    visualization::SceneFurniture* sceneFurniture() const;

    bool isGridVisible() const;
    void setGridVisible(bool visible);

    Q_INVOKABLE void fitAll();
    Q_INVOKABLE void setViewPreset(int preset); // 0=Front,1=Right,2=Top,3=Iso
    Q_INVOKABLE void toggleGrid();
    Q_INVOKABLE void requestRender();

    // 事件转换工具（public，供测试和外部使用）
    static vsg::ButtonMask qtButtonToVsgMask(Qt::MouseButton btn);
    static vsg::ButtonMask qtButtonsToVsgMask(Qt::MouseButtons btns);
    static vsg::KeyModifier qtModifiersToVsg(Qt::KeyboardModifiers mods);
    static vsg::KeySymbol qtKeyToVsg(int qtKey);

signals:
    void gridVisibleChanged();
    void deleteRequested();   // Delete 键
    void renderRequested();   // 场景需要重绘
};
} // namespace ui
```

**设计决策**:
- 使用 QQuickItem 而非 QQuickFramebufferObject 作为基类，因为后者仅支持 OpenGL 后端，而 VSG 使用 Vulkan
- Qt 鼠标事件转换为 VSG ButtonPressEvent/ButtonReleaseEvent/MoveEvent/ScrollWheelEvent，通过 `event->accept(*trackball)` 直接分派
- 键盘快捷键: F=FitAll, Delete=发信号, G=切换网格, Numpad 1/3/7/0=视图预设
- 未处理的键盘事件转发给 Trackball（支持 WASD 移动等 Trackball 内置功能）
- VsgQuickItem 持有非拥有指针（SceneManager*/CameraController*/SceneFurniture*），生命期由外部管理
- currentMask_ 跟踪当前按下的鼠标按钮组合，用于 VSG 事件的 mask 字段
- geometryChange() 自动更新 Camera ViewportState 以匹配 QML 项大小
- updatePaintNode() 目前返回背景色矩形占位节点，实际 VSG 离屏渲染→纹理在 T17 应用入口集成

**已知限制**:
- updatePaintNode() 当前为占位实现（纯色背景），实际 Vulkan 离屏渲染需在 T17 集成 Viewer 后完成
- 事件转发给 Trackball 时 Window 参数为 nullptr；Trackball 在 windowOffsets 为空时接受所有事件
- Qt numpad 数字键需通过 KeypadModifier 区分，非 numpad 数字键不触发视图预设

**后续任务注意**:
- T17 (工作台 + QML 桥接) 需要: 在 main.cpp 中 `qmlRegisterType<ui::VsgQuickItem>("PipeCAD", 1, 0, "VsgViewport")`
- T17 需要创建 SceneManager、CameraController、SceneFurniture 实例并通过 setter 注入 VsgQuickItem
- T17 需要完成 updatePaintNode() 的实际渲染：创建 VSG Viewer → 离屏渲染 → readback → QSGTexture
- T14 (拾取) 可扩展 VsgQuickItem 的鼠标事件处理，添加 PickHandler 调用
- CMakeLists.txt 已添加 Qt6::Test 组件供测试使用

### T16 — 应用层核心 (2026-03-28)

**产出文件**:
- `src/app/Document.h/cpp`
- `src/app/DependencyGraph.h/cpp`
- `src/app/TransactionManager.h/cpp`
- `src/engine/RecomputeEngine.h/cpp`
- `tests/test_app_core.cpp`

**关键接口** (后续任务需要知道的):
```cpp
// Document 负责管理所有的 DocumentObject
app::Document doc;
doc.addObject(std::make_shared<model::PipeSpec>("Spec1"));
auto* spec = doc.findObject(uuid);
auto specs = doc.findByType<model::PipeSpec>();

// DependencyGraph 维护对象间的依赖以供标脏传播和拓扑排序
app::DependencyGraph graph;
graph.addDependency(pointId, specId); // point 依赖 spec
graph.markDirty(specId);              // spec 变脏，级联标脏 point
auto dirtyIds = graph.collectDirty(); // 按拓扑排序返回脏对象列表

// TransactionManager 处理属性变更的事务，支持 undo/redo
app::TransactionManager tm(doc, graph);
tm.open("修改OD");
tm.recordChange(specId, "OD", 200.0, 219.1);
tm.commit(); // 触发 DependencyGraph 标脏，并调用 recompute 回调

// RecomputeEngine 进行脏对象的最终几何重算
engine::RecomputeEngine engine(doc, graph);
engine.setSceneUpdateCallback([&](const std::string& uuid, const TopoDS_Shape& shape){
    // sceneMgr.updateNode(uuid, node);
});
tm.setRecomputeCallback([&](const std::vector<foundation::UUID>& dirtyIds){
    engine.recompute(dirtyIds);
});
```

**设计决策**:
- **层级依赖调整**: 为避免 `app` 和 `visualization` 层的双向依赖问题，决定将 `RecomputeEngine.cpp` 从 `engine` 移除并纳入 `app` 层的库统一编译中进行调用。后由于设计清晰度，只使用回调 `SceneUpdateCallback` 解耦，重算后直接通过此回调传出生成的 `TopoDS_Shape` 通知外层(T17)去调用 `SceneManager` 或 `OcctToVsg`。
- **存储拓扑顺序**: `collectDirty()` 返回脏对象 UUID 时，直接利用字符串来重构存储（因为当前用字符串作 map 键）。

**已知限制**:
- `RecomputeEngine` 当前通过简单的线性查找计算所有管点的前后邻居位置；由于 `GeometryDeriver` 需要 `std::shared_ptr` 进行调用，`Document` 提供的是裸指针时需再倒查一次。目前实现虽然能跑过，但性能有优化的空间。若点位对象激增可加入缓存或直接传 `std::shared_ptr`。

**后续任务注意**:
- T17 可以全面介入并实现 `GeometryDeriver` 结合 `OcctToVsg` 和 `SceneManager` 的更新逻辑：在回调中直接转换形状至场景节点。

### T17 — 工作台系统 + C++→QML 桥接 (2026-03-28)

**产出文件**:
- `src/app/Workbench.h`
- `src/app/WorkbenchManager.h`
- `src/app/WorkbenchManager.cpp`
- `src/app/CadWorkbench.h`
- `src/app/CadWorkbench.cpp`
- `src/app/SelectionManager.h`
- `src/app/SelectionManager.cpp`
- `src/ui/AppController.h`
- `src/ui/AppController.cpp`
- `src/ui/WorkbenchController.h`
- `src/ui/WorkbenchController.cpp`
- `src/main.cpp`
- `ui/main.qml`
- `tests/test_workbench_bridge.cpp`
- `src/app/CMakeLists.txt` (更新)
- `src/ui/CMakeLists.txt` (更新)
- `src/CMakeLists.txt` (更新)
- `tests/CMakeLists.txt` (更新)

**关键接口** (后续任务需要知道的):
```cpp
// app/Workbench.h
enum class ViewportType { Vsg, Vtk };
struct ToolbarAction { std::string id; std::string label; };
class Workbench {
public:
    virtual std::string name() const = 0;
    virtual void activate(Document& document) = 0;
    virtual void deactivate(Document& document) = 0;
    virtual std::vector<ToolbarAction> toolbarActions() const = 0;
    virtual std::vector<std::string> panelIds() const = 0;
    virtual ViewportType viewportType() const = 0;
};

// app/WorkbenchManager.h
class WorkbenchManager {
public:
    using WorkbenchChangedCallback = std::function<void(const Workbench*)>;
    void registerWorkbench(std::unique_ptr<Workbench> workbench);
    bool switchWorkbench(const std::string& name);
    Workbench* activeWorkbench() const;
    std::vector<std::string> workbenchNames() const;
    void setWorkbenchChangedCallback(WorkbenchChangedCallback cb);
};

// app/SelectionManager.h
class SelectionManager {
public:
    using SelectionChangedCallback =
        std::function<void(const std::vector<foundation::UUID>&)>;
    bool select(const foundation::UUID& id);
    bool deselect(const foundation::UUID& id);
    void clear();
    bool isSelected(const foundation::UUID& id) const;
    const std::vector<foundation::UUID>& selected() const;
    void setSelectionChangedCallback(SelectionChangedCallback cb);
};

// ui/AppController.h
class AppController : public QObject {
    Q_PROPERTY(QString documentName READ documentName WRITE setDocumentName NOTIFY documentNameChanged)
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectionChanged)
    Q_PROPERTY(QStringList selectedUuids READ selectedUuids NOTIFY selectionChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY transactionStateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY transactionStateChanged)
};

// ui/WorkbenchController.h
class WorkbenchController : public QObject {
    Q_PROPERTY(QString activeWorkbench READ activeWorkbench NOTIFY activeWorkbenchChanged)
    Q_PROPERTY(QStringList activePanels READ activePanels NOTIFY activeWorkbenchChanged)
    Q_PROPERTY(QStringList toolbarActions READ toolbarActions NOTIFY activeWorkbenchChanged)
    Q_PROPERTY(QStringList workbenchNames READ workbenchNames CONSTANT)
    Q_INVOKABLE bool switchWorkbench(const QString& name);
};
```

**设计决策**:
- `WorkbenchManager` 使用 `Document&` 注入 + 回调通知方式，不把 Qt 依赖引入 `app` 层，保持层间解耦。
- `SelectionManager` 使用 `std::vector<UUID>` 保持稳定选择顺序，便于后续 T18/T19 做表格和树视图联动。
- `AppController`/`WorkbenchController` 负责把纯 C++ 管理器转换为 QML 可消费的 `Q_PROPERTY` / `Q_INVOKABLE` 接口。
- 应用入口 `main.cpp` 完成了 T15 留下的桥接要求：`qmlRegisterType<ui::VsgQuickItem>("PipeCAD", 1, 0, "VsgViewport")`，并注入 `SceneManager` / `CameraController` / `SceneFurniture`。
- `RecomputeEngine::SceneUpdateCallback` 在入口层连接 `OcctToVsg + createComponentNode + SceneManager`，实现重算到场景节点的闭环。

**已知限制**:
- `VsgQuickItem::updatePaintNode()` 仍为占位色块渲染，尚未实现 Vulkan 离屏渲染 readback → `QSGTexture` 的完整路径。
- 当前 `ui/main.qml` 为最小可启动骨架布局，详细业务面板将在 T18/T19 继续细化。

**后续任务注意**:
- T18 可直接消费 `AppController` 与 `SelectionManager` 的选择状态接口，实现表格/树与 3D 选择联动。
- T18/T19 在新增 Qt 源文件时，注意与 `foundation::Signal::emit` 的命名冲突：应优先包含 `app/...` 头，再包含 Qt 头，避免 Qt `emit` 宏污染。
- 应用可执行目标为 `pipecad_app`，QML 入口由 `PIPECAD_QML_MAIN` 编译宏指定为 `ui/main.qml`。

### T14 — 3D 拾取与高亮 (2026-03-28)

**产出文件**:
- `src/visualization/PickHandler.h`
- `src/visualization/PickHandler.cpp`
- `src/visualization/SelectionHighlight.h`
- `src/visualization/SelectionHighlight.cpp`
- `src/visualization/SceneManager.h` (更新，新增节点反查 UUID 接口)
- `src/visualization/SceneManager.cpp` (更新)
- `src/visualization/CMakeLists.txt` (更新，添加新源文件)
- `tests/test_pick_highlight.cpp`
- `tests/CMakeLists.txt` (更新，添加 test_pick_highlight)

**关键接口** (后续任务需要知道的):
```cpp
// visualization/SceneManager.h
class SceneManager {
public:
    // ...existing...
    std::string findUuidByNode(const vsg::Node* node) const;
};

// visualization/PickHandler.h
class PickHandler {
public:
    using SelectionCallback = std::function<void(const std::optional<std::string>&)>;
    using ContextMenuCallback =
        std::function<void(const vsg::dvec3&, const std::optional<std::string>&)>;

    explicit PickHandler(const SceneManager* sceneManager = nullptr);
    void setSceneManager(const SceneManager* sceneManager);
    void setSelectionCallback(SelectionCallback callback);
    void setContextMenuCallback(ContextMenuCallback callback);

    std::optional<std::string> pick(const vsg::Camera& camera,
                                    vsg::ref_ptr<vsg::Node> sceneRoot,
                                    int32_t x, int32_t y,
                                    vsg::dvec3* worldPosition = nullptr) const;

    void handleLeftClick(const vsg::Camera& camera,
                         vsg::ref_ptr<vsg::Node> sceneRoot,
                         int32_t x, int32_t y) const;
    void handleRightClick(const vsg::Camera& camera,
                          vsg::ref_ptr<vsg::Node> sceneRoot,
                          int32_t x, int32_t y) const;
};

// visualization/SelectionHighlight.h
class SelectionHighlight {
public:
    static constexpr const char* kHighlightColorKey = "pipecad.highlightColor";
    explicit SelectionHighlight(SceneManager* sceneManager = nullptr);

    bool setSelected(const std::string& uuid);
    bool clear();
    const std::string& selectedUuid() const;

    void setHighlightColor(const vsg::vec4& color);
    const vsg::vec4& highlightColor() const;
};
```

**设计决策**:
- 拾取实现采用 `vsg::LineSegmentIntersector(camera, x, y)`，并选择最近命中（最小 `ratio`）。
- 通过命中结果的 `nodePath` 逆序遍历 + `SceneManager::findUuidByNode()` 反查文档 UUID，保持可视化层内闭环，不引入 app 层依赖。
- 左键拾取通过 `SelectionCallback(optional<string>)` 通知选择结果；命中空白时回调 `nullopt` 用于清空选择。
- 右键拾取通过 `ContextMenuCallback(worldPos, optional<string>)` 回传拾取位置和可选 UUID，供上下文菜单定位。
- 高亮当前阶段使用节点元数据键 `pipecad.highlightColor` 存储高亮色（#0078D4），并在取消选中时恢复原值，避免提前耦合未落地的渲染管线细节。

**已知限制**:
- 当前高亮是材质颜色的“数据标记层”实现；真正 shader/material 替换需在后续渲染集成阶段读取该 key 并绑定到 pipeline。
- `SceneManager::findUuidByNode()` 是精确指针匹配（匹配注册根节点），依赖命中路径中包含被注册节点。

**后续任务注意**:
- T18/T19 做三方联动时，可把 `PickHandler::SelectionCallback` 直接桥接到 `SelectionManager::clear/select`。
- T19 的右键菜单可直接消费 `ContextMenuCallback` 返回的 `worldPos` 作为菜单锚点或对象属性面板跳转输入。
- 若后续实现真正渲染高亮，建议在渲染阶段读取 `SelectionHighlight::kHighlightColorKey` 并切换材质/描述符。

### T18 — QML 表格模型层 (2026-03-28)

**产出文件**:
- `src/ui/PipePointTableModel.h`
- `src/ui/PipePointTableModel.cpp`
- `src/ui/SegmentTreeModel.h`
- `src/ui/SegmentTreeModel.cpp`
- `src/ui/PropertyModel.h`
- `src/ui/PropertyModel.cpp`
- `src/ui/PipeSpecModel.h`
- `src/ui/PipeSpecModel.cpp`
- `src/ui/UuidUtil.h`
- `src/ui/AppController.h` (更新，暴露 4 个模型对象)
- `src/ui/AppController.cpp` (更新)
- `src/ui/CMakeLists.txt` (更新，添加模型源文件)
- `src/app/SelectionManager.h` (更新，支持多回调监听)
- `src/app/SelectionManager.cpp` (更新)
- `src/app/TransactionManager.cpp` (更新，支持坐标/类型/pipeSpecId 回放)
- `tests/test_qml_models.cpp`
- `tests/CMakeLists.txt` (更新，添加 `test_qml_models`)

**关键接口** (后续任务需要知道的):
```cpp
// ui/PipePointTableModel.h
class PipePointTableModel : public QAbstractTableModel {
    Q_PROPERTY(int selectedRow READ selectedRow NOTIFY selectedRowChanged)
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool selectRow(int row);
    // 列: Name | X | Y | Z | Type | PipeSpec | bendMultiplier
};

// ui/SegmentTreeModel.h
class SegmentTreeModel : public QAbstractItemModel {
    enum Roles { NameRole = Qt::UserRole + 1, KindRole, UuidRole, SelectedRole };
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool selectNodeByUuid(const QString& uuid);
};

// ui/PropertyModel.h
class PropertyModel : public QAbstractListModel {
    enum Roles { GroupRole = Qt::UserRole + 1, KeyRole, ValueRole, EditableRole };
    Q_INVOKABLE void refresh();
};

// ui/PipeSpecModel.h
class PipeSpecModel : public QAbstractTableModel {
    // 列: Name | OD | wallThickness | material
    Q_INVOKABLE void refresh();
};

// ui/AppController.h
Q_PROPERTY(QObject* pipePointTableModel READ pipePointTableModel CONSTANT)
Q_PROPERTY(QObject* segmentTreeModel READ segmentTreeModel CONSTANT)
Q_PROPERTY(QObject* propertyModel READ propertyModel CONSTANT)
Q_PROPERTY(QObject* pipeSpecModel READ pipeSpecModel CONSTANT)
```

**设计决策**:
- `PipePointTableModel` 以 Route/Segment 优先顺序构建管点行；未挂接到 Route 的对象回退到全局扫描并按名称排序，保证 UI 可见性与顺序稳定。
- Bend N/M/F 行识别规则为 `PipePointType::Bend` 且名称后缀为 `N/M/F`，这些行统一灰底且不可编辑。
- 表格与规格编辑统一走 `TransactionManager::open -> recordChange -> commit`，满足单步可撤销。
- `SelectionManager` 增加 `addSelectionChangedCallback`，支持 AppController 与多个模型并行订阅，避免回调互相覆盖。
- `SegmentTreeModel` 输出 `UuidRole` 与 `SelectedRole`，为 T19 树节点委托实现三方联动提供最小桥接接口。

**已知限制**:
- `Document` 当前仅暴露原始指针查询，`PipePoint::setPipeSpec(shared_ptr)` 无法在事务回放中完整恢复共享所有权；当前使用 `pipeSpecId` 字段记录引用信息，后续可在 Document 增补共享对象查询 API 后收敛。
- 当前模型层已提供 C++ 数据与选择联动接口，但详细 QML 面板布局与交互动画仍在 T19 完成。

**后续任务注意**:
- T19 可直接消费 `appController.pipePointTableModel`、`appController.segmentTreeModel`、`appController.propertyModel`、`appController.pipeSpecModel` 绑定控件。
- 树/表格点击建议统一调用 `selectNodeByUuid()` 或 `selectRow()`，3D 拾取侧继续通过 `SelectionManager` 同步即可形成三方联动。
- 若 T19 需要表格中 PipeSpec 下拉选择，建议在 Document 层补充“按 UUID 获取 shared_ptr<PipeSpec>”接口后再把 `pipeSpecId` 升级为真实对象引用。

### T19 — QML UI 面板 (2026-03-28)

**产出文件**:
- `ui/style/Theme.qml`
- `ui/components/CollapsiblePanel.qml`
- `ui/components/SplitView.qml`
- `ui/components/IconButton.qml`
- `ui/components/EditableCell.qml`
- `ui/components/ContextMenu.qml`
- `ui/panels/TopBar.qml`
- `ui/panels/StructureTree.qml`
- `ui/panels/Viewport3D.qml`
- `ui/panels/PipePointTable.qml`
- `ui/panels/PropertyPanel.qml`
- `ui/panels/StatusBar.qml`
- `ui/dialogs/NewProjectDialog.qml`
- `ui/dialogs/OpenProjectDialog.qml`
- `ui/main.qml`
- `tests/test_qml_ui_panels.cpp`
- `tests/CMakeLists.txt` (更新，添加 `test_qml_ui_panels`)

**关键接口** (后续任务需要知道的):
```qml
// ui/main.qml
ApplicationWindow {
    Shortcut { sequence: StandardKey.Undo; onActivated: appController.undo() }
    Shortcut { sequence: StandardKey.Redo; onActivated: appController.redo() }
    Shortcut { sequence: "Ctrl+S" }
    Shortcut { sequence: "Ctrl+N" }
    Shortcut { sequence: "Ctrl+O" }
    Shortcut { sequence: "Delete" }

    TopBar { workbenchController: workbenchController; appController: appController }

    SplitView { // 左: 结构树, 中: 3D 视口, 右: 表格 + 属性
        StructureTree { treeModel: appController.segmentTreeModel }
        Viewport3D { onInspectRequested: propertyPanel.ensureExpandedAndFlash() }
        SplitView {
            PipePointTable { tableModel: appController.pipePointTableModel }
            PropertyPanel { id: propertyPanel; propertyModel: appController.propertyModel }
        }
    }

    StatusBar { appController: appController }
}

// ui/panels/PropertyPanel.qml
function ensureExpandedAndFlash() {
    collapsed = false
    flashAnim.restart() // #0078D4, 300ms
}
```

**设计决策**:
- 采用组件化拆分：`main.qml` 只做布局与事件编排，面板/控件职责下沉到 `ui/panels` 与 `ui/components`。
- 使用自定义 `SplitView.qml`（基于 Qt Quick Controls SplitView）统一分割条样式，满足左右/上下可拖动分割。
- `Viewport3D` 右键菜单通过 `ContextMenu` 触发“查看属性”，调用 `PropertyPanel.ensureExpandedAndFlash()` 实现“已展开闪烁、已收缩自动展开”。
- 属性面板按 `PropertyModel` 的 `group` 分段渲染，天然支持不同管点类型动态字段差异。
- 新增 `tests/test_qml_ui_panels.cpp` 做 QML 加载烟测，验证核心面板对象存在，避免后续重构破坏主界面装配。

**已知限制**:
- 当前快捷键 `Ctrl+S/N/O/Delete` 为 UI 层触发与提示占位，实际保存/打开/删除业务逻辑待 T20/T21 或后续任务接入。
- `PipePointTable.qml` 当前通过模型通用 `setData` 入口提交编辑，单元格编辑器与字段类型校验仍有进一步细化空间。
- `PropertyPanel` 当前展示与引导完成，但字段写回仍由既有 C++ 模型层接口主导，尚未增加批量编辑表单行为。

**后续任务注意**:
- T20 (JSON 序列化) 可直接复用 `NewProjectDialog/OpenProjectDialog` 入口，把当前占位动作替换为真实 `save/load` 调用。
- 若后续接入 3D 拾取右键定位，可将 `Viewport3D.inspectRequested` 扩展为携带 UUID/世界坐标参数传递至属性面板。
- `test_qml_ui_panels` 使用 `qmlRegisterType<ui::VsgQuickItem>("PipeCAD", 1, 0, "VsgViewport")`，后续改动 VSG QML 注册名时需同步更新测试。

### T20 — JSON 序列化 (2026-03-28)

**产出文件**:
- `src/app/ProjectSerializer.h`
- `src/app/ProjectSerializer.cpp`
- `src/model/DocumentObject.h` (更新，增加反序列化 UUID 恢复接口)
- `src/app/CMakeLists.txt` (更新，编译 `ProjectSerializer.cpp`)
- `tests/test_project_serializer.cpp`
- `tests/CMakeLists.txt` (更新，添加 `test_project_serializer`)

**关键接口** (后续任务需要知道的):
```cpp
// app/ProjectSerializer.h
namespace app {
class ProjectSerializer {
public:
    static bool save(const Document& document, const std::string& filePath);
    static std::unique_ptr<Document> load(const std::string& filePath);
};
}

// model/DocumentObject.h
class DocumentObject {
public:
    const foundation::UUID& id() const;
    void setIdForDeserialization(const foundation::UUID& id);
};
```

**设计决策**:
- JSON 顶层结构实现为 `version + projectConfig + pipeSpecs + routes + accessories + beams`，满足 T20 任务定义。
- 为保证 `save -> load -> save` 稳定，序列化输出对 PipeSpec/Route/Accessory/Beam 采用 UUID 排序；Segment 与 PipePoint 保留原有拓扑顺序。
- `Variant` 字段采用显式类型编码：`{"type":"double|int|string","value":...}`，避免数字/字符串歧义。
- 反序列化时恢复跨对象引用：`PipePoint->PipeSpec`、`Accessory->PipePoint`（并回填到 `PipePoint::accessories`）、`Beam->start/end PipePoint`。
- `load()` 完成后创建临时 `DependencyGraph` 并调用 `RecomputeEngine::recomputeAll()`，满足“打开后全量重建”约定。
- 为维持历史工程 ID 稳定性，新增 `DocumentObject::setIdForDeserialization()` 仅用于加载阶段。

**已知限制**:
- 目前 JSON 中 `projectConfig` 仅按单对象处理（符合当前单文档模式）；若未来支持多配置模板需扩展为数组或命名集合。
- `ProjectSerializer::load()` 当前采用失败返回 `nullptr` 的容错策略，未区分细粒度错误码。

**后续任务注意**:
- T21 可直接复用 `ProjectSerializer` 的已加载对象关系，导出时无需再次做 PipeSpec/拓扑修复。
- 若 T21 需要把 STEP 与 JSON 双文件联动保存，建议沿用本任务字段命名，避免破坏已有 round-trip 测试基线。
- UI 层（T19）保存/打开占位动作可以直接接入 `ProjectSerializer::save/load`。

<!-- === COMPLETION LOG END === -->
