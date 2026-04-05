# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 第一个任务 **T50** 已全部完成。

**T50 完成摘要（2026-04-05）**：
- 已冻结 `src/lib` / `src/apps` 为唯一长期代码根目录
- 已冻结目标命名：`pipecad_lib`（统一架构库）、`pipecad_app`（业务库）、`pipecad`（可执行）
- 已明确新增 app 采用 `<name>_app` + `<name>` 命名对及目录模板
- 冻结文档：`docs/tasks/phase4-lib-app-refactor/t50-directory-target-freeze.md`
- 编译 219/219，测试 41/41 全部通过

**当前基线事实**：
- 代码仍处于旧目录结构：`src/foundation`、`src/geometry`、`src/model`、`src/engine`、`src/app`、`src/command`、`src/ui`
- T50 规则已冻结，T51/T52 均处于 `ready` 状态
- `app` 静态库仍吞并 `command` 与 `RecomputeEngine.cpp`（将在 T54 解除）

## 下一个任务

**T51 — 冻结 include/import 规则**

工作目标：
1. 定义 `lib` 与 `apps` 的允许依赖路径和禁止路径
2. 明确 apps 不允许直接 include OCCT/VSG/VTK 头文件
3. 明确 `lib::runtime` 不允许反向依赖 `lib::framework`
4. 明确不允许跨 app 直接依赖

推荐模型：**GPT-5.3 Codex**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/t50-directory-target-freeze.md`
2. `docs/tasks/phase4-lib-app-refactor/m0-rule-freeze.md`
3. `docs/lib-app-refactor-plan.md`
4. `docs/tasks/status.md`
5. `src/CMakeLists.txt`
6. `src/app/CMakeLists.txt`
