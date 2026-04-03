# 命令模式架构设计文档

> **版本**: 2.0 | **日期**: 2026-04-04 | **状态**: 设计中（已修订）

---

## 1. 概述

### 1.1 背景

当前 `TransactionManager` 仅支持属性级变更（坐标、名称、参数等）的 undo/redo，存在以下局限：

| 问题 | 现状 |
|------|------|
| 对象创建/删除不可撤销 | `Document::addObject()`/`removeObject()` 直接调用，无事务记录 |
| 组件插入流程断裂 | `AppController::insertComponent()` 发出 `insertComponentRequested` 信号，但无 C++ 处理器 |
| 删除流程断裂 | `AppController::deleteSelected()` 发出 `deleteRequested` 信号，但无 C++ 处理器 |
| 属性分派硬编码 | `TransactionManager::applyChanges()` 约 150 行 `dynamic_cast` 链，每增加对象类型都要修改 |
| 无命令序列化 | 不支持脚本回放、宏录制、网络同步 |

### 1.2 目标

引入完整的 **Command 模式**，使所有建模和分析操作可通过命令执行、撤销、重做，并支持命令序列化到 JSON：

1. 统一的命令基类和命令栈管理
2. 支持属性修改、对象创建/删除、拓扑变更、组件插入等全部操作
3. 命令可序列化为 JSON（用于脚本回放、宏录制、持久化）
4. 替代现有 `TransactionManager` 的 undo/redo 功能
5. 分阶段迁移，与现有系统并行运行

### 1.3 设计原则

- **命令存储 ID，不存储指针** — 命令只记录 UUID 和参数，执行时通过 Document 查找对象，确保可序列化、可重放
- **UUID 稳定性** — 所有创建/重建对象的命令必须保证 UUID 不变。`DeletePipePointCommand::undo()` 重建对象时必须使用 `setIdForDeserialization()` 恢复原 UUID，确保 DependencyGraph、TopologyManager 等外部引用不失效
- **CommandContext 注入** — 命令通过 `CommandContext` 获取 Document、DependencyGraph、TopologyManager 等运行时依赖
- **MacroCommand 组合** — 复合操作（如插入三通 = 创建管点 + 创建分支段）作为宏命令原子执行
- **统一工厂注册** — `CommandSerializer` 和 `CommandDispatcher` 共享同一套 `json → unique_ptr<Command>` 工厂注册表，新增命令类型无需修改核心代码
- **属性分派多态** — 属性读写通过 `DocumentObject` 虚方法 `setProperty`/`getProperty` 分派到各模型子类，而非集中式 `dynamic_cast` 链

---

## 2. 核心类设计

### 2.1 类图

```
┌─────────────────────────────────────────────────────────┐
│                     Command (抽象基类)                    │
│  + execute(ctx)  + undo(ctx)  + toJson()  + type()      │
│  + lastResult() → CommandResult                          │
│  + tryMerge(next) → bool                                 │
│  # id_: UUID                                            │
└────────────┬────────────────────────────────────────────┘
             │
    ┌────────┼────────────────────────────────────┐
    │        │                                    │
    ▼        ▼                                    ▼
┌──────────────┐  ┌──────────────────┐  ┌──────────────────────┐
│ SetProperty  │  │ MacroCommand     │  │ CreatePipePoint      │
│ Command      │  │ + addCommand()   │  │ Command              │
└──────────────┘  │ - children_[]    │  └──────────────────────┘
                  └────────────────────────────────┘
                                ▲
                                │ 继承
                  ┌──────────────────────┐
                  │ InsertComponent      │
                  │ Command              │
                  │ type()→Macro         │
                  └──────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                    CommandContext                        │
│  document: Document&                                     │
│  dependencyGraph: DependencyGraph&                       │
│  topologyManager: TopologyManager*                       │
│  sceneRemove: function<void(string)>                     │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                   CommandResult                          │
│  success: bool                                           │
│  errorMessage: string                                    │
│  createdIds / deletedIds / affectedIds: vector<UUID>     │
│  createdObjects: vector<json>                            │
│  + toJson() → json                                       │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                    CommandStack                          │
│  + execute(unique_ptr<Command>) → CommandResult          │
│  + undo()  + redo()                                      │
│  + canUndo()  + canRedo()                                │
│  + openMacro(desc)  + closeMacro()  + abortMacro()       │
│  + markClean()  + isClean()                              │
│  + setMaxStackSize(size_t)                               │
│  - undoStack_: vector<unique_ptr<Command>>               │
│  - redoStack_: vector<unique_ptr<Command>>               │
│  - pendingMacro_: unique_ptr<MacroCommand>               │
│  Signal<> stackChanged                                   │
│  Signal<vector<UUID>> commandCompleted                   │
│  Signal<vector<UUID>> commandUndone                      │
│  Signal<vector<UUID>> commandRedone                      │
│  Signal<string> sceneRemoveRequested                     │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│           CommandRegistry (统一工厂注册表)                 │
│  + registerFactory(name, json→unique_ptr<Command>)        │
│  + create(name, paramsJson) → unique_ptr<Command>        │
│  + createFromFullJson(json) → unique_ptr<Command>        │
│  + serialize(cmd) → json                                 │
│  + serializeSequence(cmds) → json                        │
│  + deserializeSequence(json) → vector<unique_ptr<Command>>│
│  + registeredCommands() → vector<string>                 │
│  - factories_: map<string, Factory>                      │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                   PropertyApplier                        │
│  + static apply(obj, key, value)   → 薄转发到 obj→setProperty│
│  + static read(obj, key) → Variant → 薄转发到 obj→getProperty│
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│              DocumentObject (已有基类，新增虚方法)          │
│  + virtual setProperty(key, value) → bool                │
│  + virtual getProperty(key) → Variant                    │
└─────────────────────────────────────────────────────────┘
```

### 2.2 CommandType 枚举

```cpp
enum class CommandType {
    SetProperty,         ///< 属性修改
    BatchSetProperty,    ///< 批量属性修改
    CreatePipePoint,     ///< 创建管点
    DeletePipePoint,     ///< 删除管点
    CreateRoute,         ///< 创建路由
    DeleteRoute,         ///< 删除路由
    CreateSegment,       ///< 创建段
    DeleteSegment,       ///< 删除段
    CreateLoad,          ///< 创建载荷
    DeleteLoad,          ///< 删除载荷
    CreateAccessory,     ///< 创建附属构件
    DeleteAccessory,     ///< 删除附属构件
    ModifyTopology,      ///< 拓扑变更
    Macro                ///< 宏命令（复合操作，含 InsertComponent）
};
```

> **修订说明**：移除 `InsertComponent` 枚举值。`InsertComponentCommand` 的 `type()` 统一返回 `Macro`，子类型通过 JSON 的 `"componentType"` 字段区分。新增命令只需注册工厂和 JSON 反序列化函数，无需修改此枚举。

### 2.3 CommandContext — 执行上下文

命令不持有 Document/DependencyGraph 等引用，而是在执行时通过 `CommandContext` 获取。这样命令可以序列化、跨文档重放。

```cpp
struct CommandContext {
    Document& document;                                    ///< 文档对象管理器
    DependencyGraph& dependencyGraph;                      ///< 依赖图
    engine::TopologyManager* topologyManager = nullptr;    ///< 拓扑管理器
    std::function<void(const std::string&)> sceneRemove;   ///< 场景节点移除回调（删除操作需立即从视口移除）
};
```

