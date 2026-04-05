# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50、T51、T52** 已全部完成。

**T52 完成摘要（2026-04-05）**：
- 已创建线程安全边界冻结文档：`docs/tasks/phase4-lib-app-refactor/t52-thread-boundary-freeze.md`
- 已冻结主线程专属对象清单：CommandStack、Document 写接口、DependencyGraph 写接口、RecomputeEngine、SceneManager 写操作、SelectionManager 写接口、WorkbenchManager、所有 Qt/QML/VTK 对象
- 已冻结后台线程允许行为：只读快照消费、const 方法、独立 OCCT 几何推导（基于快照）、Qt::QueuedConnection 信号发射
- 已冻结快照边界：版本令牌 + PipePoint/PipeSpec 值拷贝 + 依赖拓扑只读视图
- 已冻结结果回投协议与失效丢弃规则

**当前基线事实**：
- 代码仍处于旧目录结构：`src/foundation`、`src/geometry`、`src/model`、`src/engine`、`src/app`、`src/command`、`src/ui`
- `app` 静态库仍吞并 `command` 与 `RecomputeEngine.cpp`（将在 T54 解除）
- `src/CMakeLists.txt` 已有 T50 命名规则注释，但 target 拓扑仍是旧结构
- T53 现处于 `ready`（T50、T51、T52 均已完成）

## 下一个任务

**T53 — 设计 lib/apps 顶层 CMake 拓扑**

工作目标：
1. 设计新的顶层 CMake 装配方案（不大迁目录，先形成 lib/apps 逻辑结构）
2. 规划 `pipecad_lib` 统一架构库如何聚合现有各子库
3. 规划 `pipecad_app`（业务库）与 `pipecad`（可执行）的构建目标结构
4. 保留旧 target 别名（ALIAS）实现向下兼容，确保编译仍通过
5. 输出 CMake 拓扑设计文档，作为 T54-T62 实施的前置约束

推荐模型：**GPT-5.3 Codex**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/t50-directory-target-freeze.md`
2. `docs/tasks/phase4-lib-app-refactor/t51-include-import-freeze.md`
3. `docs/tasks/phase4-lib-app-refactor/t52-thread-boundary-freeze.md`
4. `docs/tasks/phase4-lib-app-refactor/m0-rule-freeze.md`
5. `docs/lib-app-refactor-plan.md`
6. `src/CMakeLists.txt`
7. `src/app/CMakeLists.txt`
8. `src/engine/CMakeLists.txt`
9. `src/command/CMakeLists.txt`
10. `docs/tasks/status.md`
