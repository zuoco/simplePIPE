# Phase 4 完成记录 — lib/apps 架构重构 (T50–T77)

> **实施计划书**: `docs/lib-app-refactor-plan.md`
> **任务卡目录**: `docs/tasks/phase4-lib-app-refactor/`
> **任务状态**: `docs/tasks/status.md`

---

### T50 — 冻结目录与目标命名规则 (2026-04-05)

**产出文件**: `docs/tasks/phase4-lib-app-refactor/t50-directory-target-freeze.md` · `docs/tasks/phase4-lib-app-refactor/m0-rule-freeze.md` · `docs/lib-app-refactor-plan.md` · `src/CMakeLists.txt`

**接口**: → `docs/tasks/phase4-lib-app-refactor/t50-directory-target-freeze.md`

**设计决策**:
- 确立 `src/lib` 与 `src/apps` 为唯一长期代码根目录
- 架构统一静态库固定命名 `pipecad_lib`
- 当前 app 业务库固定命名 `pipecad_app`，可执行固定命名 `pipecad`
- 新增 app 采用 `<name>_app`（业务库）+ `<name>`（可执行）命名对
- 新增 app 目录模板固定为 `src/apps/<name>/model|engine|workbench|ui|resources|main.cpp`
- 旧 target（`app`、`command`）过渡期保留 ALIAS 壳，在 T75 清理

**已知限制**:
- 本任务仅冻结规则文档，代码目录结构与构建拓扑仍在旧结构，需等 T53–T62 逐步落地
