# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个 Ready 任务（2个可并行）

| 属性 | 值 |
|------|---|
| **任务 ID** | T42 |
| **任务名** | VTK-QML 桥接 |
| **推荐模型** | **Gemini** (3.1 Pro) |
| **前置依赖** | T38 |
| **前置状态** | ✅ 所有依赖已满足 |

### 其他 ready 任务

| 任务 ID | 任务名 | 推荐模型 | 前置依赖 |
|---------|--------|----------|----------|
| T43 | 序列化扩展 (Load/LoadCase) | **Codex** | T33 |

> T44 (AnalysisWorkbench) 依赖 T39✅ T42❌ T33✅，等待 T42 完成后解锁。

## 项目进度

- 已完成: 33/45 个任务
- 当前阶段: Phase 2 — 扩展功能开发

## 上一个完成的任务

T41 — ComponentToolStrip 元件插入 (2026-03-29)
- 产出: `ui/panels/ComponentToolStrip.qml`（双列图标条），`src/ui/AppController.h/cpp`（新增 insertComponent + 信号），`ui/main.qml`（绑定 + toast），`tests/test_qml_ui_panels.cpp`（2个新测试）
- 关键接口: `AppController::insertComponent(type)` / `insertComponentRequested(type)` 信号
- 注意事项: 工具条固定宽 68px，嵌入 SplitView 无法拖拽；插入的具体业务逻辑（创建管点）留给 T45

## 给 AI 的指令（T42）

1. 读取 `docs/development-plan.md` 中 **T42** 章节
2. 读取 `docs/architecture.md` **§8** 相关章节（VTK-QML 桥接）
3. 读取 `lib/vtk/AGENTS.md` 了解 VTK API
4. 读取前置代码: `src/vtk-visualization/VtkSceneManager.h`，`src/vtk-visualization/OcctToVtk.h`
5. 如需领域知识: 读取 `.github/skills/industrial-software-dev/SKILL.md`
6. 完成后运行 `cd build/debug && cmake --build . -j4 && QT_QPA_PLATFORM=offscreen tests/test_vtk_qml`（若有测试）
7. 验证通过后更新 `docs/tasks/status.md` 和本文件