**修订说明**：移除 `sceneAdd`。新增对象的可视化统一通过 `CommandStack::commandCompleted` 信号 → `RecomputeEngine` 路径处理（生成 TopoDS_Shape → 更新 SceneManager）。仅保留 `sceneRemove`，因为删除操作需立即从视口移除节点，不能等 recompute。

`CommandContext` 由调用方（通常在 `main.cpp` 的信号处理器中）构建并注入，而非由 `CommandStack` 持有。

### 2.4 Command 基类

```cpp
class Command {
public:
    virtual ~Command() = default;

    /// 执行命令（正向变更）
    virtual void execute(CommandContext& ctx) = 0;

    /// 撤销命令（恢复到执行前状态）
    virtual void undo(CommandContext& ctx) = 0;

    /// 人类可读描述（显示在 undo 栈 UI 中）
    virtual std::string description() const = 0;

    /// 命令类型标识（用于序列化分派）
    virtual CommandType type() const = 0;

    /// 序列化为 JSON（包含 params + 执行后 state）
    virtual nlohmann::json toJson() const = 0;

    /// 尝试将后续命令合并到本命令（用于连续输入合并）
    /// 返回 true 表示合并成功，调用方不再 push 后续命令
    virtual bool tryMerge(const Command& next) { return false; }

    /// 命令实例唯一 ID
    const foundation::UUID& id() const { return id_; }

    /// 执行后的结果（仅 execute 后有效）
    const CommandResult& lastResult() const { return lastResult_; }

    /// 创建时间戳（用于 tryMerge 时间窗口判断）
    const std::chrono::steady_clock::time_point& timestamp() const { return timestamp_; }

protected:
    Command() = default;
    foundation::UUID id_ = foundation::UUID::generate();
    CommandResult lastResult_;
    std::chrono::steady_clock::time_point timestamp_ = std::chrono::steady_clock::now();
};
```

> **修订说明**：
> - 新增 `tryMerge()` — 默认返回 false。`SetPropertyCommand` 覆写：若 `objectId` + `key` 相同且时间间隔 < 500ms，合并（保留最旧 oldValue + 最新 newValue）。`CommandStack::execute()` 在 push 前尝试合并。
> - 新增 `timestamp_` — 为 tryMerge 的时间窗口提供判断依据。

### 2.5 MacroCommand — 宏命令

将多个子命令组合为一个原子操作。execute 时顺序执行所有子命令，undo 时逆序撤销。**若子命令执行失败，自动回滚已执行的子命令。**

```cpp
class MacroCommand : public Command {
public:
    explicit MacroCommand(std::string desc);
    void addCommand(std::unique_ptr<Command> cmd);

    void execute(CommandContext& ctx) override;   // 顺序执行，异常时回滚
    void undo(CommandContext& ctx) override;      // 逆序撤销子命令
    std::string description() const override;
    CommandType type() const override;              // 返回 CommandType::Macro
    nlohmann::json toJson() const override;

    const std::vector<std::unique_ptr<Command>>& children() const;

protected:
    std::string description_;
    std::vector<std::unique_ptr<Command>> children_;
};
```

**回滚语义**（修订新增）：

```cpp
void MacroCommand::execute(CommandContext& ctx) {
    size_t executed = 0;
    try {
        for (auto& child : children_) {
            child->execute(ctx);
            ++executed;
        }
    } catch (...) {
        // 逆序回滚已执行的子命令
        for (size_t i = executed; i > 0; --i) {
            try {
                children_[i - 1]->undo(ctx);
            } catch (...) {
                // 回滚失败时记录日志但不重新抛出，避免掩盖原始异常
            }
        }
        throw;  // 重新抛出原始异常
    }
}
```

> **修订说明**：MacroCommand 执行失败时必须回滚已执行的子命令，保证文档处于一致状态。回滚中若某个子命令的 undo 也失败，记录日志但不抛出，避免掩盖原始异常。

### 2.6 CommandStack — 命令栈管理器

管理命令的执行、撤销、重做。替代 `TransactionManager` 的 undo/redo 功能。

**修订要点**：
- 不持有 `Document&`、`DependencyGraph&`、`RecomputeCallback` — 通过信号解耦
- 不持有 `SceneAddCallback` — 移除（见 §2.3 说明）
- 新增 `openMacro`/`closeMacro` — 支持交互式多步操作
- 新增 `markClean`/`isClean` — 文档保存状态跟踪
- 新增 `maxSize_` — 可配置栈深度限制

```cpp
class CommandStack {
public:
    // ---- 信号 ----
    foundation::Signal<> stackChanged;                              ///< 栈状态变化（UI 刷新）
    foundation::Signal<const std::vector<foundation::UUID>&> commandCompleted;  ///< 命令执行后（含 affectedIds）
    foundation::Signal<const std::vector<foundation::UUID>&> commandUndone;     ///< 命令撤销后
    foundation::Signal<const std::vector<foundation::UUID>&> commandRedone;     ///< 命令重做后
    foundation::Signal<> cleanStateChanged;                         ///< clean 标记变化
    foundation::Signal<const std::string&> sceneRemoveRequested;    ///< 请求移除场景节点

    explicit CommandStack();  // 不再需要 Document& / DependencyGraph&

    /// 执行命令：尝试合并 → 入 undo 栈 → 清空 redo → emit 信号
    CommandResult execute(std::unique_ptr<Command> cmd);

    /// 撤销最近命令
    void undo();

    /// 重做最近撤销
    void redo();

    // ---- 交互式宏 ----
    void openMacro(const std::string& description);
    void closeMacro();
    void abortMacro();       // 丢弃待定宏，回滚已执行子命令
    bool isMacroOpen() const;

    // ---- 查询 ----
    bool canUndo() const;
    bool canRedo() const;
    std::string nextUndoDescription() const;
    std::size_t undoCount() const;
    std::size_t redoCount() const;
    void clear();

    // ---- Clean 标记（文档保存状态）----
    void markClean();            // 保存时调用，记录当前 undo 栈深度
    bool isClean() const;        // 当前深度是否等于 clean 深度

    // ---- 栈深度限制 ----
    void setMaxSize(std::size_t maxSize);  // 默认 1000，超出时丢弃最旧命令
    std::size_t maxSize() const;

private:
    std::vector<std::unique_ptr<Command>> undoStack_;
    std::vector<std::unique_ptr<Command>> redoStack_;
    std::unique_ptr<MacroCommand> pendingMacro_;
    bool macroOpen_ = false;

    std::size_t maxSize_ = 1000;
    std::size_t cleanIndex_ = 0;   // markClean 时的 undoStack_.size()

    void trimToMaxSize();
};
```

**关键行为**：

| 操作 | 行为 |
|------|------|
| `execute(cmd)` | 若 `macroOpen_`，执行子命令并加入 `pendingMacro_`（不入栈）。否则：尝试 `tryMerge` → 执行 → 入栈 → 清空 redo → trim → emit |
| `undo()` | 从 undoStack_ 取出 → undo(ctx) → 入 redoStack_ → emit `commandUndone` + `stackChanged` |
| `redo()` | 从 redoStack_ 取出 → execute(ctx) → 入 undoStack_ → emit `commandRedone` + `stackChanged` |
| `openMacro(desc)` | 创建空 `pendingMacro_`，设 `macroOpen_=true` |
| `closeMacro()` | 设 `macroOpen_=false`，将 `pendingMacro_` 整体推入 undoStack_ |
| `abortMacro()` | 对 `pendingMacro_` 中已执行的子命令逆序 undo，丢弃 |
| `markClean()` | 记录 `cleanIndex_ = undoStack_.size()` |
| `isClean()` | `return undoStack_.size() == cleanIndex_` |

