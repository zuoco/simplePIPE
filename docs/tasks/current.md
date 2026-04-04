# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）和 Phase 2（T30-T45）已全部完成（45/45）。
Phase 3 T0、T1、T2、T3、T4、T5 已完成。

**T5 完成内容**：
- `src/command/CommandRegistry.h` — 统一命令工厂注册表接口
- `src/command/CommandRegistry.cpp` — 实现：registerFactory / createFromParams / createFromFullJson / serialize / deserialize / serializeSequence / deserializeSequence / registeredCommands / hasCommand / registerBuiltins
- `src/command/CMakeLists.txt` — 追加 `CommandRegistry.cpp` 到 command 静态库
- `tests/test_command_registry.cpp` — 21 个测试全部通过
- `tests/CMakeLists.txt` — 追加 `test_command_registry` 目标（链接 command + app）

**关键上下文**：
- `Factory = std::function<unique_ptr<Command>(const nlohmann::json&)>` — 统一接收完整 JSON（含 "type" 字段）
- `createFromParams(name, params)` 将 params 平铺并注入 `"type"` 字段后分派工厂
- Macro 工厂用 `[this]` 捕获以递归调用 `deserialize()` 反序列化子命令
- Variant JSON 辅助函数和 UUID 解析在 CommandRegistry.cpp 匿名 namespace 中复制，避免跨层依赖
- `registerBuiltins()` 注册 SetProperty、BatchSetProperty、Macro 三种内置命令；重复注册安全

**T6 需要做的**：
- `Application` 单例新增 `CommandStack`、`CommandRegistry`、`TopologyManager` 成员
- `Application::createCommandContext()` 便利方法
- `main.cpp` 连线 CommandStack 六个信号（commandCompleted/commandUndone/commandRedone 做脏传播 + recompute；sceneRemoveRequested 做场景移除）
- `commandRegistry.registerBuiltins()` 在 main.cpp 初始化时调用
- `AppController` 构造函数改为接收 `CommandStack&` 替代 `TransactionManager&`（undo/redo/canUndo 迁移）
- 同步迁移 `PipePointTableModel`、`PipeSpecModel` 使用 `SetPropertyCommand::createWithOldValue()`
- `tests/test_app_core.cpp` / `test_workbench_bridge.cpp` 等保持全部通过

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T6 |
| **任务名** | Application 集成 + main.cpp 信号连线 |
| **推荐模型** | Sonnet 4.6 |
| **前置依赖** | T0-T5（均已完成） |

### 具体工作

1. **`src/app/Application.h` / `Application.cpp`**（修改现有文件）：
   - 新增 `#include "command/CommandStack.h"` / `CommandRegistry.h`
   - 新增私有成员：`unique_ptr<CommandStack> commandStack_`、`unique_ptr<CommandRegistry> commandRegistry_`、`unique_ptr<engine::TopologyManager> topologyManager_`
   - 新增公有访问方法：`commandStack()` / `commandRegistry()` / `topologyManager()`
   - 新增 `CommandContext createCommandContext()` 便利方法

2. **`src/main.cpp`**（修改现有文件）：
   - 从 `appSingleton` 获取 `commandStack` 和 `commandRegistry`
   - 调用 `commandRegistry.registerBuiltins()`
   - 连线 `commandStack.commandCompleted`、`commandUndone`、`commandRedone` → markDirty + recompute
   - 连线 `commandStack.sceneRemoveRequested` → `sceneManager.removeNode`
   - 将 `AppController` 构造改为接收 `commandStack`（替代 `transactionManager`）

3. **`src/ui/AppController.h` / `AppController.cpp`**（修改现有文件）：
   - 构造函数参数改为 `CommandStack&` 替代 `TransactionManager&`
   - `undo()` → `auto ctx = Application::instance().createCommandContext(); commandStack_.undo(ctx)`
   - `redo()` → `commandStack_.redo(ctx)`
   - `canUndo()` → `commandStack_.canUndo()`
   - `canRedo()` → `commandStack_.canRedo()`
   - 栈变化通知：连线 `commandStack_.stackChanged` → emit `undoRedoStateChanged` 等信号

