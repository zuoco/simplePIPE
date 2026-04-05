# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50–T71** 已全部完成。

**T71 完成摘要（2026-04-05）**：
- `RecomputeEngine` 新增 `enableAsyncMode(AsyncFn, DrainFn)` 二元组注入接口
- `AsyncFn = std::function<void()>` 封装完整异步管线（快照 → clearDirty → 后台提交 → ResultChannel 回投），由 `main.cpp` 注入，避免 `pipecad_app_engine` 对 `lib_runtime` 的循环依赖
- `asyncRecompute()` 实现：有脏对象时调用 `asyncFn_()`，无异步模式时退化同步
- `GeometryDeriver::deriveFromSnapshot()` 新增静态方法，接受只读 `PipePointSnapshot/PipeSpecSnapshot`，后台线程安全
- `main.cpp` 内 `asyncFn` 完整封装 T70 快照窗口协议，WorkerGroup 后台推导 + ResultChannel 回投 + QTimer drain（每 16ms）
- 10 个 `AsyncRecompute` 测试均通过
- 当前测试基线为 **43/43**

**当前基线事实**：
- `src/apps/pipecad/engine/RecomputeEngine.h` 接口：`enableAsyncMode(AsyncFn, DrainFn)` + `asyncRecompute()` + `drainResults()` + `recompute()` + `recomputeAll()`
- `src/apps/pipecad/engine/GeometryDeriver.h` 接口：`deriveGeometry()`（同步）+ `deriveFromSnapshot()`（异步快照版本）
- `src/engine/` 下的两个 shim 头文件已同步为最新接口（含 `deriveFromSnapshot` 声明）
- `src/engine/GeometryDeriver.h` 与 `src/apps/pipecad/engine/GeometryDeriver.h` 是两个独立文件，不互相 include（`main.cpp` 不能显式 include `"engine/GeometryDeriver.h"`，否则因相对路径解析产生重定义）
- T72（后台化 ShapeMesher 与批量重算）状态已标为 `ready`
- 可执行路径：`build/debug/src/apps/pipecad/pipecad`

## 下一个任务

**T72 — 后台化 ShapeMesher 与批量重算**

工作目标：
1. 将 `ShapeMesher`（网格化）移入后台线程执行，不阻塞主线程
2. 批量重算时，多个脏对象的几何推导并行执行（利用 WorkerGroup 多线程）
3. 结果通过 `ResultChannel` 回投，主线程 drain 直接更新场景
4. 保持 `recomputeAll()` 同步降级路径

参考任务卡：`docs/tasks/phase4-lib-app-refactor/m5-async-recompute.md`

推荐模型：**Claude Sonnet 4.6**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/m5-async-recompute.md`（任务卡）
2. `src/apps/pipecad/engine/RecomputeEngine.h`（当前接口）
3. `src/apps/pipecad/engine/RecomputeEngine.cpp`（当前实现）
4. `src/apps/pipecad/engine/GeometryDeriver.h`（当前接口，含 deriveFromSnapshot）
5. `src/apps/pipecad/engine/GeometryDeriver.cpp`（当前实现）
6. `src/lib/runtime/app/DocumentSnapshot.h`（快照契约）
7. `src/lib/runtime/task/TaskQueue.h`（WorkerGroup 接口）
8. `src/lib/runtime/task/ResultChannel.h`（结果回投通道）
9. `src/apps/pipecad/main.cpp`（asyncFn 完整管线参考实现）
10. `docs/tasks/status.md`
