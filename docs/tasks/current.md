# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T05 / T11 |
| **任务名** | 核心文档对象 / OCCT→VSG 网格转换 |
| **推荐模型** | **Opus** (T05) / Sonnet (T11) |
| **前置依赖** | T05←T02 ✅ / T11←T04 ✅ |
| **前置状态** | ✅ 两个任务均可立即开始（可并行） |

## 项目进度

- 已完成: 4/22 个任务
- 当前阶段: Phase 1 → Phase 2（基础设施完成）

## 上一个完成的任务

**T04 — OCCT 网格化 + STEP I/O (2026-03-28)**
- 产出: ShapeMesher / StepIO / ShapeProperties + test_mesh_step.cpp
- 验证: `pixi run test` ✅ 4/4 套件通过（14/14 T04 测试）
- 关键接口:
  - `geometry::ShapeMesher::mesh(shape, deflection)` → MeshData {vertices/normals/indices}
  - `geometry::StepIO::exportStep(shapes, path)` / `importStep(path)`
  - `geometry::ShapeProperties::volume(shape)` / `surfaceArea(shape)`

## 给 AI 的指令

**做 T05（核心文档对象）** — 推荐 **Opus**:

1. 读取 `docs/development-plan.md` 中 **T05** 章节（约 line 165-250）
2. 读取 `docs/architecture.md` §3 数据模型相关章节
3. 前置代码: `src/foundation/Types.h` / `foundation/Signal.h` / `foundation/Math.h`
4. 创建文件路径（均在 `src/model/` 下）:
   - `DocumentObject.h/cpp` — 基类: UUID + name + typed + 属性变更信号
   - `SpatialObject.h/cpp` — +position(gp_Pnt) 继承 DocumentObject
   - `PropertyObject.h/cpp` — 无坐标属性基类
   - `ContainerObject.h/cpp` — 子对象管理
   - `PipePoint.h/cpp` — type 枚举 + PipeSpec 引用 + 类型参数 map
   - `PipeSpec.h/cpp` — map<string,Variant> 可扩展字段
   - `ProjectConfig.h/cpp` — 工程名/作者/标准/单位制
   - `Segment.h/cpp` — 有序管点列表 + 段 ID
   - `Route.h/cpp` — 段的树状集合
5. 更新 `src/model/CMakeLists.txt` 添加所有 .cpp 源文件
6. 创建 `tests/test_model.cpp`，更新 `tests/CMakeLists.txt`
7. 完成后运行 `pixi run build-debug && pixi run test`

**做 T11（OCCT→VSG 网格转换）** — Sonnet:

1. 读取 `docs/development-plan.md` 中 **T11** 章节（约 line 360-400）
2. 读取 `lib/vsg/AGENTS.md` 获取 VSG API 使用指南
3. 前置代码: `src/geometry/ShapeMesher.h`（关键: MeshData 结构）
4. 主要文件路径:
   - `src/visualization/OcctToVsg.h/cpp` — `toVsgGeometry(shape, deflection)`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
8. 验证通过后更新 `docs/tasks/status.md` 和本文件
