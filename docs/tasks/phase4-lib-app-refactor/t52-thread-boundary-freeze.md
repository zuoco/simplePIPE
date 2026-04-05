# T52 冻结线程安全边界（冻结版）

> 任务：T52  
> 日期：2026-04-05  
> 状态：冻结完成（done）  
> 前置：T50

---

## 1. 适用范围

本文档冻结 PipeCAD Phase 4 重构过程中的线程所有权规则，作为 T68-T71（并发基础设施）、M4（后台重算落地）等后续实现任务的前置约束。

规则适用于：
1. 主线程（Qt 事件线程 / GUI 线程）持有的可变对象与写操作
2. 后台线程允许执行的操作
3. 后台线程将结果回投主线程的边界协议
4. 失效（stale）结果的丢弃规则

---

## 2. 核心原则

### 2.1 单一可变状态线程

**所有可变文档状态、命令状态、事务状态、场景状态、Qt/QML 状态，均由主线程独占持有和修改。**

任何写操作，无论来自 UI 交互、命令执行还是重算回调，必须在主线程完成。

### 2.2 后台线程只读快照原则

**后台线程只允许在不可变快照（snapshot）或只读接口（const 方法）上工作。**

后台线程不持有对 Document、CommandStack、SceneManager 等可变对象的直接引用。

### 2.3 结果回投边界原则

**后台线程将计算结果提交到主线程的唯一合法通道为：Qt 信号（Q_EMIT）或 Qt::QueuedConnection 连接的槽函数。**

### 2.4 失效丢弃原则

后台线程计算期间，若主线程侧文档状态已被修改（版本号变化或对象被删除），则后台结果回投时必须丢弃，不得写入文档或场景。

---

## 3. 主线程专属对象清单

以下对象及其写操作**只允许在主线程调用**：

| 对象 | 类/命名空间 | 主线程专属原因 | 写操作示例 |
|------|------------|--------------|----------|
| 命令栈 | `command::CommandStack` | OCCT 非线程安全；execute/undo/redo 触发 OCCT 几何运算 | `execute()`, `undo()`, `redo()` |
| 文档容器（写） | `app::Document` | 对象增删操作不可重入 | `addObject()`, `removeObject()` |
| 依赖图（写） | `app::DependencyGraph` | 边的增删需在文档写锁内完成 | `addEdge()`, `removeNode()` |
| 重算引擎 | `engine::RecomputeEngine` | 直接调用 OCCT 建模函数 | `recompute()`, `recomputeAll()` |
| VSG 场景管理器（写） | `visualization::SceneManager` | VSG Vulkan 命令录制必须在固定线程执行 | `addNode()`, `updateNode()`, `removeNode()`, `batchUpdate()` |
| 拾取与高亮 | `visualization::PickHandler` | 依赖 VSG 场景图状态 | 所有写方法 |
| 视图管理器 | `visualization::ViewManager` | VSG 渲染器帧循环所在线程 | `setCamera()`, `render()` |
| 选择管理器（写） | `app::SelectionManager` | QML 属性变更通知必须回到主线程 | `setSelection()`, `clearSelection()` |
| 工作台管理器 | `app::WorkbenchManager` | 工作台切换触发 QML 模型更新 | `switchWorkbench()` |
| Application 单例 | `app::Application` | 持有上述所有对象 | `init()`, `destroy()` |
| 所有 Qt 对象 | `QObject` 子类 | Qt 线程亲和性要求 | 所有属性写入与信号发射 |
| QML 对象 | `QQuickItem` 子类 | QML 引擎线程绑定（通常为主线程） | 所有属性赋值 |
| VTK 视图 | `vtk_visualization::VtkViewport` | VTK 渲染管道必须单线程调用 | 所有 VTK 渲染相关写操作 |

---

## 4. 后台线程允许行为清单

后台线程（通过 `std::thread`、`QThread`、`QtConcurrent::run` 或 `std::async` 启动）**只允许**执行以下操作：

### 4.1 允许：消费文档只读快照

- 读取由主线程在任务提交前拷贝/冻结的不可变文档快照数据结构
- 调用文档对象的 `const` 方法

### 4.2 允许：调用 `const` 接口

- `app::Document::findObject()`（只读查找）
- `app::Document::findByType<T>()` const 版本
- `model::PipePoint`、`model::PipeSpec` 等对象的只读访问器
- `app::DependencyGraph` 的只读查询

### 4.3 允许：独立的几何推导计算

- 基于快照数据调用 OCCT 几何计算函数（在后台线程内独立上下文中）
- 生成中间的 `TopoDS_Shape` 或网格数据（不写入 SceneManager）
- 准备 VTK 分析数据集

