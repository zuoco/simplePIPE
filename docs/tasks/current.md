# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T33 |
| **任务名** | LoadCase 与 LoadCombination |
| **推荐模型** | Sonnet |
| **前置依赖** | T32 |
| **前置状态** | ✅ 所有依赖已满足 (T32 done 2026-03-29) |

## 项目进度

- 已完成: 25/38 个任务 (Phase 1: 22/22, Phase 2: 3/16)
- 当前阶段: Phase 2 — 二期开发

## 上一个完成的任务

T32 — Load 载荷数据模型 (2026-03-29)
- 产出: `Load.h`, `DeadWeightLoad.h`, `ThermalLoad.h`, `PressureLoad.h`, `WindLoad.h`, `SeismicLoad.h`, `DisplacementLoad.h`, `UserDefinedLoad.h`, `test_load_model.cpp`
- 关键接口: `Load::loadType()`, `affectedObjects()`, `addAffectedObject()`, `removeAffectedObject()`
- 注意事项: Vec3 类型使用 `foundation::math::Vec3`，不是 `foundation::Vec3`

## 当前 ready 任务

| ID | 任务名 | 推荐模型 |
|----|--------|---------|
| T33 | LoadCase 与 LoadCombination | Sonnet |
| T34 | DesignWorkbench 工作台 | Sonnet |
| T35 | SpecWorkbench 工作台 | Sonnet |
| T37 | OCCT→VTK 网格转换 | **Codex** |

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T33** 章节
2. 读取 `docs/architecture.md` **§6.9.3, §6.9.4, §6.9.5** (LoadCase/LoadCombination)
3. 读取前置代码:
   - `src/model/Load.h` — LoadCase 引用 Load 的 UUID
   - `src/model/DocumentObject.h` — LoadCase/LoadCombination 继承基类
4. 如需领域知识: 读取 `.github/skills/industrial-software-dev/SKILL.md`
5. 完成后运行 `pixi run build-debug && ./build/debug/tests/test_load_case`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
