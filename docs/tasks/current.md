# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T06 / T10 / T11 / T16 / T21 |
| **任务名** | 附属对象与梁 / 拓扑管理与约束 / OCCT→VSG 网格转换 / 应用层核心 / STEP 导出 |
| **推荐模型** | Sonnet(T06,T10,T11,T21) / **Opus**(T16) |
| **前置依赖** | 全部满足 ✅ |
| **前置状态** | ✅ 5 个任务均可立即开始 |

## 项目进度

- 已完成: 9/22 个任务（T01-T05, T07, T08, T09）
- 当前阶段: Phase 2 — 领域模型与几何算法 + 引擎层

## 上一个完成的任务

**T09 — 管件几何 Valve/FlexJoint/Beam/Accessory (2026-06-03)**
- 产出: ValveBuilder / FlexJointBuilder / BeamBuilder / AccessoryBuilder + test_valve_flex_beam.cpp (19 tests)
- 验证: 19/19 测试通过（5 Valve + 4 FlexJoint + 6 Beam + 4 Accessory）
- 关键接口:
  - `engine::ValveBuilder::build(start, end, OD, WT, valveType)` → 阀门几何（膨大阀体+管段）
  - `engine::FlexJointBuilder::build(start, end, OD, WT, segmentCount)` → 波纹管（交替锥体）
  - `engine::BeamBuilder::build(start, end, sectionType, width, height)` → 型钢梁（BRepPrimAPI_MakePrism）
  - `engine::AccessoryBuilder::buildFlange(center, normal, pipeDiameter, thickness)` → 法兰圆盘
  - `engine::AccessoryBuilder::buildBracket(base, top, width)` → 支架方柱
  - `engine::GeometryDeriver` 已完整覆盖所有 PipePointType（Run/Bend/Reducer/Tee/Valve/FlexJoint）

## 给 AI 的指令

**推荐 T10（拓扑管理与约束）** — Sonnet:

1. 读取 `docs/development-plan.md` 中 **T10** 章节
2. 读取 `docs/architecture.md` 相关章节
3. 前置代码: `src/model/PipePoint.h`, `src/model/Segment.h`, `src/model/Route.h`
4. 完成后运行 `pixi run build-debug && pixi run test`
5. 验证通过后更新 `docs/tasks/status.md` 和本文件

**其他可选任务**:

- **T06 (附属对象与梁)** — Sonnet: 读取 T06，前置: `src/model/*.h`
- **T11 (OCCT→VSG 网格转换)** — Sonnet: 读取 T11 + `lib/vsg/AGENTS.md`，前置: `src/geometry/ShapeMesher.h`
- **T16 (应用层核心)** — **Opus**: 读取 T16，前置: `src/model/*.h`, `src/engine/GeometryDeriver.h`
- **T21 (STEP 导出)** — Sonnet: 读取 T21，前置: `src/geometry/StepIO.h`, `src/engine/GeometryDeriver.h`

