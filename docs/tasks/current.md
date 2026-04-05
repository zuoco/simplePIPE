# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50–T70** 已全部完成。

**T70 完成摘要（2026-04-05）**：
- 实现 `task::SceneUpdateAdapter`（`src/lib/runtime/task/`），封装主线程消费 ResultChannel 的协议
- `SceneUpdateAdapter` 通过 `VersionProvider` 函数对象获取当前文档版本，调用 `drainFresh(version)` 完成版本校验
- 在 `DependencyGraph.h` dirty 方法添加主线程独占注释，冻结 `collectDirty → clearDirty` 原子序列要求
- 在 `DocumentSnapshot.h` 的 `makeDocumentSnapshot` 添加"快照窗口"协议注释（正确的调用顺序）
- 在 `RecomputeEngine.h` 添加主线程独占原因注释（OCCT 非线程安全）和 T71 迁移说明
- `pipecad.runtime.task` 模块边界新增 `SceneUpdateAdapter` 前向声明导出
- `tests/test_runtime_tasking.cpp` 新增 6 个 SceneUpdateAdapter 单测（版本匹配、延迟查询、drainAll、discard、WorkerGroup 联动、pendingCount）
- 同步策略冻结文档：`docs/tasks/phase4-lib-app-refactor/t70-sync-policy.md`

**当前基线事实**：
- `src/apps/pipecad/` 承载完整业务闭包（model/engine/ui/main）
- `DocumentSnapshot`（T67）+ `WorkerGroup`（T68）+ `ResultChannel`（T69）+ `SceneUpdateAdapter`（T70）构成异步重算的完整输入/调度/输出/消费通道
- T70 冻结的"快照窗口"协议（collectDirty → makeDocumentSnapshot → clearDirty → submit）是 T71 重构 RecomputeEngine 的关键规范
- `lib_base_modules`、`lib_platform_occt_modules`、`lib_platform_vsg_modules`、`lib_platform_vtk_modules`、`lib_runtime_modules`、`lib_framework_modules` 已全部存在
- 当前测试基线为 `42/42`
- ready 任务：T71（重构 RecomputeEngine 异步管线）
- 可执行路径：`build/debug/src/apps/pipecad/pipecad`

## 下一个任务

**T71 — 重构 RecomputeEngine 异步管线**

工作目标：
1. 将 `RecomputeEngine::recompute()` 改造为异步管线：
   - 主线程：`collectDirty` → `makeDocumentSnapshot` → `clearDirty` → 提交后台任务
   - 后台线程：基于 `DocumentSnapshot` 推导几何（调用 OCCT 计算函数）
   - 后台线程：计算完成后通过 `ResultChannel::post(version, applyFn)` 回投
   - 主线程（事件循环）：通过 `SceneUpdateAdapter::drain()` 消费结果并更新 SceneManager
2. 维持现有 `recomputeAll()` 路径作为同步全量刷新的降级模式
3. 确保编译通过，`42/42` 测试基线不退步（可添加新并发测试）

核心挑战：
- `RecomputeEngine` 目前持有 `Document&` 和 `DependencyGraph&` 可变引用；异步化后需改为在提交任务前构建快照，后台只访问快照
- `SceneUpdateCallback` 目前是同步回调；异步化后需改为通过 `ResultChannel` 传递，主线程调用时触发场景更新
- 需要在 `WorkerGroup` 上持有 `SceneUpdateAdapter` 引用，并在主线程事件循环（或定时器）中驱动 `drain()`

推荐模型：**Claude Sonnet 4.6**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/t70-sync-policy.md`（T70 冻结的同步策略，必读）
2. `docs/tasks/phase4-lib-app-refactor/t52-thread-boundary-freeze.md`（T52 线程边界规则）
3. `src/apps/pipecad/engine/RecomputeEngine.h`（当前接口）
4. `src/apps/pipecad/engine/RecomputeEngine.cpp`（当前实现）
5. `src/lib/runtime/app/DocumentSnapshot.h`（快照契约）
6. `src/lib/runtime/task/ResultChannel.h`（结果回投通道）
7. `src/lib/runtime/task/SceneUpdateAdapter.h`（主线程消费适配器）
8. `src/lib/runtime/task/TaskQueue.h`（WorkerGroup 接口）
9. `src/apps/pipecad/engine/GeometryDeriver.h`（当前几何推导接口）
10. `docs/tasks/status.md`
