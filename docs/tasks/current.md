# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T18 |
| **任务名** | QML 表格模型层 |
| **推荐模型** | Sonnet |
| **前置依赖** | T17 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 18/22 个任务
- 当前阶段: Phase 5 — 应用层 / UI 桥接

## 上一个完成的任务

T14 — 3D 拾取与高亮 (2026-03-28)
- 产出: `PickHandler` / `SelectionHighlight`，`SceneManager::findUuidByNode()`，`tests/test_pick_highlight.cpp`
- 关键接口: `PickHandler::pick()/handleLeftClick()/handleRightClick()`、`SelectionHighlight::setSelected()/clear()`
- 注意事项: 当前高亮通过节点元数据键 `pipecad.highlightColor` 标记，后续渲染层需读取该键完成真正材质替换

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T18** 章节
2. 读取 `docs/architecture.md` 中 UI 表格模型与三方联动相关章节
3. 读取前置代码: `src/ui/AppController.h`、`src/ui/WorkbenchController.h`、`src/app/SelectionManager.h`、`src/app/TransactionManager.h`
4. 如需库指南: 读取 `lib/vsg/AGENTS.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