> **修订说明**：CommandStack 不再持有 Document/DependencyGraph/RecomputeCallback。脏传播和场景更新通过信号在 `main.cpp` 中连线（见 §6.2）。CommandContext 由外部构建并传入 execute/undo。

### 2.7 PropertyApplier — 属性变更薄转发层

替代 `TransactionManager::applyChanges()` 的 150 行 `dynamic_cast` 硬编码分派链。

**修订说明**：不再使用集中式 type-key switch。属性读写通过 `DocumentObject` 虚方法分派到各模型子类，`PropertyApplier` 仅为薄转发层。

```cpp
class PropertyApplier {
public:
    /// 将 key=value 应用到对象上（转发到虚方法）
    static void apply(model::DocumentObject* obj,
                      const std::string& key,
                      const foundation::Variant& value) {
        obj->setProperty(key, value);
    }

    /// 读取对象的指定属性值（转发到虚方法，用于捕获 oldValue）
    static foundation::Variant read(const model::DocumentObject* obj,
                                     const std::string& key) {
        return obj->getProperty(key);
    }
};
```

**DocumentObject 基类新增虚方法**：

```cpp
// model/DocumentObject.h 新增
virtual bool setProperty(const std::string& key, const foundation::Variant& value);
virtual foundation::Variant getProperty(const std::string& key) const;
```

**各模型子类覆写示例**：

| 子类 | 处理的 key | 实现 |
|------|-----------|------|
| `PipePoint` | `"x"/"y"/"z"` | 重构 `gp_Pnt`，调用 `setPosition()` |
| `PipePoint` | `"type"` | `setType(PipePointType(int))` |
| `PipePoint` | `"pipeSpecId"` 及其他 | `setParam(key, value)` |
| `ThermalLoad` | `"installTemp"/"operatingTemp"` | 对应 setter |
| `PressureLoad` | `"pressure"/"isExternal"` | 对应 setter |
| `WindLoad` | `"speed"/"windDirectionX/Y/Z"` | 对应 setter |
| `LoadCombination` | `"category"/"method"` | 枚举 setter |
| `DocumentObject`（基类） | `"name"` | `setName()` |
| 默认 | 未识别的 key | 返回 `false`（setProperty）/ 空 Variant（getProperty） |

**好处**：新增模型类型只需覆写自己的 `setProperty`/`getProperty`，无需修改任何集中式分派代码。

### 2.8 CommandResult — 命令执行结果

每个命令执行后产生结构化结果，用于 UI 反馈和未来 TCP 远程返回。

```cpp
struct CommandResult {
    bool success = false;
    std::string errorMessage;

    // 影响范围
    std::vector<foundation::UUID> createdIds;     // 新创建的对象 ID
    std::vector<foundation::UUID> deletedIds;      // 被删除的对象 ID
    std::vector<foundation::UUID> affectedIds;     // 受影响的对象 ID（含脏对象）

    // 创建的对象快照（供确认）
    std::vector<nlohmann::json> createdObjects;

    nlohmann::json toJson() const;
};
```

`Command` 基类持有 `lastResult_`，`CommandStack::execute()` 返回 `CommandResult`：

```cpp
class Command {
public:
    const CommandResult& lastResult() const { return lastResult_; }
protected:
    CommandResult lastResult_;
};

CommandResult CommandStack::execute(std::unique_ptr<Command> cmd);
```

### 2.9 CommandRegistry — 统一工厂注册表

**修订说明**：将原 `CommandDispatcher` + `CommandFactory` + `CommandRequest` + `CommandSerializer` 四个类合并为统一的 `CommandRegistry`。消除 `CommandRequest` 基类和 `CommandFactory` 抽象接口。

每种命令类型只注册一个 `json → unique_ptr<Command>` 的 lambda。运行时命令执行和序列化反序列化共享同一套注册表。

```cpp
class CommandRegistry {
public:
    using Factory = std::function<std::unique_ptr<Command>(const nlohmann::json&)>;

    /// 注册命令工厂
    void registerFactory(const std::string& typeKey, Factory factory);

    /// 从外部参数 JSON 创建命令（CommandDispatcher 用途）
    /// 将 {"command":"X","params":{...}} 转为完整 JSON 后调用工厂
    std::unique_ptr<Command> createFromParams(const std::string& commandName,
                                              const std::string& paramsJson) const;

    /// 从完整命令 JSON 创建命令（Serializer 用途）
    std::unique_ptr<Command> createFromFullJson(const nlohmann::json& j) const;

    /// 序列化单个命令
    static nlohmann::json serialize(const Command& cmd);

    /// 反序列化单个命令
    std::unique_ptr<Command> deserialize(const nlohmann::json& j) const;

    /// 序列化命令序列（脚本/宏）
    static nlohmann::json serializeSequence(const std::vector<const Command*>& cmds);

    /// 反序列化命令序列
    std::vector<std::unique_ptr<Command>> deserializeSequence(const nlohmann::json& j) const;

    /// 查询已注册的命令名称列表
    std::vector<std::string> registeredCommands() const;
    bool hasCommand(const std::string& name) const;

private:
    std::unordered_map<std::string, Factory> factories_;
};
```

**统一分派流程**：

```
createFromParams("CreatePipePoint", '{"routeId":"uuid","name":"A07","pointType":"Bend","x":5000}')
  │
  ├── 补全为完整 JSON：
  │     {"type":"CreatePipePoint","params":{"routeId":"uuid",...}}
  ├── factories_.find("CreatePipePoint") → Factory lambda
  ├── factory(fullJson) → unique_ptr<CreatePipePointCommand>
  └── 返回命令实例

// 然后由调用方执行：
stack.execute(registry.createFromParams(name, params));
```

**工厂注册示例（在 main.cpp 或 Application 初始化时）**：

```cpp
registry.registerFactory("SetProperty", [](const nlohmann::json& j) {
    auto objId = parseUuid(j.at("objectId"));
    auto key = j.at("key").get<std::string>();
    auto newVal = jsonToVariant(j.at("newValue"));
    // 若有 oldValue 则已知；否则留空由 execute 自动捕获
    if (j.contains("oldValue")) {
        return SetPropertyCommand::createWithOldValue(objId, key, jsonToVariant(j["oldValue"]), newVal);
    } else {
        return SetPropertyCommand::createAutoCapture(objId, key, newVal);
    }
});

registry.registerFactory("Macro", [](const nlohmann::json& j) {
    // 检查是否有 componentType 字段 → InsertComponentCommand
    if (j.contains("componentType")) {
        return std::make_unique<InsertComponentCommand>(j);
    }
    // 否则 → 普通 MacroCommand
    auto macro = std::make_unique<MacroCommand>(j.value("description", ""));
    for (auto& child : j.at("children")) {
        macro->addCommand(CommandRegistry::instance().deserialize(child));
    }
    return macro;
});
```

---

## 3. 具体命令类

### 3.1 SetPropertyCommand — 属性修改命令

替代当前 `TransactionManager::recordChange()` + `open()/commit()` 模式。

**修订说明**：用两个静态工厂方法替代原来的 `autoCapture_` + `captured_` 双布尔标志。内部使用 `std::optional<Variant>` 判断是否已捕获 oldValue。

