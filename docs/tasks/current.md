# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务（推荐）

| 属性 | 值 |
|------|---|
| **任务 ID** | T17 |
| **任务名** | 工作台 + QML 桥接 |
| **推荐模型** | Sonnet |
| **前置依赖** | T16 ✅ |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 16/22 个任务（T01-T13, T15, T16）
- 当前阶段: Phase 5 — 应用层

## 其他 ready 任务

| 任务 | 推荐模型 | 说明 |
|------|---------|------|
| T14 | Sonnet | 3D 拾取与高亮（依赖 T12 ✅） |
| T20 | Sonnet | JSON 序列化（依赖 T05 ✅, T06 ✅） |
| T21 | Sonnet | STEP 导出（依赖 T08 ✅, T09 ✅, T04 ✅） |

## 上一个完成的任务

**T16 — 应用层核心 (2026-03-28)**
- 产出:
  - `src/app/Document.h/cpp` — 文档容器模型
  - `src/app/DependencyGraph.h/cpp` — 图拓扑顺序依赖管理与标脏
  - `src/app/TransactionManager.h/cpp` — 集中事务处理及 Undo/Redo
  - `src/engine/RecomputeEngine.h/cpp` — 聚合对象依赖图进行统一几何推算
  - `tests/test_app_core.cpp` — 单元测试
- 关键接口:
  - `Document` 全量数据操作：`addObject` / `allSegments` / `findByType<T>`
  - `DependencyGraph` 进行脏对象重排及遍历标脏图 `markDirty(uuid)` → 传递
  - `TransactionManager` 处理字段变化： `open()` → `recordChange()` → `commit()` 支持撤销与重做。
- 注意事项:
  - 设计了解耦方式：`RecomputeEngine` 提供 `setSceneUpdateCallback( (uuid, TopoDS_Shape)->... )`
  - T17 需实现该回调内联，并结合 `OcctToVsg::convert(shape)` 用其产生的 `Node` 同步给 `SceneManager` 更新模型视角。

## 给 AI 的指令（T17）

1. 读取 `docs/development-plan.md` 中 **T17** 章节获取交付物和验收标准
2. 读取前置代码:
   - `src/app/Document.h`
   - `src/app/Workbench.h` (本次将新增/实现)
   - `src/ui/VsgQuickItem.h`
   - `src/visualization/SceneManager.h`
   - `src/engine/RecomputeEngine.h`
3. 完成后运行: `pixi run build-debug && pixi run test`
4. 验证通过后更新 `docs/tasks/status.md` 和本文件
