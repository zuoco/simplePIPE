# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T43 |
| **任务名** | 序列化扩展 (Load/LoadCase) |
| **推荐模型** | Codex (GPT 5.2) / Opus |
| **前置依赖** | T33, T20 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 42/45 个任务
- 当前阶段: Phase 14 — 集成 & 序列化

## 上一个完成的任务

T42 — VTK-QML 桥接 (2026-03-29)
- 产出: `VtkViewport.h/cpp`, `VtkViewport.qml`, `test_vtk_qml.cpp`
- 关键接口: `VtkViewport` 继承 `QQuickFramebufferObject` 显示 `vtkGenericOpenGLRenderWindow` 的渲染。
- 注意事项: `AnalysisWorkbench` 会加载 `VtkViewport`；注意测试在无界面的环境下可能的 OpenGL 验证影响。

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T43** 章节
2. 读取 `docs/architecture.md` **§6.9** 相关章节
3. 读取前置代码: `src/app/ProjectSerializer.h`
4. 完成后运行 `pixi run build-debug && pixi run test`
5. 验证通过后更新 `docs/tasks/status.md` 和本文件
