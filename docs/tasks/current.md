# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50–T73** 已全部完成。

**T73 完成摘要（2026-04-06）**：
- 在 `RecomputeEngine` 新增 `asyncRecomputeAll()` 方法
- 无异步模式时退化为同步 `recomputeAll()`；有异步模式时标记所有 PipePoint 为脏 → 触发 `asyncRecompute()`
- 同步更新兼容 shim `src/engine/RecomputeEngine.h`（避免 include 路径选取旧版本声明）
- 新增 `tests/test_load_save_async.cpp`（4 个 T73 测试）+ CMakeLists.txt 更新
- 测试基线提升到 **45/45**（新增 LoadSaveAsync 测试套件）

**当前基线事实**：
- `asyncRecomputeAll()` 位于 `src/apps/pipecad/engine/RecomputeEngine.h/.cpp`
- 兼容 shim `src/engine/RecomputeEngine.h` 已同步声明 `asyncRecomputeAll()`
- T74（建立并发回归测试）所有前置（T71/T72/T73）已 done，状态 `ready`

## 下一个任务

**T74 — 建立并发回归测试**

工作目标：
为异步重算、取消、过期结果丢弃和退出回收建立测试覆盖。至少覆盖以下四类场景：
1. **批量编辑**：连续执行多个命令 → 多次 asyncRecompute() → 所有结果正确交付
2. **撤销重做**：undo/redo 后文档版本提升 → 旧版本结果被丢弃（ResultChannel.drainFresh 版本校验）
3. **工作台切换**：切换时丢弃挂起结果（resultChannel.discard()）→ 再次重算正确
4. **应用退出**：workers.shutdown() + resultChannel.discard() → 后台线程安全回收，不崩溃

参考任务卡：`docs/tasks/phase4-lib-app-refactor/m5-async-recompute.md`（T74 章节）

推荐模型：**Claude Sonnet 4.6**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/m5-async-recompute.md`（T74 章节，验收标准）
2. `src/lib/runtime/task/TaskQueue.h`（WorkerGroup + CancellationToken 接口）
3. `src/lib/runtime/task/ResultChannel.h`（drainFresh/discard 接口）
4. `src/lib/runtime/task/SceneUpdateAdapter.h`（drain 接口）
5. `src/apps/pipecad/engine/RecomputeEngine.h`（asyncRecompute/asyncRecomputeAll/drainResults 接口）
6. `tests/test_batch_mesh.cpp`（T72 测试参考：WorkerGroup 并发测试模式）
7. `tests/test_async_recompute.cpp`（T71 测试参考：版本过期丢弃模式）
8. `tests/CMakeLists.txt`（添加新测试目标的格式参考）

