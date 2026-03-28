# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务（推荐）

| 属性 | 值 |
|------|---|
| **任务 ID** | T16 |
| **任务名** | 应用层核心 (Document + Transaction + DependencyGraph) |
| **推荐模型** | **Opus** |
| **前置依赖** | T05 ✅, T07 ✅ |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 15/22 个任务（T01-T13, T15）
- 当前阶段: Phase 4 — 可视化层 + Phase 5 — 应用层

## 其他 ready 任务

| 任务 | 推荐模型 | 说明 |
|------|---------|------|
| T14 | Sonnet | 3D 拾取与高亮（依赖 T12 ✅） |
| T20 | Sonnet | JSON 序列化（依赖 T05 ✅, T06 ✅） |
| T21 | Sonnet | STEP 导出（依赖 T08 ✅, T09 ✅, T04 ✅） |

## 上一个完成的任务

**T15 — VSG-QML 桥接 (2026-03-28)**
- 产出:
  - `src/ui/VsgQuickItem.h/cpp` — QQuickItem 子类，桥接 VSG 与 QML
  - `tests/test_vsg_qml_bridge.cpp` — 28 个测试全部通过
- 关键接口:
  - `VsgQuickItem` = QQuickItem + SceneManager/CameraController/SceneFurniture 管理
  - `setGridVisible(bool)` / `fitAll()` / `setViewPreset(int)` / `toggleGrid()`
  - 鼠标事件 → VSG ButtonPress/Release/Move/ScrollWheel 转发给 Trackball
  - 键盘: F=FitAll, Delete=信号, G=网格, Numpad=视图预设
  - 信号: `gridVisibleChanged()`, `deleteRequested()`, `renderRequested()`
- 注意事项:
  - updatePaintNode() 为占位实现，实际渲染需 T17 集成 VSG Viewer
  - VsgQuickItem 持有非拥有指针，T17 需创建并注入依赖对象
  - CMakeLists.txt 已添加 Qt6::Test 组件

## 给 AI 的指令（T16）

1. 读取 `docs/development-plan.md` 中 **T16** 章节获取交付物和验收标准
2. 读取 `docs/architecture.md` 中 §7 事务管理机制 相关章节
3. 读取前置代码:
   - `src/model/DocumentObject.h` (T05 产出)
   - `src/model/PipePoint.h` (T05 产出)
   - `src/model/PipeSpec.h` (T05 产出)
   - `src/model/Segment.h` / `Route.h` (T05 产出)
   - `src/engine/GeometryDeriver.h` (T08 产出)
   - `src/visualization/SceneManager.h` (T12 产出)
4. 完成后运行: `pixi run build-debug && pixi run test`
5. 验证通过后更新 `docs/tasks/status.md` 和本文件