```
SetPropertyCommand {
    objectId_: UUID                       // 目标对象
    key_: string                            // 属性名（"x", "name", "installTemp" 等）
    oldValue_: optional<Variant>            // 空=待捕获，有值=已知
    newValue_: Variant                      // 新值（redo 用）

    // 静态工厂方法
    static createWithOldValue(objId, key, oldVal, newVal) → unique_ptr<SetPropertyCommand>
    static createAutoCapture(objId, key, newVal) → unique_ptr<SetPropertyCommand>

    // tryMerge 覆写
    tryMerge(next):
        if next is SetPropertyCommand
           && next.objectId_ == this.objectId_
           && next.key_ == this.key_
           && (next.timestamp_ - this.timestamp_) < 500ms:
            this.newValue_ = next.newValue_
            return true
        return false
}

execute(ctx):
    obj = ctx.document.findObject(objectId_)
    if !oldValue_.has_value():           // 首次执行且未提供旧值
        oldValue_ = obj->getProperty(key_)
    obj->setProperty(key_, newValue_)

undo(ctx):
    obj = ctx.document.findObject(objectId_)
    obj->setProperty(key_, oldValue_.value())
```

**JSON 序列化**：
```json
{
    "type": "SetProperty",
    "id": "a1b2c3d4-...",
    "objectId": "e5f6g7h8-...",
    "key": "x",
    "oldValue": {"type": "double", "value": 1000.0},
    "newValue": {"type": "double", "value": 1500.0}
}
```

### 3.2 BatchSetPropertyCommand — 批量属性修改

多个属性变更作为一个原子 undo 单元。UI 表格编辑多列时使用。

```
BatchSetPropertyCommand {
    description_: string
    changes_: vector<Change>    // 每个 Change = {objectId, key, oldValue, newValue}
}

execute(ctx):
    for each change in changes_:
        PropertyApplier::apply(obj, change.key, change.newValue)

undo(ctx):
    for each change in reverse(changes_):
        PropertyApplier::apply(obj, change.key, change.oldValue)
```

### 3.3 CreatePipePointCommand — 创建管点

```
CreatePipePointCommand {
    // 输入参数
    routeId_: UUID
    segmentId_: UUID
    name_: string
    type_: PipePointType (Run/Bend/Reducer/Tee/Valve/FlexJoint)
    x_, y_, z_: double
    pipeSpecId_: string
    insertIndex_: size_t     // SIZE_MAX = 追加到末尾

    // 执行后填充
    createdPointId_: UUID
    createdBranchId_: UUID   // Tee 创建的分支段 UUID（空 if not Tee）
}

execute(ctx):
    1. 创建 PipePoint(name, type, gp_Pnt(x,y,z))
    2. 若 pipeSpecId 非空，查找 PipeSpec 并关联
    3. ctx.document.addObject(pp)
    4. 查找 Route 和 Segment
    5. ctx.topologyManager->appendPoint(route, segment, pp)
       - 若 Tee，自动创建分支段 → 记录 createdBranchId_
    6. 记录 createdPointId_ = pp.id()
    7. 显式注册 DependencyGraph 依赖：
       ctx.dependencyGraph.addDependency(createdPointId_, 前邻管点ID)
       ctx.dependencyGraph.addDependency(createdPointId_, 后邻管点ID)

undo(ctx):
    1. 若 createdBranchId_ 非空，移除分支段
    2. ctx.topologyManager->removePoint(route, createdPointId_)
    3. ctx.document.removeObject(createdPointId_)
    4. ctx.dependencyGraph.removeObject(createdPointId_)
    5. ctx.sceneRemove(createdPointId_.toString())
```

> **修订说明**：新增步骤 7 — 命令内部显式调用 `dependencyGraph.addDependency()` 注册与相邻管点的依赖关系，确保新管点参与脏传播。

**JSON 序列化**：
```json
{
    "type": "CreatePipePoint",
    "id": "uuid",
    "routeId": "uuid",
    "segmentId": "uuid",
    "name": "PP_001",
    "pointType": "Bend",
    "x": 1000.0, "y": 0.0, "z": 500.0,
    "pipeSpecId": "",
    "insertIndex": 3,
    "createdPointId": "uuid",
    "createdBranchId": ""
}
```

### 3.4 DeletePipePointCommand — 删除管点

execute 前捕获完整管点状态，用于 undo 重建。

```
PipePointState {
    id: UUID
    name: string
    type: PipePointType
    x, y, z: double
    pipeSpecId: string
    typeParams: map<string, Variant>
    routeId: UUID              // 所属路由
    segmentId: UUID
    indexInSegment: size_t
    branchSegmentId: string   // Tee 关联的分支段
    accessories: vector<AccessoryState>  // 关联的附属构件状态
}

DeletePipePointCommand {
    pointId_: UUID

    // execute 前捕获
    savedState_: PipePointState
}

execute(ctx):
    1. 捕获 savedState_（位置、类型、参数、段索引、附属构件）
    2. 若 Tee，记录 branchSegmentId
    3. ctx.topologyManager->removePoint(route, pointId_)
    4. ctx.document.removeObject(pointId_)
    5. ctx.dependencyGraph.removeObject(pointId_)
    6. ctx.sceneRemove(pointId_.toString())

undo(ctx):
    1. 重建 PipePoint（使用 savedState_ 恢复所有属性）
    2. pp.setIdForDeserialization(savedState_.id)  ← 恢复原 UUID
    3. ctx.document.addObject(pp)
    4. 若有 pipeSpecId，重新关联 PipeSpec
    5. 恢复到 Segment 的 savedState_.indexInSegment 位置
    6. 若 Tee，重建分支段
    7. 恢复附属构件
    8. 显式注册 DependencyGraph 依赖（同 CreatePipePointCommand 步骤 7）
```

### 3.5 InsertComponentCommand — 插入组件

`MacroCommand` 的子类。**`type()` 统一返回 `CommandType::Macro`**（不覆写）。子类型信息通过 JSON 的 `"componentType"` 字段保留，反序列化时由 `CommandRegistry` 的 Macro 工厂检查此字段路由到正确子类型。

```
InsertComponentCommand : MacroCommand {
    componentType_: string   // "insert-pipe", "insert-elbow", 等

    // type() 继承自 MacroCommand，不覆写
    CommandType type() const override { return CommandType::Macro; }

    // toJson() 额外输出 componentType 字段
    nlohmann::json toJson() const override {
        auto j = MacroCommand::toJson();
        j["componentType"] = componentType_;
        return j;
    }
}

// componentType → PipePointType 映射：
// "insert-pipe"     → Run
// "insert-elbow"    → Bend
// "insert-tee"      → Tee
// "insert-reducer"  → Reducer
// "insert-valve"    → Valve
// "insert-flange"   → Run + Flange Accessory
```

**构造时组合子命令**：

| componentType | 子命令 |
|---------------|--------|
| `insert-pipe` | `CreatePipePointCommand(type=Run)` |
| `insert-elbow` | `CreatePipePointCommand(type=Bend)` |
| `insert-tee` | `CreatePipePointCommand(type=Tee)` — TopologyManager 自动创建分支段 |
| `insert-reducer` | `CreatePipePointCommand(type=Reducer)` |
| `insert-valve` | `CreatePipePointCommand(type=Valve)` |
| `insert-flange` | `CreatePipePointCommand(type=Run)` + `CreateAccessoryCommand(class=Flange)` |

**JSON 序列化**（宏命令嵌套结构）：
```json
{
    "type": "InsertComponent",
    "id": "uuid",
    "componentType": "insert-tee",
    "description": "插入三通",
    "children": [
        {"type": "CreatePipePoint", ...}
    ]
}
```

### 3.6 后续迭代命令（本次不实现）

