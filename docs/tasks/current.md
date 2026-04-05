# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）、Phase 2（T30-T45）、Phase 3（T0-T10）以及 Phase 4 的 **T50–T72** 已全部完成。

**T72 完成摘要（2026-04-05）**：
- `main.cpp` asyncFn 重写：从「一个 WorkerGroup 任务串行处理所有脏 ID」改为「每个脏 ID 独立提交一个任务」（真正并行批量重算）
- 每个后台任务完整路径：`deriveFromSnapshot()` → `toVsgGeometry()`（含 ShapeMesher::mesh()）→ `createComponentNode()`，ShapeMesher 不再占用主线程
- ResultChannel applyFn（主线程）只做 `sceneManager.addNode/updateNode(uuid, vsgNode)`，零几何运算
- 使用 `std::make_shared<DocumentSnapshot>` 共享快照，避免 N 次深拷贝
- 新增 `tests/test_batch_mesh.cpp`（4 个 T72 测试）+ CMakeLists.txt 更新
- 测试基线提升到 **44/44**（新增 BatchMesh 测试套件）

**当前基线事实**：
- asyncFn 注入点：`recomputeEngine.enableAsyncMode(asyncFn, drainFn)` 位于 `src/apps/pipecad/main.cpp`
- 同步路径（`recompute()` / `recomputeAll()`）通过 `setSceneUpdateCallback` 不变
- `sceneUpdateFn`（原 `sceneCb`）仍用于同步路径；异步路径直接捕获 `&sceneManager`
- T73（后台化加载恢复与保存前准备）已标为 `ready`，T74 仍为 `pending`（需 T73 完成）

## 下一个任务

**T73 — 后台化加载恢复与保存前准备**

工作目标：
1. 工程加载（`ProjectSerializer::load()`）后的几何恢复阶段异步化：加载完成后向后台提交批量 recompute 任务，不阻塞主线程
2. 保存前准备（如 STEP 导出 mesh 化）可选后台化
3. 保持同步降级路径（`recomputeAll()` 在无异步模式时直接调用）

参考任务卡：`docs/tasks/phase4-lib-app-refactor/m5-async-recompute.md`（T73 章节）

推荐模型：**Claude Sonnet 4.6**

## 需要读取的文件

1. `docs/tasks/phase4-lib-app-refactor/m5-async-recompute.md`（任务卡 T73 章节）
2. `src/lib/framework/app/ProjectSerializer.h`（或 `src/app/ProjectSerializer.h`）（加载接口）
3. `src/apps/pipecad/main.cpp`（当前 asyncFn 实现参考，找 ProjectSerializer 加载回调位置）
4. `src/apps/pipecad/engine/RecomputeEngine.h`（接口参考）
5. `src/lib/runtime/task/TaskQueue.h`（WorkerGroup 接口）
6. `src/lib/runtime/task/ResultChannel.h`（结果回投）
7. `docs/tasks/status.md`
