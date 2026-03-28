# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T10 / T11 / T16 / T20 / T21 |
| **任务名** | 拓扑管理与约束 / OCCT→VSG 网格转换 / 应用层核心 / JSON 序列化 / STEP 导出 |
| **推荐模型** | Sonnet(T10,T11,T20,T21) / **Opus**(T16) |
| **前置依赖** | 全部满足 ✅ |
| **前置状态** | ✅ 5 个任务均可立即开始 |

## 项目进度

- 已完成: 10/22 个任务（T01-T09）
- 当前阶段: Phase 2 完成 — 进入 Phase 3/4/5/6

## 上一个完成的任务

**T06 — 附属对象与梁 (2026-03-28)**
- 产出: Accessory/FixedPoint/Support/Flange/Gasket/SealRing/Beam 7 个 model 类 + PipePoint accessory 管理 + test_accessory_beam.cpp (36 tests)
- 关键接口:
  - `model::Accessory` — 附属构件基类，weak_ptr 引用管点 + offset
  - `model::FixedPoint/Support/Flange/Gasket/SealRing` — 5 种附属构件子类
  - `model::Beam` — 梁，双端 weak_ptr 引用管点，length() 实时计算
  - `model::PipePoint::addAccessory/removeAccessory/accessories()` — 管点的附件列表管理
- 注意事项: accessory 列表存储为 shared_ptr<DocumentObject>，需 dynamic_cast 获取具体类型

## 给 AI 的指令

**推荐 T10（拓扑管理与约束）** — Sonnet:

1. 读取 `docs/development-plan.md` 中 **T10** 章节
2. 读取 `docs/architecture.md` 相关章节
3. 前置代码: `src/model/PipePoint.h`, `src/model/Segment.h`, `src/model/Route.h`
4. 完成后运行 `pixi run build-debug && pixi run test`
5. 验证通过后更新 `docs/tasks/status.md` 和本文件

**其他可选任务**:

- **T11 (OCCT→VSG 网格转换)** — Sonnet: 读取 T11 + `lib/vsg/AGENTS.md`，前置: `src/geometry/ShapeMesher.h`
- **T16 (应用层核心)** — **Opus**: 读取 T16，前置: `src/model/*.h`, `src/engine/GeometryDeriver.h`
- **T20 (JSON 序列化)** — Sonnet: 读取 T20，前置: `src/model/*.h`（含 T06 新增的 Accessory/Beam 类）
- **T21 (STEP 导出)** — Sonnet: 读取 T21，前置: `src/engine/*Builder.h`, `src/geometry/StepIO.h`
- **T21 (STEP 导出)** — Sonnet: 读取 T21，前置: `src/geometry/StepIO.h`, `src/engine/GeometryDeriver.h`