| 命令 | 说明 |
|------|------|
| `CreateRouteCommand` | 创建空 Route + 默认空 Segment |
| `DeleteRouteCommand` | 删除 Route 及其所有 Segment 和 PipePoint |
| `CreateSegmentCommand` | 向 Route 追加新 Segment |
| `CreateLoadCommand` | 创建载荷（7 种子类型） |
| `DeleteLoadCommand` | 删除载荷，恢复完整状态 |
| `CreateLoadCaseCommand` / `DeleteLoadCaseCommand` | 基本工况 |
| `CreateLoadCombinationCommand` / `DeleteLoadCombinationCommand` | 组合工况 |
| `CreateAccessoryCommand` / `DeleteAccessoryCommand` | 附属构件 |
| `AppendPointCommand` / `InsertPointCommand` / `RemovePointCommand` | 纯拓扑操作 |

---

## 4. 执行流程

### 4.1 命令执行总流程

**修订说明**：CommandStack 不再直接调用 `markDirtyAndRecompute`。脏传播和场景更新通过信号在 `main.cpp` 中连线（见 §6.2）。

```
用户操作（UI 表格编辑 / 点击插入按钮 / Delete 键）
  │
  ▼
AppController / TableModel
  │  构造 Command 对象
  ▼
CommandStack::execute(unique_ptr<Command>)
  │
  ├── 尝试 tryMerge（连续相同属性变更合并为一条命令）
  ├── 构建 CommandContext（由调用方注入 Document, DependencyGraph 等）
  ├── cmd->execute(ctx)          // 命令执行实际变更
  ├── undoStack_.push(cmd)
  ├── redoStack_.clear()
  ├── trimToMaxSize()
  ├── emit commandCompleted(affectedIds)     // ← 信号通知
  └── emit stackChanged                         // UI 刷新 canUndo/canRedo
```

**main.cpp 信号连线**（脏传播 + 场景更新）：

```
commandStack.commandCompleted.connect([&](const auto& affectedIds) {
    for (auto& id : affectedIds) dependencyGraph.markDirty(id);
    auto dirtyIds = dependencyGraph.collectDirty();
    recomputeEngine.recompute(dirtyIds);      // → SceneUpdateCallback → SceneManager
    dependencyGraph.clearDirty();
});

commandStack.sceneRemoveRequested.connect([&](const auto& uuid) {
    sceneManager.removeNode(uuid);
});

commandStack.commandUndone.connect([&](const auto& affectedIds) {
    // 同上：markDirty → recompute → clearDirty
    // + sceneRemove（若 undo 是删除操作则移除节点）
});
```

### 4.2 Undo 流程

```
CommandStack::undo()
  │
  ├── cmd = undoStack_.pop_back()
  ├── 构建 CommandContext
  ├── cmd->undo(ctx)                // 反向恢复
  ├── redoStack_.push(cmd)
  ├── emit commandUndone(affectedIds)  // ← 信号：触发脏传播+recompute
  └── emit stackChanged
```

### 4.3 示例：用户插入三通

```
1. QML: appController.insertComponent("insert-tee")
2. AppController:
   - 确定当前路由和段（从选中对象或默认）
   - 确定插入位置坐标
   - 构建 CommandContext
   - 创建 InsertComponentCommand("insert-tee", routeId, segmentId, x, y, z)
3. CommandStack::execute(cmd)
   - cmd 是 MacroCommand，子命令为 CreatePipePointCommand(type=Tee)
4. CreatePipePointCommand::execute(ctx):
   - 创建 PipePoint(name, Tee, gp_Pnt(x,y,z))
   - Document::addObject(pp)
   - TopologyManager::appendPoint(route, segment, pp)
     → 检测到 Tee，自动创建空分支段 Branch_A06
     → Route::addSegment(branchSegment)
   - 记录 createdPointId_ 和 createdBranchId_
   - ctx.dependencyGraph.addDependency(createdPointId_, 前邻管点ID)
   - ctx.dependencyGraph.addDependency(createdPointId_, 后邻管点ID)
5. CommandStack emit commandCompleted({ppId, neighborIds})
   → main.cpp 信号处理器：
     - dependencyGraph.markDirty(ppId), markDirty(neighborIds)
     - dirtyIds = dependencyGraph.collectDirty()
     - RecomputeEngine 重算三通及其相邻管点几何
     - SceneManager 添加/更新 3D 节点
     - dependencyGraph.clearDirty()
6. undoStack_.push(cmd), redoStack_.clear()
7. emit stackChanged → QML 刷新 undo/redo 按钮状态
```

### 4.4 示例：撤销三通插入

```
1. QML: appController.undo() (Ctrl+Z)
2. CommandStack::undo()
   - 取出 InsertComponentCommand
3. 构建 CommandContext
4. MacroCommand::undo(ctx) → 逆序执行子命令的 undo
5. CreatePipePointCommand::undo(ctx):
   - 若 createdBranchId_ 非空，移除分支段
   - TopologyManager::removePoint(route, createdPointId_)
   - Document::removeObject(createdPointId_)
   - DependencyGraph::removeObject(createdPointId_)
   - ctx.sceneRemove(createdPointId_.toString())  → 触发 sceneRemoveRequested 信号
6. CommandStack emit commandUndone(affectedNeighborIds)
   → main.cpp 信号处理器：
     - dependencyGraph.markDirty(neighborIds)
     - dirtyIds = dependencyGraph.collectDirty()
     - RecomputeEngine 重算相邻管点几何
     - SceneManager 更新 3D 节点
     - dependencyGraph.clearDirty()
7. redoStack_.push(cmd)
8. emit stackChanged
```

---

## 5. 序列化格式

### 5.1 命令名称 + 参数 JSON（外部协议）

外部协议（TCP 客户端、脚本文件）使用 **命令名称 + 参数 JSON** 格式：

```json
{
    "command": "CreatePipePoint",
    "params": {
        "routeId": "uuid-string",
        "segmentId": "uuid-string",
        "name": "A07",
        "pointType": "Bend",
        "x": 5000.0, "y": 0.0, "z": 0.0,
        "pipeSpecId": "",
        "insertIndex": 3
    }
}
```

`CommandRegistry` 根据 `"command"` 字符串查找工厂 lambda，将 `"params"` 与 `"type"` 合并为完整 JSON 后调用工厂创建 `Command` 实例。

### 5.2 完整命令序列化（内部 undo 栈持久化）

undo 栈中的命令序列化包含执行后的状态（如 `createdPointId`），用于栈持久化：

```json
{
    "type": "CreatePipePoint",
    "id": "cmd-uuid",
    "params": {
        "routeId": "uuid",
        "segmentId": "uuid",
        "name": "A07",
        "pointType": "Bend",
        "x": 5000.0, "y": 0.0, "z": 0.0
    },
    "state": {
        "createdPointId": "uuid-of-created-point",
        "createdBranchId": "",
        "executed": true
    }
}
```

`state` 字段记录命令执行后的内部状态，用于 undo 栈持久化恢复。外部客户端不需要也不应该设置 `state`。

### 5.3 宏命令序列化

```json
{
    "type": "Macro",
    "id": "uuid",
    "componentType": "insert-tee",
    "description": "插入三通",
    "children": [
        {
            "type": "CreatePipePoint",
            "id": "uuid",
            "params": {
                "routeId": "uuid",
                "segmentId": "uuid",
                "name": "A06",
                "pointType": "Tee",
                "x": 3500.0, "y": 1200.0, "z": 0.0,
                "insertIndex": 5
            },
            "state": {
                "createdPointId": "uuid",
                "createdBranchId": "uuid"
            }
        }
    ]
}
```

> **修订说明**：`"type"` 统一为 `"Macro"`，`"componentType"` 作为可选字段区分 `InsertComponentCommand` 与普通 `MacroCommand`。`CommandRegistry` 的 Macro 工厂检查 `"componentType"` 是否存在来决定创建哪个子类。

### 5.4 命令序列（脚本/宏文件）

