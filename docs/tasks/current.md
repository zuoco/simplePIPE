# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T06 / T07 / T09 / T10 / T11 |
| **任务名** | 附属对象与梁 / 弯头几何计算器 / 管件几何(Valve/Flex/Beam) / 拓扑管理与约束 / OCCT→VSG 网格转换 |
| **推荐模型** | Sonnet(T06) / **Opus**(T07) / Sonnet(T09) / Sonnet(T10) / Sonnet(T11) |
| **前置依赖** | T06←T05 ✅ / T07←T05,T02 ✅ / T09←T03,T05 ✅ / T10←T05 ✅ / T11←T04 ✅ |
| **前置状态** | ✅ 5 个任务均可立即开始 |

## 项目进度

- 已完成: 5/22 个任务
- 当前阶段: Phase 2 — 领域模型与几何算法

## 上一个完成的任务

**T05 — 核心文档对象 (2026-03-28)**
- 产出: 9 个头文件 (DocumentObject/SpatialObject/PropertyObject/ContainerObject/PipeSpec/PipePoint/ProjectConfig/Segment/Route) + test_model.cpp (30 tests)
- 验证: `pixi run test` ✅ 5/5 套件通过 (30/30 T05 测试)
- 关键接口:
  - `model::DocumentObject` — UUID id(), name(), ChangeSignal changed
  - `model::PipePoint` — PipePointType enum {Run,Bend,Reducer,Tee,Valve,FlexJoint}, pipeSpec(), typeParams()
  - `model::PipeSpec` — od(), wallThickness(), material(), 可扩展 field()/setField()
  - `model::Segment` — addPoint(), insertPoint(), removePoint(), pointAt(), pointCount()
  - `model::Route` — addSegment(), removeSegment(), segmentAt(), segmentCount()

## 给 AI 的指令

**优先推荐 T07（弯头几何计算器）** — 推荐 **Opus**（关键算法）:

1. 读取 `docs/development-plan.md` 中 **T07** 章节
2. 读取 `docs/architecture.md` §3 弯头几何相关章节
3. 前置代码: `src/model/PipePoint.h`, `src/foundation/Math.h` (Vec3, lineLineIntersect), `src/foundation/Types.h`
4. 如需 OCCT 库指南: `lib/occt/AGENTS.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件

**其他可选任务 (均 Sonnet)**:

- **T06 (附属对象与梁)**: 读取 development-plan.md T06 章节，前置: `src/model/*.h`
- **T09 (管件几何 Valve/Flex/Beam)**: 读取 T09 章节，前置: `src/geometry/ShapeBuilder.h`, `src/model/PipePoint.h`
- **T10 (拓扑管理与约束)**: 读取 T10 章节，前置: `src/model/Segment.h`, `src/model/Route.h`
- **T11 (OCCT→VSG 网格转换)**: 读取 T11 章节 + `lib/vsg/AGENTS.md`，前置: `src/geometry/ShapeMesher.h`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
8. 验证通过后更新 `docs/tasks/status.md` 和本文件