4. **`src/ui/PipePointTableModel.cpp`** 和 **`src/ui/PipeSpecModel.cpp`**（同步迁移）：
   - 用 `SetPropertyCommand::createWithOldValue(objectId, key, oldVal, newVal)` + `commandStack_.execute(cmd, ctx)` 替代 `open()/recordChange()/commit()`

5. **`tests/test_app_core.cpp`**：验证 Application 单例新增的三个成员可正常访问

6. **`tests/CMakeLists.txt`** 若需要新增测试目标则添加

### 验收标准

- `pixi run test` 全部通过（保持 T4 + T5 的 21+24+29 = 74 条测试）
- Application::instance().commandStack() 和 commandRegistry() 可正常访问
- AppController undo/redo 通过 CommandStack 执行，TransactionManager 不再被 AppController 调用

## 需要读取的文件

1. `docs/command-pattern-design.md` §6.1–6.4（Application/main.cpp/AppController/TableModel 改造规格）
2. `src/app/Application.h` — 现有单例结构（已读）
3. `src/main.cpp` — 现有信号连线方式（已读）
4. `src/ui/AppController.h` — 现有构造函数签名
5. `src/ui/PipePointTableModel.h` / `PipePointTableModel.cpp` — 现有属性修改路径
6. `src/command/CommandStack.h` — CommandStack 六个信号 + execute/undo/redo 签名
7. `src/command/CommandRegistry.h` — registerBuiltins() 签名
8. `src/command/CommandContext.h` — CommandContext 结构体字段

- `CommandStack::execute` tryMerge 合并成功后需执行新命令（cmd2->execute）以将新值写入文档；原实现只更新参数而不更新文档，导致文档值为旧值

**关键上下文**：
- `command` 库的 cpp 文件包含 `app/Document.h`（通过 model 传递的 OCCT includes），但不在 cmake 链接 `app`，防止循环依赖；符号由测试目标链接时的 `app` 提供
- `SetPropertyCommand::tryMerge`：只更新 top 命令的 `newValue_`，文档更新由 CommandStack 执行新命令完成
- `BatchSetPropertyCommand` 失败时逆序回滚，保证文档一致性
- T5 将实现 `CommandRegistry`（统一工厂注册表，替代 CommandDispatcher + CommandSerializer）

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T5 |
| **任务名** | CommandRegistry 统一工厂 + 序列化 |
| **推荐模型** | Sonnet 4.6 |
| **前置依赖** | T0-T4（均已完成） |

### 具体工作

1. **`src/command/CommandRegistry.h` / `CommandRegistry.cpp`**（加入 `command` 层）：
   - `registerFactory(typeKey, json→unique_ptr<Command> lambda)` — 注册命令工厂
   - `createFromParams(commandName, params json)` — 外部协议创建（TCP 客户端/脚本）
   - `createFromFullJson(j)` — 从完整命令 JSON 创建（反序列化）
   - `serialize(cmd)` / `deserialize(j)` — 静态序列化 / 实例反序列化
   - `serializeSequence` / `deserializeSequence` — 命令序列（宏文件）
   - `registeredCommands()` / `hasCommand(name)` — 查询接口

2. **注册内容**（在 `CommandRegistry` 构造或独立 `registerBuiltins()` 方法中）：
   - "SetProperty" → `SetPropertyCommand::createWithOldValue` 或 `createAutoCapture`
   - "BatchSetProperty" → `BatchSetPropertyCommand`
   - "Macro" → `MacroCommand`（含 componentType subtype routing）

3. **`tests/test_command_registry.cpp`**：
   - registerFactory + createFromParams 基础流程
   - createFromFullJson 含 SetProperty 反序列化 round-trip
   - serialize + deserialize round-trip（SetProperty）
   - serializeSequence / deserializeSequence
   - hasCommand / registeredCommands
   - 未知命令名抛异常

4. **`tests/CMakeLists.txt`**：添加 `test_command_registry` 目标，链接 `command` + `app`

### 验收标准

- `pixi run test` 全部通过（在 T4 的 24+29+29=82 基础上新增 CommandRegistry 测试）
- SetPropertyCommand round-trip（serialize → deserialize → execute → 结果与原始一致）
- 未注册命令名 createFromParams 抛 `std::out_of_range`

## 需要读取的文件

