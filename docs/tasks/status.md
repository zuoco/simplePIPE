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
| T02 | Foundation 层 | `ready` | T01 | Sonnet | |
| T03 | OCCT 几何封装 | `ready` | T01 | Sonnet | |
| T04 | OCCT 网格化 + STEP I/O | `ready` | T01 | Sonnet | |
| T05 | 核心文档对象 | `pending` | T02 | **Opus** | |
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
<!-- === COMPLETION LOG END === -->