```json
{
    "version": "1.0",
    "description": "示例建模脚本",
    "commands": [
        {"command": "CreateRoute",     "params": {"name": "Route-1"}},
        {"command": "CreatePipePoint", "params": {"routeId": "...", ...}},
        {"command": "SetProperty",     "params": {"objectId": "...", "key": "x", ...}},
        {"command": "InsertComponent", "params": {"componentType": "insert-tee", ...}}
    ]
}
```

### 5.5 Variant 序列化

复用 `ProjectSerializer` 中已有的 `variantToJson()`/`jsonToVariant()` 辅助函数：

```json
{"type": "double", "value": 1000.0}
{"type": "int",    "value": 2}
{"type": "string", "value": "A106-B"}
```

### 5.6 UUID 序列化

使用 `UUID::toString()` 输出 `"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"` 格式，反序列化时使用已有的 `parseUuid()` 辅助函数。

---

## 6. 集成改造

### 6.1 Application 单例

```cpp
class Application {
public:
    // 新增
    CommandStack&          commandStack()       { return *commandStack_; }
    CommandRegistry&       commandRegistry()    { return *commandRegistry_; }
    engine::TopologyManager& topologyManager()  { return *topologyManager_; }

    // 保留（过渡期）
    TransactionManager&    transactionManager() { return *transactionManager_; }

private:
    std::unique_ptr<Document>           document_;
    std::unique_ptr<DependencyGraph>    dependencyGraph_;
    std::unique_ptr<CommandStack>       commandStack_;        // 新增
    std::unique_ptr<CommandRegistry>    commandRegistry_;     // 新增（替代 CommandSerializer 单例）
    std::unique_ptr<engine::TopologyManager> topologyManager_; // 新增
    std::unique_ptr<TransactionManager> transactionManager_;   // 保留
    std::unique_ptr<SelectionManager>   selectionManager_;
    std::unique_ptr<WorkbenchManager>   workbenchManager_;
};
```

### 6.2 main.cpp 改造

**修订说明**：CommandStack 不再持有 Document/DependencyGraph/RecomputeCallback。脏传播和场景更新通过信号连线。CommandContext 在调用点构建。

```
auto& commandStack   = appSingleton.commandStack();
auto& commandRegistry = appSingleton.commandRegistry();
auto& topologyManager = appSingleton.topologyManager();

// ---- 注册命令工厂（替代 CommandSerializer 单例内部注册）----
commandRegistry.registerFactory("SetProperty", [](const json& j){ ... });
commandRegistry.registerFactory("CreatePipePoint", [](const json& j){ ... });
commandRegistry.registerFactory("Macro", [](const json& j){ ... });  // 含 InsertComponent 子路由
// ...

// ---- 信号连线：脏传播 + 场景更新 ----

commandStack.commandCompleted.connect([&](const auto& affectedIds) {
    for (auto& id : affectedIds) dependencyGraph.markDirty(id);
    auto dirtyIds = dependencyGraph.collectDirty();
    recomputeEngine.recompute(dirtyIds);
    dependencyGraph.clearDirty();
});

commandStack.commandUndone.connect([&](const auto& affectedIds) {
    for (auto& id : affectedIds) dependencyGraph.markDirty(id);
    auto dirtyIds = dependencyGraph.collectDirty();
    recomputeEngine.recompute(dirtyIds);
    dependencyGraph.clearDirty();
});

commandStack.commandRedone.connect([&](const auto& affectedIds) {
    for (auto& id : affectedIds) dependencyGraph.markDirty(id);
    auto dirtyIds = dependencyGraph.collectDirty();
    recomputeEngine.recompute(dirtyIds);
    dependencyGraph.clearDirty();
});

commandStack.sceneRemoveRequested.connect([&](const auto& uuid) {
    sceneManager.removeNode(uuid);
});

// ---- AppController 改为接收 CommandStack ----
ui::AppController appController(document, commandStack, selectionManager);
```

### 6.3 AppController 改造

| 改造点 | 当前行为 | 改造后 |
|--------|---------|--------|
| 构造函数 | 接收 `TransactionManager&` | 接收 `CommandStack&` |
| `undo()` | `transactionManager_.undo()` | `commandStack_.undo()` |
| `redo()` | `transactionManager_.redo()` | `commandStack_.redo()` |
| `canUndo()` | `transactionManager_.canUndo()` | `commandStack_.canUndo()` |
| `insertComponent()` | `emit insertComponentRequested(type)` | 构建 CommandContext + `InsertComponentCommand` + `commandStack_.execute()` |
| `deleteSelected()` | `emit deleteRequested(uuids)` | 构建 CommandContext + `DeletePipePointCommand` + `commandStack_.execute()` |
| 栈变化通知 | 直接 emit | `commandStack_.stackChanged.connect()` → emit |
| 保存状态 | 无 | `document.save()` 后调用 `commandStack_.markClean()` |

### 6.4 PipePointTableModel 改造

```
// 当前模式（四步）：
transactionManager_.open("Edit X");
double oldX = pp->position().X();
pp->setPosition(gp_Pnt(newX, ...));
transactionManager_.recordChange(id, "x", oldX, newX);
transactionManager_.commit();

// 命令模式（一步）：
auto cmd = SetPropertyCommand::createWithOldValue(id, "x", oldX, newX);
commandStack_.execute(std::move(cmd));
```

### 6.5 PipeSpecModel 改造

同 PipePointTableModel，用 `SetPropertyCommand::createWithOldValue()` 替代 `open()/recordChange()/commit()` 模式。

### 6.6 CMakeLists.txt

在 `src/app/CMakeLists.txt` 中添加新源文件：

```cmake
add_library(app STATIC
    # ... 现有文件 ...
    Command.cpp
    CommandStack.cpp
    CommandRegistry.cpp
    PropertyApplier.cpp
    commands/PropertyCommands.cpp
    commands/PipePointCommands.cpp
    commands/ComponentCommands.cpp
)
```

---

## 7. 文件组织

**修订说明**：移除 `factories/` 子目录（统一工厂注册在 `CommandRegistry` 中）。移除 `CommandFactory.h`、`CommandRequest.h`、`CommandDispatcher`。新增 `CommandRegistry`。

```
src/app/
    Command.h                         -- Command 基类、CommandType 枚举、MacroCommand 声明
    Command.cpp                       -- MacroCommand 实现（含回滚语义）
    CommandContext.h                  -- CommandContext 结构体
    CommandStack.h / .cpp             -- 命令栈管理器（信号解耦 + openMacro + 栈限制 + clean）
    CommandRegistry.h / .cpp          -- 统一工厂注册表（替代 CommandDispatcher + CommandSerializer）
    PropertyApplier.h / .cpp          -- 属性变更薄转发层（调用 obj→setProperty/getProperty）
    commands/                         -- 具体命令子目录
        PropertyCommands.h / .cpp     -- SetPropertyCommand, BatchSetPropertyCommand
        PipePointCommands.h / .cpp    -- CreatePipePointCommand, DeletePipePointCommand
        ComponentCommands.h / .cpp    -- InsertComponentCommand
        （后续迭代）
        RouteCommands.h / .cpp        -- CreateRouteCommand, DeleteRouteCommand, CreateSegmentCommand
        LoadCommands.h / .cpp         -- Create/Delete Load, LoadCase, LoadCombination
        AccessoryCommands.h / .cpp    -- Create/Delete Accessory
        TopologyCommands.h / .cpp     -- AppendPointCommand, InsertPointCommand, RemovePointCommand
    CommandServer.h / .cpp            -- TCP 远程命令服务器（未来）
    CommandProtocol.h                 -- 请求/响应 JSON 格式辅助函数（未来）

src/model/
    DocumentObject.h                  -- 新增 virtual setProperty/getProperty
    PipePoint.h                       -- 覆写 setProperty/getProperty
    ThermalLoad.h 等                   -- 各自覆写 setProperty/getProperty
    ...

tests/
    test_commands.cpp                 -- 命令系统单元测试
```

