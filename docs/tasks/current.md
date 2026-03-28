# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T20 |
| **任务名** | JSON 序列化 |
| **推荐模型** | Sonnet |
| **前置依赖** | T05, T06 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 20/22 个任务
- 当前阶段: Phase 6 — 数据交换

## 上一个完成的任务

T19 — QML UI 面板 (2026-03-28)
- 产出: `ui/main.qml` + `ui/style` + `ui/components` + `ui/panels` + `ui/dialogs`，`tests/test_qml_ui_panels.cpp`
- 关键接口: `PropertyPanel.ensureExpandedAndFlash()`；`Viewport3D.onInspectRequested` 引导属性面板；全局快捷键 Ctrl+S/Z/Y/N/O, Delete
- 注意事项: 当前保存/打开/删除仍是 UI 占位动作，T20 可直接接入真实序列化逻辑

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T20** 章节
2. 读取 `docs/architecture.md` 中数据交换与应用约定相关章节（JSON 结构、单文档约定）
3. 读取前置代码: `src/app/Document.h`、`src/model/PipePoint.h`、`src/model/PipeSpec.h`、`src/model/Route.h`、`src/model/Segment.h`、`src/model/Accessory.h`、`src/model/Beam.h`
4. 如需库指南: 无（本任务以 `nlohmann/json` 为主）
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
