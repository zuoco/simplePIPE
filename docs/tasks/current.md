# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T06 / T08 / T09 / T10 / T11 / T16 |
| **任务名** | 附属对象与梁 / 管件几何(Run/Reducer/Tee) / 管件几何(Valve/Flex/Beam) / 拓扑管理与约束 / OCCT→VSG 网格转换 / 应用层核心 |
| **推荐模型** | Sonnet(T06,T08,T09,T10,T11) / **Opus**(T16) |
| **前置依赖** | T06←T05 ✅ / T08←T07,T03 ✅ / T09←T03,T05 ✅ / T10←T05 ✅ / T11←T04 ✅ / T16←T05,T07 ✅ |
| **前置状态** | ✅ 6 个任务均可立即开始 |

## 项目进度

- 已完成: 6/22 个任务
- 当前阶段: Phase 2 — 领域模型与几何算法 + 引擎层

## 上一个完成的任务

**T07 — 弯头几何计算器 (2026-03-28)**
- 产出: BendCalculator.h/.cpp + test_engine.cpp (12 tests)
- 验证: `pixi run test` ✅ 6/6 套件通过 (12/12 T07 测试)
- 关键接口:
  - `engine::BendCalculator::calculateBend(prevPt, intersectPt, nextPt, OD, multiplier)` → `optional<BendResult>`
  - `BendResult` = {nearPoint, midPoint, farPoint, arcCenter, bendAngle, bendRadius}
- 注意: 返回 nullopt 表示退化(直线/U-turn/重合点)

## 给 AI 的指令

**推荐优先 T08（管件几何 Run/Reducer/Tee）** — Sonnet:

1. 读取 `docs/development-plan.md` 中 **T08** 章节
2. 读取 `docs/architecture.md` 相关章节
3. 前置代码: `src/engine/BendCalculator.h`, `src/geometry/ShapeBuilder.h`, `src/geometry/BooleanOps.h`
4. 如需 OCCT 库指南: `lib/occt/AGENTS.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件

**其他可选任务**:

- **T06 (附属对象与梁)** — Sonnet: 读取 T06 章节，前置: `src/model/*.h`
- **T09 (管件几何 Valve/Flex/Beam)** — Sonnet: 读取 T09 章节，前置: `src/geometry/ShapeBuilder.h`, `src/model/PipePoint.h`
- **T10 (拓扑管理与约束)** — Sonnet: 读取 T10 章节，前置: `src/model/Segment.h`, `src/model/Route.h`
- **T11 (OCCT→VSG 网格转换)** — Sonnet: 读取 T11 章节 + `lib/vsg/AGENTS.md`，前置: `src/geometry/ShapeMesher.h`
- **T16 (应用层核心)** — **Opus**: 读取 T16 章节，前置: `src/model/*.h`, `src/engine/BendCalculator.h`
