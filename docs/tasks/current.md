# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T14 |
| **任务名** | 3D 拾取与高亮 |
| **推荐模型** | Sonnet |
| **前置依赖** | T12 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 17/22 个任务
- 当前阶段: Phase 5 — 应用层 / UI 桥接

## 上一个完成的任务

T17 — 工作台系统 + C++→QML 桥接 (2026-03-28)
- 产出: `Workbench` / `WorkbenchManager` / `CadWorkbench` / `SelectionManager`，`AppController` / `WorkbenchController`，`src/main.cpp`，`ui/main.qml`，`tests/test_workbench_bridge.cpp`
- 关键接口: `WorkbenchManager::switchWorkbench()`、`SelectionManager::setSelectionChangedCallback()`、`WorkbenchController::activePanels`、`AppController::selectedUuids`
- 注意事项: Qt `emit` 宏会污染 `foundation::Signal::emit`，头文件包含顺序需保持“先 app/model，后 Qt”

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T14** 章节
2. 读取 `docs/architecture.md` 中 3D 拾取与高亮相关章节
3. 读取前置代码: `src/ui/VsgQuickItem.h`、`src/ui/VsgQuickItem.cpp`、`src/visualization/SceneManager.h`、`src/visualization/CameraController.h`
4. 如需库指南: 读取 `lib/vsg/AGENTS.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
