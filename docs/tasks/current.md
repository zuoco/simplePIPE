# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务（推荐）

| 属性 | 值 |
|------|---|
| **任务 ID** | T15 |
| **任务名** | VSG-QML 桥接 |
| **推荐模型** | **Opus** |
| **前置依赖** | T12 ✅, T13 ✅ |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 14/22 个任务（T01-T13）
- 当前阶段: Phase 4 — 可视化层

## 其他 ready 任务

| 任务 | 推荐模型 | 说明 |
|------|---------|------|
| T14 | Sonnet | 3D 拾取与高亮（依赖 T12 ✅） |
| T16 | **Opus** | 应用层核心（依赖 T05 ✅, T07 ✅） |
| T20 | Sonnet | JSON 序列化（依赖 T05 ✅, T06 ✅） |
| T21 | Sonnet | STEP 导出（依赖 T08 ✅, T09 ✅, T04 ✅） |

## 上一个完成的任务

**T13 — 相机控制与场景基础设施 (2026-03-28)**
- 产出:
  - `src/visualization/CameraController.h/cpp` — 封装 Trackball，提供视图预设和 FitAll
  - `src/visualization/SceneFurniture.h/cpp` — 坐标轴指示器+地面网格+背景颜色常量
  - `tests/test_camera_furniture.cpp` — 27 个测试全部通过
- 关键接口:
  - `CameraController(camera, trackball)` → `setViewPreset(preset, duration)` / `fitAll(scene)`
  - `ViewPreset::Front/Right/Top/Isometric`
  - `SceneFurniture::axisNode()` → Group；`gridSwitch()` → Switch（加入场景用这个）
  - `setGridVisible(bool)` / `isGridVisible()`
  - 背景色常量: `backgroundTopR/G/B()` = #E8E8E8, `backgroundBotR/G/B()` = #D0D0D0
- 注意事项:
  - `CameraController` 持有 Trackball 引用，T15 用 `ctrl.trackball()` 注册到 Viewer 事件链
  - 轴线/网格几何已存储顶点，颜色/Pipeline 留给 T15 填充
  - `gridSwitch()` 而非 `gridNode()` 才有可见性控制能力

## 给 AI 的指令（T15）

1. 读取 `docs/development-plan.md` 中 **T15** 章节获取交付物和验收标准
2. 读取 `docs/architecture.md` 中 QML/Vulkan 集成相关章节
3. 读取前置代码:
   - `src/visualization/SceneManager.h` (T12 产出)
   - `src/visualization/CameraController.h` (T13 产出)
   - `src/visualization/SceneFurniture.h` (T13 产出)
4. 读取 `lib/vsg/AGENTS.md` 了解 VSG Viewer、RenderGraph、CommandGraph API
5. 完成后运行: `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件

---

## 下一个任务（推荐）

| 属性 | 值 |
|------|---|
| **任务 ID** | T13 |
| **任务名** | 相机控制与场景基础设施 |
| **推荐模型** | Sonnet |
| **前置依赖** | T12 ✅ |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 13/22 个任务（T01-T12）
- 当前阶段: Phase 4 — 可视化层

## 其他 ready 任务

| 任务 | 推荐模型 | 说明 |
|------|---------|------|
| T14 | Sonnet | 3D 拾取与高亮（也依赖 T12 ✅） |
| T16 | **Opus** | 应用层核心 |
| T20 | Sonnet | JSON 序列化 |
| T21 | Sonnet | STEP 导出 |

## 上一个完成的任务

**T12 — VSG 场景管理 (2026-03-28)**
- 产出:
  - `src/visualization/PipePointNode.h/cpp` — 管点小正方体节点（手动构建 VertexIndexDraw）
  - `src/visualization/ComponentNode.h/cpp` — 管件节点（MatrixTransform → StateGroup → VertexIndexDraw）
  - `src/visualization/LodStrategy.h/cpp` — 两级 VSG LOD 节点工厂
  - `src/visualization/SceneManager.h/cpp` — UUID→VSG节点映射管理器
  - `tests/test_scene_manager.cpp` — 29 个测试全部通过
- 关键接口:
  - `SceneManager::root()` → `vsg::ref_ptr<vsg::Group>` 场景根节点
  - `SceneManager::addNode/removeNode/updateNode/batchUpdate` — 增量更新
  - `createPipePointNode(x, y, z, size)` → `MatrixTransform`
  - `createComponentNode(vid, matrix)` → `MatrixTransform → StateGroup → VertexIndexDraw`
  - `createLodNode(highDetail, lowDetail, center, radius)` → `vsg::LOD`
- 注意事项:
  - StateGroup 的 stateCommands 为空，T13/T15 需要在 compile 前填充 Pipeline
  - PipePointNode 正方体几何无纹理/颜色绑定（颜色参数预留）
  - LOD 使用 minimumScreenHeightRatio 机制，高精度 0.05 低精度 0.0

## 给 AI 的指令（T13）

1. 读取 `docs/development-plan.md` 中 **T13** 章节获取交付物和验收标准
2. 读取 `docs/architecture.md` 中场景基础设施相关章节
3. 读取前置代码:
   - `src/visualization/SceneManager.h` (T12 产出)
   - `src/visualization/LodStrategy.h` (T12 产出)
4. 读取 `lib/vsg/AGENTS.md` 了解 VSG Trackball / Camera / Viewer API
5. 完成后运行: `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