---

## 8. 迁移策略

### Phase 1 — 并行引入（基础设施）

创建 Command 基础设施，与现有 `TransactionManager` 共存。

- 在 `DocumentObject` 基类添加 `setProperty`/`getProperty` 虚方法
- 各模型子类覆写 `setProperty`/`getProperty`
- `PropertyApplier` 改为薄转发层
- 实现 `Command`、`MacroCommand`（含回滚语义）、`CommandContext`
- 实现 `CommandStack`（含 openMacro/closeMacro、栈限制、clean 标记、信号）
- 实现 `CommandRegistry`（统一工厂）
- 实现 `SetPropertyCommand`（含 tryMerge、静态工厂、optional oldValue）
- 实现 `BatchSetPropertyCommand`
- 将 `CommandStack`、`CommandRegistry`、`TopologyManager` 加入 `Application` 单例
- **所有现有测试继续通过**

### Phase 2 — 原子迁移 UI 层

**修订说明**：所有 UI 调用方在同一阶段**同时切换**到 CommandStack，不允许半迁移状态。

- `AppController` 接收 `CommandStack&` 替代 `TransactionManager&`
- `PipePointTableModel` 和 `PipeSpecModel` **同时**改用 `SetPropertyCommand::createWithOldValue()`
- `insertComponent()` 构建 CommandContext + `InsertComponentCommand` + `commandStack_.execute()`
- `deleteSelected()` 构建 CommandContext + `DeletePipePointCommand` + `commandStack_.execute()`
- `main.cpp` 连线 CommandStack 信号到 DependencyGraph + RecomputeEngine + SceneManager
- `TransactionManager` 不再被任何 UI 代码调用
- **验证点**：所有现有测试继续通过

### Phase 3 — 结构命令 + 序列化

- 实现 `CreatePipePointCommand`、`DeletePipePointCommand`
- 实现 `InsertComponentCommand`
- 实现 `CommandSerializer`（含所有已实现命令的工厂）
- 新增 `test_commands.cpp` 测试

### Phase 4 — 清理

- 移除 `TransactionManager`（确认所有调用已迁移）
- 移除 `AppController` 中的 `insertComponentRequested`/`deleteRequested` 信号

### Phase 5 — TCP 远程命令服务（未来）

- 实现 `CommandServer`（TCP 监听 + NDJSON 帧协议）
- 实现 `CommandProtocol`（请求解析 + 响应构建）
- 扩展 `CommandResult` 返回结构化执行结果
- 实现查询命令（FindObject / ListPipePoints / GetDocument）
- 实现控制命令（undo / redo / save / load）
- 主线程命令派发队列（线程安全）
- Python 客户端示例
- 安全：默认 127.0.0.1，连接数限制

### Phase 6 — 脚本化与宏录制（未来）

- `CommandRecorder` — 捕获所有执行的命令到脚本
- `CommandPlayer` — 反序列化并重放脚本
- 暴露到 QML 或 Python 接口

---

## 9. 测试方案

### 9.1 单元测试（`tests/test_commands.cpp`）

| 测试用例 | 验证内容 |
|---------|---------|
| `SetPropertyCommand_Execute_Undo` | 修改 PipePoint X 坐标 → undo → 恢复原值 |
| `SetPropertyCommand_Redo` | undo 后 redo → 恢复新值 |
| `SetPropertyCommand_AutoCapture` | 不提供 oldValue → execute 自动从 Document 读取 → undo 正确恢复 |
| `SetPropertyCommand_TryMerge` | 连续相同 objectId+key 的命令 → tryMerge 成功 → undo 一步恢复到最早旧值 |
| `SetPropertyCommand_TryMerge_Timeout` | 间隔 > 500ms → tryMerge 返回 false |
| `SetPropertyCommand_TryMerge_DifferentKey` | 不同 key → tryMerge 返回 false |
| `BatchSetPropertyCommand` | 多个属性变更 → undo → 全部恢复 |
| `PropertyApplier_AllTypes` | 验证所有 key 在所有对象类型上的 apply/read round-trip（通过虚方法） |
| `CreatePipePointCommand` | 创建 Run 管点 → 验证 Document 包含 → undo → 验证移除 |
| `CreatePipePointCommand_Tee` | 创建 Tee → 验证分支段创建 → undo → 验证分支段也移除 |
| `CreatePipePointCommand_DepGraph` | 创建管点 → 验证 DependencyGraph 中注册了与邻居的依赖 |
| `DeletePipePointCommand` | 删除管点 → 验证移除 → undo → 验证完全恢复（含位置、参数、UUID） |
| `DeletePipePointCommand_UUIDPreserved` | 删除后 undo → 重建对象 UUID 与原 UUID 相同 |
| `MacroCommand_Order` | 三步子命令 → execute 顺序正确 → undo 逆序正确 |
| `MacroCommand_Rollback` | 子命令 2 失败 → 子命令 1 已执行 → 验证自动 undo 子命令 1 → 文档不变 |
| `CommandStack_UndoRedo` | 执行多个命令 → 连续 undo → 连续 redo → 新命令清空 redo |
| `CommandStack_Clear` | clear 后 canUndo/canRedo 均为 false |
| `CommandStack_OpenCloseMacro` | openMacro → execute 多个子命令 → closeMacro → undo 整个宏 |
| `CommandStack_AbortMacro` | openMacro → execute 子命令 → abortMacro → 验证子命令已 undo |
| `CommandStack_MaxSize` | 执行超过 maxSize 个命令 → 验证最旧命令被丢弃 |
| `CommandStack_CleanMark` | markClean → isClean() == true → execute → isClean() == false |
| `CommandRegistry_RoundTrip` | 命令序列化为 JSON → 反序列化 → execute 产生相同结果 |
| `CommandRegistry_Sequence` | 命令序列 → 序列化 → 在新 Document 上重放 → 验证最终状态 |
| `InsertComponentCommand_Types` | 每种 componentType 都能正确创建对应类型的 PipePoint |
| `InsertComponentCommand_Serialization` | toJson() 包含 componentType → 反序列化正确路由到 InsertComponentCommand |

### 9.2 集成验证

1. `pixi run build-debug` 编译通过
2. `pixi run test` 所有现有测试通过
3. UI 验证：Ctrl+Z/Y 正常工作，组件插入按钮可用，Delete 可撤销

---

## 10. TCP 远程命令服务（未来参考）

> **说明**：TCP 服务器和客户端是后续独立开发的模块。此处仅概述接口契约，确保命令管理系统设计对远程调用友好。核心分派能力由 `CommandRegistry`（§2.9）提供，TCP 服务器只需调用 `registry.createFromParams()` 即可。

### 10.1 架构概览

```
┌──────────────┐     TCP/JSON      ┌──────────────────────────────────────┐
│   外部客户端   │ ───────────────→ │       CommandServer (PipeCAD 内)       │
│  (Python/脚本/ │                   │                                      │
│   其他工具)    │ ←─────────────── │  接收 JSON → 分派 → 执行 → 返回结果     │
└──────────────┘     JSON Response  └──────────────────────────────────────┘

CommandServer 收到请求后，直接调用 commandRegistry.createFromParams(name, paramsJson) + commandStack.execute(cmd)
```