### 4.4 允许：纯内存分配与数据结构构造

- 构造结果对象（形状、网格、分析数据），但不将其写入主线程持有的共享状态

### 4.5 允许：发射结果信号（排队连接）

- 通过 `Qt::QueuedConnection` 向主线程中的槽发送计算结果

---

## 5. 快照边界定义

### 5.1 快照触发时机

快照（Snapshot）由主线程在向后台线程提交任务前同步构建，确保不可变性。

触发时机：
1. 用户发起重算请求时（命令执行后 RecomputeEngine 触发点）
2. 分析工作台提交应力计算调用前
3. 导出任务（STEP 导出、VTK 导出）发起前

### 5.2 快照需包含的数据

| 数据项 | 说明 |
|--------|------|
| 文档版本号（version token） | 用于回投时对比，判断结果是否失效 |
| PipePoint 列表（值拷贝） | 含坐标、类型、UUID |
| PipeSpec 参数（值拷贝） | 管径、壁厚等几何参数 |
| 依赖关系拓扑（只读视图或副本） | Segment/Route 连接关系 |

### 5.3 快照不包含的内容

- SceneManager 的可变节点引用
- CommandStack、Document 的可变引用
- Qt/QML 对象引用

---

## 6. 结果回投主线程协议

### 6.1 回投通道

**唯一合法通道**：`Qt::QueuedConnection` 信号槽调用，或等效的 `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`。

禁止：
- 后台线程直接调用主线程对象的方法（无论是否加锁）
- 后台线程直接写 SceneManager、Document

### 6.2 回投数据类型

后台线程将以下数据类型通过信号传递到主线程：

| 数据类型 | 对应后台任务 | 主线程接收后操作 |
|----------|------------|----------------|
| `std::vector<std::pair<UUID, TopoDS_Shape>>` | 几何重算结果 | 校验版本 → `SceneManager::batchUpdate()` |
| `std::vector<std::pair<UUID, MeshData>>` | 网格化结果 | 校验版本 → 场景节点更新 |
| VTK 分析数据集 | 应力计算结果 | 校验版本 → `VtkViewport` 更新 |

### 6.3 失效结果丢弃规则

回投时，主线程接收方必须执行以下检查：

```
检查版本号是否一致（token_at_submit == document.currentVersion()）
  是 → 正常写入场景和文档
  否 → 静默丢弃，不写入任何状态，可选地触发新一轮重算
```

同时检查：
- 对应 UUID 的文档对象是否仍然存在（`Document::findObject()` 非 nullptr）
- 否则丢弃该对象的结果

---

## 7. 禁止行为清单

| 禁止行为 | 说明 |
|----------|------|
| 后台线程调用 `CommandStack::execute/undo/redo` | 违反命令栈主线程所有权 |
| 后台线程写 `Document::addObject/removeObject` | 文档容器非线程安全 |
| 后台线程写 `SceneManager::addNode/updateNode/removeNode` | VSG 命令录制必须在固定线程 |
| 后台线程持有 `Application::instance()` 的非 const 引用并修改状态 | Application 单例写访问主线程独占 |
| 后台线程调用 QObject 方法（除信号发射外） | Qt 线程亲和性要求 |
| 后台线程直接访问 QML 属性 | QML 引擎线程绑定 |
| 不经快照直接读取 `Document` 的实时可变状态 | 数据竞争风险（即使只读，Document 写锁不能跨线程推断） |

---

## 8. 迁移期约束

当前（Phase 4 起始基线）整个系统为单线程执行：
- 命令执行 → RecomputeEngine::recompute → SceneManager 更新，全部在主线程事件循环中串行执行

阶段性放开规则：
1. **T68 之前**：不允许任何后台线程访问 Document/SceneManager，现有串行路径保持不变
2. **T68-T71（并发基础设施）**：引入快照机制和任务队列，但几何计算仍在主线程完成，仅框架搭建
3. **M4 全面落地后**：符合本文档规则的后台几何重算链路才可启用

---

## 9. 与后续任务关系

| 任务 | 依赖本文内容 | 说明 |
|------|------------|------|
| T53 | 间接 | CMake 拓扑应预留后台任务队列目标位置 |
| T68 | 直接 | 基于本文快照定义实现 `DocumentSnapshot` 类型 |
| T69 | 直接 | 基于本文任务队列协议实现后台任务调度器 |
| T70 | 直接 | 基于本文结果回投协议实现 `SceneUpdateAdapter` |
| T71 | 直接 | 基于本文失效丢弃规则实现版本令牌机制 |
| M4 | 全依赖 | 后台几何重算基础设施的整体实现前提 |
