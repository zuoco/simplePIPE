# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T40 |
| **任务名** | 场景树 (SceneGraph) QML 面板实现 |
| **推荐模型** | Opus / Gemini / Sonnet |
| **前置依赖** | T30, T38 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 19/22 个任务
- 当前阶段: Phase 6 — C++ 业务逻辑与 UI 桥接

## 上一个完成的任务

T39 — 工作台切换 + QML 面板动态加载 (2024-05-31)
- 产出: [ui/main.qml, ui/panels/TopBar.qml, src/ui/WorkbenchController.cpp, src/main.cpp]
- 关键接口: `switchWorkbench(QString)`, `viewportLoaded(QObject*)` 提供动态 QML 重载接口
- 注意事项: 新的面板由 `Repeater` 和 `Loader` 提供按需加载。后续扩展只需添加 QML 并修改对应的 `panelIds`。

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T40** 章节
2. 读取 `docs/architecture.md` **§X** 相关章节
3. 读取前置代码: `src/ui/WorkbenchController.h` 和 `src/app/DesignWorkbench.h`
4. 如需领域知识或库指南: 读取 `.github/skills/industrial-software-dev/SKILL.md` 和 `lib/vsg/AGENTS.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
