# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T39 |
| **任务名** | 工作台切换 + QML 面板动态加载 |
| **推荐模型** | **Gemini** |
| **前置依赖** | T34, T35 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 31/38 个任务
- 当前阶段: Phase 2 — 二期开发

## 当前 ready 任务

| ID | 任务名 | 推荐模型 |
|----|--------|---------|
| T39 | 工作台切换 + QML 面板动态加载 | **Gemini** |
| T40 | StatusBar + 右键菜单 + 框选 | Sonnet |
| T41 | ComponentToolStrip 元件插入 | Sonnet |
| T42 | VTK-QML 桥接 | **Gemini** |
| T43 | 序列化扩展 (Load/LoadCase) | **Codex** |

## 上一个完成的任务

T38 — VTK 场景管理 (2026-03-29)
- 产出: `src/vtk-visualization/VtkSceneManager.h/.cpp`, `tests/test_vtk_scene.cpp`, `src/vtk-visualization/CMakeLists.txt`, `tests/CMakeLists.txt`
- 关键接口: `vtk_vis::VtkSceneManager::addActor/removeActor/updateActor/setRenderMode/renderer`
- 注意事项: T42 可直接复用 `VtkSceneManager::renderer()` 做 QML 桥接，实体/梁模式切换可直接调用 `setRenderMode`

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T39** 章节
2. 读取 `docs/architecture.md` 中 **§6.7** 与工作台动态面板加载相关章节
3. 读取前置代码: `ui/main.qml`, `ui/panels/TopBar.qml`, `src/ui/WorkbenchController.h`, `src/ui/WorkbenchController.cpp`
4. 如需工作台上下文，读取 `src/app/WorkbenchManager.h` 与 `src/app/Workbench.h`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
