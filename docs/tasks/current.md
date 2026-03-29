# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务（推荐）

| 属性 | 值 |
|------|---|
| **任务 ID** | T36 |
| **任务名** | DesignTree + ParameterPanel QML 重构 |
| **推荐模型** | **Codex** (GPT 5.2) |
| **前置依赖** | T34 |
| **前置状态** | ✅ 所有依赖已满足 (T34 done 2026-03-29) |

## 项目进度

- 已完成: 28/38 个任务 (Phase 1: 22/22, Phase 2: 6/16)
- 当前阶段: Phase 2 — 二期开发

## 当前 ready 任务

| ID | 任务名 | 推荐模型 |
|----|--------|---------|
| T36 | DesignTree + ParameterPanel 重构 | **Codex** |
| T37 | OCCT→VTK 网格转换 | **Codex** |
| T39 | 工作台切换 + QML 面板动态加载 | **Gemini** |
| T43 | 序列化扩展 (Load/LoadCase) | **Codex** |

## 上一个完成的任务

T35 — SpecWorkbench 工作台 (2026-03-29)
- 产出: `src/app/SpecWorkbench.h`, `src/app/SpecWorkbench.cpp`, `tests/test_spec_workbench.cpp`
- 关键接口: `SpecWorkbench::name()→"Specification"`, `toolbarActions()`(5项: new-spec/import-code/add-material/add-component/validate), `panelIds()`(4项: SpecTree/MaterialTable/ComponentTable/PropertyPanel), `viewportType()→Vsg`
- 注意事项: T39 两个依赖(T34+T35)均已满足，状态已改 ready；main.cpp 注册 SpecWorkbench 留给 T39

## 给 AI 的指令（T36）

1. 读取 `docs/development-plan.md` 中 **T36** 章节
2. 读取 `docs/architecture.md` **§9.2, §9.3** 相关章节
3. 读取前置代码:
   - `ui/panels/StructureTree.qml` — 现有结构树面板
   - `ui/panels/PropertyPanel.qml` — 现有属性面板
   - `ui/panels/PipePointTable.qml` — 现有管点表格
   - `ui/main.qml` — 主窗口布局
4. 完成后运行 `pixi run build-debug && pixi run test`
5. 验证通过后更新 `docs/tasks/status.md` 和本文件
