# T70 共享状态同步策略（冻结版）

> 任务：T70  
> 日期：2026-04-05  
> 状态：冻结完成（done）  
> 前置：T67, T69

---

## 1. 背景

T67（DocumentSnapshot）、T68（WorkerGroup）、T69（ResultChannel）已构建完整的
后台任务输入/调度/输出通道。T70 的目标是：

- 在代码注释和策略文档中冻结主线程与后台线程之间共享状态的同步规则
- 实现 `SceneUpdateAdapter`，封装主线程消费 ResultChannel 的协议
- 为 T71（RecomputeEngine 异步管线重构）提供明确的契约

---

## 2. 关键并发访问点分析

### 2.1 DependencyGraph dirty 集合

| 操作 | 调用线程 | 同步要求 |
|------|----------|---------- |
| `markDirty()` | **主线程** | dirty_ 集合非线程安全，仅主线程写 |
| `clearDirty()` | **主线程** | 必须与 `collectDirty()` 在同一主线程帧内原子执行 |
| `collectDirty()` | **主线程** | 返回值副本（vector<UUID>），可安全传入快照 |
| `isDirty()` | **主线程** | 只读，但 dirty_ 无锁保护，仍限主线程 |

**关键约束**：`collectDirty()` 与 `clearDirty()` 必须在同一主线程执行帧内顺序调用，
不允许被其他写操作打断。此序列是构建快照前的"快照窗口"。

### 2.2 DocumentSnapshot 构建入口

`makeDocumentSnapshot(doc, graph)` 必须在主线程调用，执行顺序如下：

```
[主线程帧开始]
  ┌─ graph.collectDirty()          → 获取待重算 UUID 列表（不清除标记）
  ├─ makeDocumentSnapshot(doc, graph) → 值拷贝所有对象数据 + dirtyIds
  ├─ graph.clearDirty()            → 清除标记（下一轮命令才会重新标脏）
  └─ workers.submit(snap, applyFn) → 提交后台任务（snap 不可变副本）
[主线程帧结束] → 后台线程持有只读 snap，主线程继续接受命令
```

快照完成后，Document 和 DependencyGraph 对象无需锁保护，
因为后台线程不持有对它们的任何引用（只持有快照副本）。

### 2.3 ResultChannel 场景提交边界

ResultChannel 已有内部 `std::mutex` 保护队列，提供线程安全的 post/drain 握手。

| 操作 | 调用线程 | 说明 |
|------|----------|------|
| `ResultChannel::post(version, applyFn)` | **后台线程** | 任务计算完成后调用 |
| `ResultChannel::drainFresh(currentVersion)` | **主线程** | 版本匹配则执行 applyFn |
| `SceneUpdateAdapter::drain()` | **主线程** | 封装 drainFresh(versionProvider_()) |

**applyFn 执行线程**：applyFn 在 drainFresh() 内部调用，因此在**主线程**执行。
applyFn 可安全调用 SceneManager、Document 等主线程专属对象。

---

## 3. SceneUpdateAdapter 设计

`task::SceneUpdateAdapter` 封装主线程消费 ResultChannel 的协议：

```cpp
// 初始化（主线程）
task::SceneUpdateAdapter adapter(resultChannel,
    [&doc]() { return doc.currentVersion(); });

// 事件循环（主线程，每帧或 QueuedConnection 触发）
std::size_t applied = adapter.drain();
// applied == 0: 无新鲜结果，或全部已过期丢弃
```

### 3.1 版本校验规则（由 ResultChannel 执行）

```
if (submittedVersion == doc.currentVersion())
    执行 applyFn  →  更新 SceneManager 节点
else
    静默丢弃      →  不写入任何共享状态
```

### 3.2 强制刷新场景

```cpp
adapter.drainAll();   // 跳过版本校验，强制执行所有 applyFn
adapter.discard();    // 丢弃所有结果，不执行任何 applyFn（文档关闭等）
```

---

## 4. 禁止行为清单（T70 补充约束）

在 T52 已有的禁止清单基础上，T70 补充以下约束：

| 禁止行为 | 说明 |
|----------|------|
| 后台线程调用 `DependencyGraph::markDirty/clearDirty/collectDirty` | dirty_ 集合非线程安全 |
| 后台线程调用 `makeDocumentSnapshot()` | 必须在主线程执行（doc 非线程安全） |
| 主线程以外调用 `SceneUpdateAdapter::drain*` 或 `discard` | versionProvider_() 中的 Document::currentVersion() 非线程安全 |
| `collectDirty()` 与 `clearDirty()` 之间插入命令执行 | 破坏快照窗口原子性 |
| applyFn 中启动新的后台线程 | applyFn 必须在主线程内同步完成 |

---

## 5. 迁移期现状（T71 实现前）

当前 `RecomputeEngine::recompute()` 仍在主线程同步执行几何推导，
`SceneUpdateAdapter` 尚未接入实际的异步路径。

T71 完成后的目标调用链：

```
命令执行 (主线程)
  → DependencyGraph::markDirty()        [主线程]
  → graph.collectDirty() + makeDocumentSnapshot() + graph.clearDirty()  [主线程]
  → workers.submit(snap, ...)           [主线程提交]
  → [后台线程] 几何推导 → channel.post(version, applyFn)
  → [主线程事件循环] adapter.drain()    → applyFn → SceneManager::batchUpdate()
```

T70 的注释与策略文档为此路径提供明确的前置规范。

---

## 6. 交付文件

| 文件 | 说明 |
|------|------|
| `src/lib/runtime/task/SceneUpdateAdapter.h` | 主线程场景更新适配器接口（含完整同步边界注释） |
| `src/lib/runtime/task/SceneUpdateAdapter.cpp` | 实现（委托 ResultChannel）|
| `src/lib/runtime/app/DependencyGraph.h` | dirty 方法添加主线程所有权注释 |
| `src/lib/runtime/app/DocumentSnapshot.h` | makeDocumentSnapshot 添加主线程序列注释 |
| `src/apps/pipecad/engine/RecomputeEngine.h` | 添加主线程独占与 T71 迁移说明注释 |
| `src/lib/runtime/runtimeMod/pipecad.runtime.task.cppm` | 模块边界新增 SceneUpdateAdapter 前向声明导出 |
| `tests/test_runtime_tasking.cpp` | 新增 6 个 SceneUpdateAdapter 单测 |
| `docs/tasks/phase4-lib-app-refactor/t70-sync-policy.md` | 本文件（策略冻结文档） |
