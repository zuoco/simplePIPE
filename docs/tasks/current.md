# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T19 |
| **任务名** | QML UI 面板 |
| **推荐模型** | Sonnet |
| **前置依赖** | T18 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 19/22 个任务
- 当前阶段: Phase 5 — 应用层 / UI 桥接

## 上一个完成的任务

T18 — QML 表格模型层 (2026-03-28)
- 产出: `PipePointTableModel`、`SegmentTreeModel`、`PropertyModel`、`PipeSpecModel`，`tests/test_qml_models.cpp`
- 关键接口: `AppController` 暴露 4 个模型属性；表格/树选择入口 `selectRow()` 与 `selectNodeByUuid()`
- 注意事项: `pipeSpecId` 目前以字段字符串记录引用，后续可在 Document 增补 shared_ptr 查询后升级为真实引用回放

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T19** 章节
2. 读取 `docs/architecture.md` 中 UI 设计 §9（布局、视觉规范、联动规则）
3. 读取前置代码: `src/ui/PipePointTableModel.h`、`src/ui/SegmentTreeModel.h`、`src/ui/PropertyModel.h`、`src/ui/PipeSpecModel.h`、`src/ui/AppController.h`
4. 如需库指南: 读取 `lib/vsg/AGENTS.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
