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
| T04 | OCCT 网格化 + STEP I/O | `ready` | T01 | Sonnet | |
| T05 | 核心文档对象 | `ready` | T02 | **Opus** | |
| T06 | 附属对象与梁 | `pending` | T05 | Sonnet | |
| T07 | 弯头几何计算器 | `pending` | T05, T02 | **Opus** | |
| T08 | 管件几何 (Run/Reducer/Tee) | `pending` | T07, T03 | Sonnet | |
| T09 | 管件几何 (Valve/Flex/Beam) | `pending` | T03, T05 | Sonnet | |
| T10 | 拓扑管理与约束 | `pending` | T05 | Sonnet | |
| T11 | OCCT→VSG 网格转换 | `pending` | T04 | Sonnet | |
| T12 | VSG 场景管理 | `pending` | T11 | Sonnet | |
| T13 | 相机控制与场景基础设施 | `pending` | T12 | Sonnet | |
| T14 | 3D 拾取与高亮 | `pending` | T12 | Sonnet | |
| T15 | VSG-QML 桥接 | `pending` | T12, T13 | **Opus** | |
| T16 | 应用层核心 | `pending` | T05, T07 | **Opus** | |
| T17 | 工作台 + QML 桥接 | `pending` | T16 | Sonnet | |
| T18 | QML 表格模型层 | `pending` | T17 | Sonnet | |
| T19 | QML UI 面板 | `pending` | T18 | Sonnet | |
| T20 | JSON 序列化 | `pending` | T05, T06 | Sonnet | |
| T21 | STEP 导出 | `pending` | T08, T09, T04 | Sonnet | |
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
<!-- === COMPLETION LOG END === -->
