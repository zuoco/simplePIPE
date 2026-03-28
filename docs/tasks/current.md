# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T32 |
| **任务名** | Load 载荷数据模型 |
| **推荐模型** | Sonnet |
| **前置依赖** | — (无前置) |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 24/38 个任务 (Phase 1: 22/22, Phase 2: 2/16)
- 当前阶段: Phase 2 — 二期开发

## 上一个完成的任务

T31 — ComponentCatalog 参数化构件模板库 (2026-03-29)
- 产出: `ComponentTemplate.h`, `ComponentCatalog.h/cpp`, 8种模板 (`templates/*.h`), `test_component_catalog.cpp`
- 关键接口: `ComponentCatalog::instance().getTemplate("GateValve")->buildShape(params)` 统一查表生成几何
- 注意事项: 仅 GateValve 在 GeometryDeriver 中改为 Catalog 查表，其他类型保持原 Builder

## 当前 ready 任务

| ID | 任务名 | 推荐模型 |
|----|--------|---------|
| T32 | Load 载荷数据模型 | Sonnet |
| T34 | DesignWorkbench 工作台 | Sonnet |
| T35 | SpecWorkbench 工作台 | Sonnet |

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T32** 章节
2. 读取 `docs/architecture.md` **§6.9.1, §6.9.2** (载荷数据模型)
3. 读取前置代码: `src/model/DocumentObject.h` (Load 继承自 DocumentObject)
4. 如需领域知识或库指南: 读取 `.github/skills/industrial-software-dev/SKILL.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
