# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务（推荐）

| 属性 | 值 |
|------|---|
| **任务 ID** | T34 |
| **任务名** | DesignWorkbench 工作台 |
| **推荐模型** | Sonnet |
| **前置依赖** | T31 |
| **前置状态** | ✅ 所有依赖已满足 (T31 done 2026-03-29) |

## 项目进度

- 已完成: 26/38 个任务 (Phase 1: 22/22, Phase 2: 4/16)
- 当前阶段: Phase 2 — 二期开发

## 当前 ready 任务

| ID | 任务名 | 推荐模型 |
|----|--------|---------|
| T34 | DesignWorkbench 工作台 | Sonnet |
| T35 | SpecWorkbench 工作台 | Sonnet |
| T37 | OCCT→VTK 网格转换 | **Codex** |
| T43 | 序列化扩展 (Load/LoadCase) | **Codex** |

## 上一个完成的任务

T33 — LoadCase 与 LoadCombination (2026-03-29)
- 产出: `src/model/LoadCase.h`, `src/model/LoadCombination.h`, `tests/test_loadcase.cpp`
- 关键接口: `LoadCase::addEntry()`, `LoadCombination::addCaseEntry()`, `CombineMethod`, `StressCategory`
- DAG: LoadCombination → LoadCase → Load，全通过 UUID 引用
- 测试: 24 个测试，全部通过

## 给 AI 的指令（T34）

1. 读取 `docs/development-plan.md` 中 **T34** 章节
2. 读取 `docs/architecture.md` **§6.5** (DesignWorkbench)
3. 读取前置代码:
   - `src/app/Workbench.h` — 工作台抽象基类
   - `src/app/WorkbenchManager.h` — 注册/切换工作台
   - `src/app/CadWorkbench.h` — 参考现有工作台实现
4. 完成后运行 `cd build/debug && ninja -j4 tests/test_design_workbench && ./tests/test_design_workbench`
5. 验证通过后更新 `docs/tasks/status.md` 和本文件