1. `docs/command-pattern-design.md` §2.9 — CommandRegistry 规格
2. `src/command/SetPropertyCommand.h` — T4 产出（含工厂方法签名）
3. `src/command/BatchSetPropertyCommand.h` — T4 产出
4. `src/command/CommandStack.h` — 命令栈（测试中用于集成验证）
5. `src/command/Command.h` — 基类接口
6. `src/app/Document.h` — findObject（测试用）


**T3 完成内容**：
- `src/command/CommandStack.h` — 声明 CommandStack 类（六种信号 + 全部公有 API）
- `src/command/CommandStack.cpp` — 实现 execute/undo/redo/openMacro/closeMacro/abortMacro/markClean/isClean/setMaxSize 等
- `src/command/CMakeLists.txt` — 追加 `CommandStack.cpp` 到 command 静态库
- `tests/test_command_stack.cpp` — 29 个测试全部通过
- `tests/CMakeLists.txt` — 追加 `test_command_stack` 测试目标

**关键上下文**：
- CommandStack 不持有 Document/DependencyGraph，通过六种 `foundation::Signal` 解耦
- 宏模式：`openMacro` → 子命令 `execute` 即时执行 + 加入 `pendingMacro_` → `closeMacro` 整体推入 undoStack_（不重复执行）
- `tryMerge` 路径：顶部命令合并成功后，undoStack_ 仍只有 1 条，emit `commandCompleted/stackChanged`
- `isClean()` 规则：undoStack_ 空时检查 cleanTopId_.isNull()；非空时比较 back()->id() == cleanTopId_
- `trimToMaxSize()` 从 undoStack_ 的 front 删除（最旧命令先删）
- T4 将实现 `SetPropertyCommand` 和 `BatchSetPropertyCommand`（`tryMerge` 的第一个具体实现）

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T4 |
| **任务名** | PropertyCommands (SetProperty/BatchSetProperty) |
| **推荐模型** | Sonnet 4.6 |
| **前置依赖** | T0、T1、T2、T3（均已完成） |

### 具体工作

1. **`src/command/SetPropertyCommand.h` / `SetPropertyCommand.cpp`**（加入 `command` 层）：
   - 构造函数：objectId + key + newValue，执行时读 oldValue
   - execute：`PropertyApplier::read` 捕获 oldValue → `PropertyApplier::apply` 写 newValue → 填写 affectedIds
   - undo：`PropertyApplier::apply` 恢复 oldValue → 填写 affectedIds
   - `tryMerge`：objectId + key 相同且时间差 < 500ms → 更新 newValue_，返回 true

2. **`src/command/BatchSetPropertyCommand.h` / `BatchSetPropertyCommand.cpp`**：
   - 构造函数：vector<{objectId, key, newValue}> → 批量执行 SetProperty
   - execute：按序读取各对象旧值，依次 apply — 失败则回滚已 apply 的修改
   - undo：逆序恢复各对象旧值
   - 不实现 tryMerge（批量命令不参与合并）

3. **`tests/test_property_commands.cpp`**：
   - SetPropertyCommand execute + undo（需真实 Document 对象）
   - tryMerge：同 key 同对象时间窗内合并；不同 key/不同对象/超时不合并
   - BatchSetPropertyCommand execute（含失败回滚） + undo

4. **`tests/CMakeLists.txt`**：添加 `test_property_commands` 目标，链接 `command` + `app`

### 验收标准

- `pixi run test` 全部通过（在 T3 的 29+29=58 基础上新增 PropertyCommands 测试）
- SetPropertyCommand 的 `tryMerge` 在相同 objectId + key 且时间差 < 500ms 时正确合并
- BatchSetPropertyCommand 失败时回滚已 apply 的修改，保证文档一致性

## 需要读取的文件

1. `docs/command-pattern-design.md` §3 — SetPropertyCommand / BatchSetPropertyCommand 规格
2. `src/command/CommandStack.h` — T3 产出（已完成）
3. `src/command/PropertyApplier.h` — T2 产出（PropertyApplier::apply/read）
4. `src/command/Command.h` — Command 基类（tryMerge 接口）
5. `src/model/DocumentObject.h` — setProperty/getProperty 虚方法（T1 产出）
6. `src/model/PipePoint.h` — 具体模型实现（测试用）
7. `src/app/Document.h` — findObject() 方法（SetPropertyCommand 通过 ctx.document 查找对象）
8. `src/command/CommandResult.h` — affectedIds 填写规范

