# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务（推荐）

| 属性 | 值 |
|------|---|
| **任务 ID** | T12 |
| **任务名** | VSG 场景管理 |
| **推荐模型** | Sonnet |
| **前置依赖** | T11 ✅ |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 12/22 个任务（T01-T11）
- 当前阶段: Phase 4 — 可视化层

## 其他 ready 任务

| 任务 | 推荐模型 | 说明 |
|------|---------|------|
| T16 | **Opus** | 应用层核心 |
| T20 | Sonnet | JSON 序列化 |
| T21 | Sonnet | STEP 导出 |

## 上一个完成的任务

**T11 — OCCT→VSG 网格转换 (2026-03-28)**
- 产出:
  - `src/visualization/OcctToVsg.h/cpp` — Shape → VertexIndexDraw 转换
  - `tests/test_visualization.cpp` — 6 个测试全部通过
- 关键接口:
  - `visualization::toVsgGeometry(shape, deflection)` → `vsg::ref_ptr<vsg::VertexIndexDraw>`
  - binding 0 = 顶点 `vsg::vec3Array`, binding 1 = 法线 `vsg::vec3Array`
  - 索引类型 `vsg::uintArray`（uint32_t，支持大网格）
  - 空形体返回 nullptr
- 注意事项:
  - VertexIndexDraw 不含材质/着色器，T12 需在外层 StateGroup 提供 Pipeline
  - `visualization` 库已链接 `vsg::vsg` 和 `geometry`（传递依赖），T12 直接链接 visualization 即可

## 给 AI 的指令（T12）

1. 读取 `docs/development-plan.md` 中 **T12** 章节获取交付物和验收标准
2. 读取 `docs/architecture.md` 中场景管理相关章节
3. 读取 `lib/vsg/AGENTS.md` 了解 VSG API（特别是 Group、StateGroup、LOD、Builder）
4. 前置代码:
   - `src/visualization/OcctToVsg.h` (T11 产出)
   - `src/model/Route.h`, `src/model/Segment.h`, `src/model/PipePoint.h` (文档对象)
5. 完成后运行: `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