### 10.2 请求格式

所有请求遵循统一结构：**命令名称字符串 + 参数 JSON**。

```json
{"requestId":"r001","command":"CreatePipePoint","params":{"routeId":"uuid","name":"A07","pointType":"Bend","x":5000,"y":0,"z":0}}
{"requestId":"r002","command":"SetProperty","params":{"objectId":"uuid","key":"x","newValue":{"type":"double","value":6000.0}}}
{"requestId":"r003","query":"ListPipePoints","params":{"segmentId":"uuid"}}
{"requestId":"r004","control":"undo"}
```

支持批量原子操作和脚本回放：

```json
{"requestId":"r005","atomic":true,"description":"批量创建","commands":[
    {"command":"CreatePipePoint","params":{...}},
    {"command":"SetProperty","params":{...}}
]}
```

### 10.3 响应格式

```json
{"requestId":"r001","status":"ok","result":{"createdIds":["uuid"],"affectedIds":["uuid1","uuid2"],"createdObjects":[{...}]}}
{"requestId":"r001","status":"error","error":{"code":"OBJECT_NOT_FOUND","message":"Route xxx not found"}}
```

### 10.4 CommandServer 接口（未来实现）

```cpp
class CommandServer {
public:
    bool start(const std::string& host = "127.0.0.1", uint16_t port = 18080);
    void stop();
    bool isRunning() const;

private:
    Document& document_;
    CommandStack& commandStack_;
    CommandRegistry&  registry_;
    CommandStack&     commandStack_;

    void acceptLoop();
    void handleClient(int socketFd);
    nlohmann::json processRequest(const nlohmann::json& request);
    // 内部调用 registry_.createFromParams(name, paramsJson) + commandStack_.execute(cmd)
};
```

### 10.5 线程安全

OCCT 非线程安全，命令执行必须在主线程。TCP 服务器通过队列将网络线程的请求串行化到主线程：

```
网络线程: 接收请求 → 封装为 lambda → enqueue
主线程:   dequeue → registry.create() → stack.execute() → 返回 future → 网络线程取回结果并发送
```

---

## 11. 关键设计决策

| 决策 | 理由 |
|------|------|
| 命令存储 UUID 不存储指针 | 保证可序列化、可跨文档重放、生命周期安全 |
| **UUID 稳定性** | 重建对象时使用 `setIdForDeserialization()` 恢复原 UUID，确保 DependencyGraph 等外部引用不失效 |
| `CommandContext` 注入运行时依赖 | 命令类保持无状态（仅参数），便于序列化和测试 |
| **属性分派多态** | `DocumentObject::setProperty/getProperty` 虚方法替代 150 行 `dynamic_cast` 链，新类型无需修改分派器 |
| `TopologyManager` 加入 Application 单例 | 命令需要访问拓扑操作，统一管理生命周期 |
| **命令显式注册依赖** | `CreatePipePointCommand` 在 execute 中显式调用 `dependencyGraph.addDependency()`，确保新对象参与脏传播 |
| `MacroCommand` 组合模式 + **回滚语义** | 子命令执行失败时逆序 undo 已执行子命令，保证文档一致性 |
| **统一工厂注册表** | `CommandRegistry` 同时服务运行时执行和序列化，消除 `CommandFactory`/`CommandRequest`/双注册表 |
| **`type()` 不被子类覆写** | `InsertComponentCommand::type()` 返回 `Macro`，子类型通过 JSON `componentType` 字段区分，避免枚举爆炸和 LSP 违反 |
| **静态工厂 + `optional<Variant>`** | `SetPropertyCommand` 用两个工厂方法区分"已知 oldValue"和"自动捕获"，消除双布尔标志 |
| **`tryMerge()` 命令合并** | 连续相同属性变更在 500ms 内合并为一条，Ctrl+Z 一步撤销拖拽操作 |
| **`openMacro/closeMacro`** | 支持交互式多步操作增量构建宏命令，CAD 系统标准模式 |
| **信号解耦** | CommandStack 不持有 Document/DependencyGraph/RecomputeCallback，通过信号在 main.cpp 连线 |
| **栈深度限制 + clean 标记** | 可配置最大栈深度（默认 1000）防止内存无限增长；`markClean/isClean` 跟踪文档保存状态 |
| **Phase 2 原子迁移** | 所有 UI 调用方同时切换到 CommandStack，不允许半迁移状态 |
| **移除 `sceneAdd`** | 新增对象的可视化统一走 recompute 路径（有 TopoDS_Shape），仅保留 `sceneRemove` 用于立即删除 |
| `foundation::Signal<>` 通知 | 与项目现有信号机制一致，app 层不依赖 Qt |
| `CommandRegistry` 非单例 | 由 `Application` 持有，在 main.cpp 注册工厂，与 TransactionManager 管理方式一致 |
| NDJSON 帧协议 | 实现简单、可调试、管道软件命令频率不高 |
| 默认 127.0.0.1 监听 | 初期仅本机访问，不暴露到网络，安全可控 |
| 主线程串行执行命令 | OCCT 非线程安全，网络线程通过队列派发到主线程 |
| `CommandResult` 结构化返回 | TCP 客户端需要明确的执行结果，包括创建的对象 ID 和数据 |
| 查询命令不经过 CommandStack | 只读操作不走 undo 栈，直接读取 Document |

---

## 附录 A：v1.0 → v2.0 修订记录

| # | 问题 | 修订内容 | 章节 |
|---|------|---------|------|
| 1 | PropertyApplier 只搬移 dynamic_cast 链 | `DocumentObject` 添加 `setProperty/getProperty` 虚方法，PropertyApplier 降为薄转发层 | §1.3, §2.7, §2.1 |
| 2 | MacroCommand 部分执行失败无回滚 | execute 异常时逆序 undo 已执行子命令 | §2.5 |
| 3 | `sceneAdd` 与 recompute 路径重复 | 移除 `sceneAdd`，新增对象统一走 recompute | §2.3, §2.1 |
| 4 | InsertComponentCommand 覆写 `type()` 违反 LSP | `type()` 统一返回 Macro，JSON 用 `componentType` 区分 | §2.2, §3.5 |
| 5 | CommandSerializer 单例破坏测试隔离 | 改为普通类由 Application 持有 | §2.9 |
| 6 | `autoCapture_`+`captured_` 双标志脆弱 | 静态工厂方法 + `optional<Variant>` | §3.1 |
| 7 | CommandDispatcher+CommandSerializer 双工厂系统 | 统一为 `CommandRegistry`，消除 CommandRequest/CommandFactory | §2.9 |
| 8 | 缺少命令合并机制 | `Command::tryMerge()` + 500ms 时间窗口 | §2.4, §3.1 |
| 9 | 不支持交互式多步操作 | `CommandStack::openMacro/closeMacro/abortMacro` | §2.6 |
| 10 | 双栈并存 Ctrl+Z 不一致 | Phase 2 原子迁移（所有 UI 同时切换） | §8 |
| 11 | undo 栈无上限 + 无 clean 标记 | 可配置 maxStackSize + `markClean/isClean` | §2.6 |
| 12 | DependencyGraph 注册责任未明确 | 命令内部显式调用 `addDependency()` | §3.3, §3.4 |
| 13 | §10.5 CommandServer 重复定义 | 删除第二个 §10.5 | §10 |
| 14 | DeletePipePointCommand undo 需恢复原 UUID | 明确使用 `setIdForDeserialization()` | §1.3, §3.4 |
| 15 | CommandStack 耦合 RecomputeCallback | 改为信号机制，在 main.cpp 连线 | §2.6, §4, §6 |
