# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务（推荐）

| 属性 | 值 |
|------|---|
| **任务 ID** | T35 |
| **任务名** | SpecWorkbench 工作台 |
| **推荐模型** | Sonnet |
| **前置依赖** | T31 |
| **前置状态** | ✅ 所有依赖已满足 (T31 done 2026-03-29) |

## 项目进度

- 已完成: 27/38 个任务 (Phase 1: 22/22, Phase 2: 5/16)
- 当前阶段: Phase 2 — 二期开发

## 当前 ready 任务

| ID | 任务名 | 推荐模型 |
|----|--------|---------|
| T35 | SpecWorkbench 工作台 | Sonnet |
| T36 | DesignTree + ParameterPanel 重构 | **Codex** |
| T37 | OCCT→VTK 网格转换 | **Codex** |
| T43 | 序列化扩展 (Load/LoadCase) | **Codex** |

## 上一个完成的任务

T34 — DesignWorkbench 工作台 (2026-03-29)
- 产出: `src/app/DesignWorkbench.h`, `src/app/DesignWorkbench.cpp`, `tests/test_design_workbench.cpp`
- 关键接口: `DesignWorkbench::name()→"Design"`, `toolbarActions()`(4项), `panelIds()`(4项: DesignTree/Viewport3D/ComponentToolStrip/ParameterPanel), `viewportType()→Vsg`
- 注意事项: CadWorkbench 保留（旧测试依赖），main.cpp 注册两者并切换到 "Design"；T35 结构完全对称，仿照实现

## 给 AI 的指令（T35）

1. 读取 `docs/development-plan.md` 中 **T35** 章节
2. 读取 `docs/architecture.md` **§6.4** (SpecWorkbench)
3. 读取前置代码:
   - `src/app/Workbench.h` — 工作台抽象基类
   - `src/app/DesignWorkbench.h/.cpp` — 参考 T34 的对称实现
4. 完成后运行 `cd build/debug && ninja -j4 tests/test_spec_workbench && ./tests/test_spec_workbench`
5. 验证通过后更新 `docs/tasks/status.md` 和本文件
