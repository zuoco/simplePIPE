# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务（4个 ready 可并行）

| 属性 | 值 |
|------|---|
| **任务 ID** | T41 |
| **任务名** | ComponentToolStrip 元件插入 |
| **推荐模型** | Sonnet |
| **前置依赖** | T31, T36 |
| **前置状态** | ✅ 所有依赖已满足 |

### 其他 ready 任务

| 任务 ID | 任务名 | 推荐模型 | 前置依赖 |
|---------|--------|----------|----------|
| T39 | 工作台切换 + QML 面板动态加载 | **Gemini** | T34, T35 |
| T42 | VTK-QML 桥接 | **Gemini** | T38 |
| T43 | 序列化扩展 (Load/LoadCase) | **Codex** | T33 |

## 项目进度

- 已完成: 32/45 个任务
- 当前阶段: Phase 2 — 扩展功能开发

## 上一个完成的任务

T40 — StatusBar + 右键菜单 + 框选 (2026-03-29)
- 产出: AppController.h/cpp, PickHandler.h/cpp, SceneManager.h/cpp, StatusBar.qml, ContextMenu.qml, Viewport3D.qml, main.qml, test_statusbar_contextmenu.cpp
- 关键接口: `AppController` 新增 selectionInfo/mouseCoord/zoomLevel/hasSelection 属性, deleteSelected()/multiSelect() 方法; `PickHandler::boxSelect()` 框选; `SceneManager::allUuids()` 枚举
- 注意事项: Qt Quick Controls MenuItem 不支持 `shortcut` 属性; 框选基于 AABB 投影可能不够精确; 删除操作只发信号需上层连接

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T41** 章节
2. 读取 `docs/architecture.md` 相关章节
3. 读取前置代码: `src/app/ComponentCatalog.h`, `src/ui/AppController.h`, `src/app/DesignWorkbench.h`
4. 如需领域知识或库指南: 读取 `.github/skills/industrial-software-dev/SKILL.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
