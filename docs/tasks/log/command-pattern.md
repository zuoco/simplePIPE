# Phase 3 完成记录 — 命令模式 (T0–T10)

> **设计文档**: `docs/command-pattern-design.md` (v3.0)
> **任务状态**: `docs/tasks/status.md`

---

<!-- 任务完成后在此追加记录 -->

### T0 — Variant 类型扩展 (bool/Vec3) (2026-04-04)

**产出文件**: `src/foundation/Types.h` · `src/app/ProjectSerializer.cpp` · `tests/test_foundation.cpp` · `tests/test_project_serializer.cpp`

**接口**: → `src/foundation/Types.h`

**设计决策**:
- `Variant` 从 `std::variant<double, int, std::string>` 扩展为 `std::variant<double, int, std::string, bool, math::Vec3>`
- 在 `Types.h` 中直接 `#include "foundation/Math.h"`（无循环依赖，Math.h 仅依赖标准库）
- 添加 `variantToBool()` 和 `variantToVec3()` 辅助函数，风格与已有函数一致
- `variantToJson()`：bool → `{"type":"bool","value":true}`；Vec3 → `{"type":"vec3","x":...,"y":...,"z":...}`
- round-trip 测试写入 `test_project_serializer.cpp`（已有 PipePoint.h 依赖，比 test_load_serialization 更合适）

**已知限制**:
- 无

---

### T1 — DocumentObject setProperty/getProperty 虚方法 (2026-04-04)

**产出文件**: `src/model/DocumentObject.h` · `src/model/SpatialObject.h` · `src/model/PropertyObject.h` · `src/model/PipePoint.h` · `src/model/ThermalLoad.h` · `src/model/PressureLoad.h` · `src/model/WindLoad.h` · `src/model/SeismicLoad.h` · `src/model/DisplacementLoad.h` · `src/model/UserDefinedLoad.h` · `tests/test_model.cpp`

**接口**: → `src/model/DocumentObject.h`（虚方法是 T2 PropertyApplier 的基础）

**设计决策**:
- `DocumentObject` 新增两个非纯虚方法：`setProperty` 返回 false（未识别 key）；`getProperty` 抛 `std::out_of_range`
- `DocumentObject` 基类只处理 `"name"` key（调用 `setName()`）
- `SpatialObject` 处理 `"x"/"y"/"z"`（重构 `gp_Pnt`，调用 `setPosition()`），其余代理到基类
- `PropertyObject` 将所有非 `"name"` key 存入 `fields_`（Accept-all 策略，适合 PipeSpec/ProjectConfig 的扩展字段语义）；`"name"` 代理到基类
- `PipePoint` 处理 `"type"`（转 int），`"x"/"y"/"z"/"name"` 代理到 SpatialObject，其余 key 存入 `typeParams_`
- 载荷子类（ThermalLoad/PressureLoad/WindLoad/SeismicLoad/DisplacementLoad/UserDefinedLoad）各自处理特有属性，其余代理到 `DocumentObject` 基类
- `PipeSpec`、`ProjectConfig` 无需额外覆写，通过 `PropertyObject` 已自动支持
- `<stdexcept>` 已加入 `DocumentObject.h`（供 `std::out_of_range` 抛出使用）

**已知限制**:
- 无

---

### T2 — Command 基类 + MacroCommand + PropertyApplier (2026-04-04)

**产出文件**: `src/command/CommandResult.h` · `src/command/CommandType.h` · `src/command/CommandContext.h` · `src/command/Command.h` · `src/command/MacroCommand.h` · `src/command/MacroCommand.cpp` · `src/command/PropertyApplier.h` · `src/command/CMakeLists.txt` · `tests/test_command_base.cpp`

**接口**: → `src/command/Command.h`、`src/command/CommandContext.h`、`src/command/PropertyApplier.h`

**设计决策**:
- 新建 `src/command/` 层，位于 `model` 与 `engine` 之间；`engine` 的 `target_link_libraries` 加入 `command`，确保上层可传递使用
- `CommandContext` 使用前向声明 + 指针（`app::Document*`、`app::DependencyGraph*`、`engine::TopologyManager*`），避免与 `app` 层循环依赖；空指针对于不需要 Document 的命令（如 MacroCommand 自身）安全
- `Command` 抽象基类：`execute/undo/description/type/toJson` 纯虚；`tryMerge` 默认返回 false；成员 `id_`、`lastResult_`、`timestamp_` 在 protected 区
- `MacroCommand::execute()` 实现回滚语义：顺序执行，任一子命令抛异常则逆序 undo 已执行的子命令，回滚中的 undo 失败只打印 stderr 警告，最后重新抛出原始异常
- `MacroCommand` 累积所有子命令的 `createdIds/deletedIds/affectedIds` 到自身 `lastResult_`
- `PropertyApplier` 完全 header-only，`apply()` 对 `setProperty` 返回 false 的 key 抛 `std::invalid_argument`；`read()` 对 null 对象抛 `std::invalid_argument`，对未知 key 传播 `getProperty` 的 `std::out_of_range`
- 测试：29 个测试全部通过（CommandResult 2、CommandBase 8、MacroCommand 9、PropertyApplier 9、CommandType 1）

**已知限制**:
- 无

---

### T3 — CommandStack 命令栈管理器 (2026-04-04)

**产出文件**: `src/command/CommandStack.h` · `src/command/CommandStack.cpp` · `tests/test_command_stack.cpp`

**接口**: → `src/command/CommandStack.h`

**设计决策**:
- 不持有 `Document&` / `DependencyGraph&`；通过 `foundation::Signal` 解耦（六种信号：stackChanged / commandCompleted / commandUndone / commandRedone / cleanStateChanged / sceneRemoveRequested）
- `execute()` 分两路：若 `macroOpen_`，子命令先执行再放入 `pendingMacro_`，不推入 undoStack_；否则尝试 `tryMerge` → 执行 → 入栈 → 清空 redo → trim → emit 信号
- `tryMerge` 合并路径：顶部命令合并成功后不 push 新命令，emit `commandCompleted/stackChanged`，undoStack_ 仍只有 1 条
- `closeMacro()`：直接将 `pendingMacro_` 推入 undoStack_（子命令已在 execute 阶段执行，不重新执行），收集所有子命令 affectedIds emit `commandCompleted`
- `abortMacro()`：对 `pendingMacro_.children()` 逆序调用 `undo()`，单个 undo 失败仅捕获忽略，整体丢弃宏
- `markClean()`：undoStack_ 空时 cleanTopId_ 置零（isNull）；非空时记录 back()->id()；仅在状态变化时 emit `cleanStateChanged`
- `isClean()`：undoStack_ 空时返回 `cleanTopId_.isNull()`；非空时比较 back()->id() == cleanTopId_
- `setMaxSize()` 调用时立即截断；`trimToMaxSize()` 从 undoStack_ front 删除最旧命令
- `emitSceneRemove()` 将 `lastResult_.deletedIds` 转为 UUID 字符串逐一 emit `sceneRemoveRequested`
- 测试：29 个全部通过（InitialState、Execute×3、UndoRedo×3、Macro×4、abortMacro×2、markClean×6、maxSize×2、sceneRemove×2、tryMerge×2、信号×2、clear×1）

**已知限制**:
- 无

---

### T4 — PropertyCommands (SetProperty/BatchSetProperty) (2026-04-04)

**产出文件**: `src/command/SetPropertyCommand.h` · `src/command/SetPropertyCommand.cpp` · `src/command/BatchSetPropertyCommand.h` · `src/command/BatchSetPropertyCommand.cpp` · `tests/test_property_commands.cpp`

**接口**: → `src/command/SetPropertyCommand.h`、`src/command/BatchSetPropertyCommand.h`

**设计决策**:
- `SetPropertyCommand` 用两个静态工厂方法替代构造函数：`createAutoCapture`（oldValue 由 execute 时从对象读取）、`createWithOldValue`（调用方显式提供，反序列化/已知旧值场景）
- 内部用 `std::optional<Variant>` 判断是否已捕获 oldValue；`toJson()` 在 oldValue 为空时抛 `std::logic_error`
- `tryMerge` 时间窗 500ms，同 objectId + key 则合并：将最新的 newValue_ 更新到 top 命令，返回 true
- **CommandStack tryMerge 修复**：合并成功后必须执行新命令（cmd2->execute）以将新值应用到文档；旧实现只更新参数不应用文档变更，导致合并后文档值仍为旧值
- `BatchSetPropertyCommand::execute` 实现原子回滚：顺序 apply 变更，任何步骤失败则逆序恢复已 apply 的修改，保证文档一致性
- `BatchSetPropertyCommand::undo` 逆序恢复各对象旧值；对象不存在时静默跳过
- JSON 序列化：Variant 使用 `{"type":"double","value":1000.0}` 格式（与 ProjectSerializer 一致，在匿名 namespace 中复制函数避免跨层包含）
- `command` 库 `.cpp` 文件通过 `${CMAKE_SOURCE_DIR}/src` 包含 `app/Document.h`（利用 model 传递的 OCCT includes），但不将 `app` 加入 `target_link_libraries(command)`，符号由测试/可执行文件链接时的 `app` 库提供
- 测试：24 个测试全部通过（SetProperty execute/undo/redo/autoCapture/withOldValue/notFound/string、tryMerge merge/noMerge/preserveOldValue、toJson×3、Stack merge、Batch execute/undo/rollback/desc/type/json/empty）

**已知限制**:
- 无

---

### T5 — CommandRegistry 统一工厂 + 序列化 (2026-04-04)

**产出文件**: `src/command/CommandRegistry.h` · `src/command/CommandRegistry.cpp` · `tests/test_command_registry.cpp`

**接口**: → `src/command/CommandRegistry.h`

**设计决策**:
- `Factory = std::function<unique_ptr<Command>(const nlohmann::json&)>` — 工厂 lambda 接收完整 JSON（含 "type" 字段）
- `createFromParams(name, params)`：将 params 字段平铺后注入 `"type"` 字段，再分派给工厂 lambda；外部协议和脚本分派共用同一入口
- `createFromFullJson(j)` / `deserialize(j)`：等价，从含 "type" 字段的完整 JSON 创建命令
- `serialize(cmd)`：静态方法，直接委托 `cmd.toJson()`；`serializeSequence` 构建 JSON 数组
- Macro 工厂用 `[this]` 捕获，以便递归调用 `deserialize()` 反序列化子命令
- Variant JSON 辅助函数（variantToJson/jsonToVariant）和 UUID 解析在匿名 namespace 中复制，避免跨层依赖
- `registerBuiltins()` 注册三种内置命令（SetProperty、BatchSetProperty、Macro）；重复调用安全（后注册覆盖同 key）
- 未知命令名 `createFromParams`/`createFromFullJson` 均抛 `std::out_of_range`
- 测试：21 个测试全部通过（hasCommand、registeredCommands、registerFactory、createFromParams×4、serialize×2、createFromFullJson、deserialize、round-trip×2、serializeSequence×3、Macro 子命令反序列化×1、边界条件×4）

**已知限制**:
- 无

---

### T6 — Application 集成 + main.cpp 信号连线 (2026-04-04)

**产出文件**: `src/app/Application.h` · `src/app/Application.cpp` · `src/main.cpp` · `src/ui/AppController.h` · `src/ui/AppController.cpp` · `src/ui/PipePointTableModel.h` · `src/ui/PipePointTableModel.cpp` · `src/ui/PipeSpecModel.h` · `src/ui/PipeSpecModel.cpp` · `src/app/CMakeLists.txt` · `tests/CMakeLists.txt` · `tests/test_app_core.cpp` · `tests/test_workbench_bridge.cpp` · `tests/test_qml_models.cpp` · `tests/test_qml_ui_panels.cpp`

**接口**: → `src/app/Application.h`, `src/ui/AppController.h`, `src/ui/PipePointTableModel.h`, `src/ui/PipeSpecModel.h`

**设计决策**:
- `Application` 单例新增 `CommandStack`、`CommandRegistry`、`TopologyManager` 成员和访问器，并提供 `createCommandContext()` 统一构建上下文
- `main.cpp` 初始化阶段调用 `commandRegistry.registerBuiltins()`；将 `commandCompleted/commandUndone/commandRedone` 连线到脏标记+重算，将 `sceneRemoveRequested` 连线到 `sceneManager.removeNode`
- `AppController` 构造签名迁移为 `CommandStack&`，`undo/redo/canUndo/canRedo` 全部改走命令栈，并通过 `stackChanged` 驱动 `transactionStateChanged`
- `PipePointTableModel` 与 `PipeSpecModel` 同步切换到 `SetPropertyCommand::createWithOldValue(...) + commandStack.execute(cmd, ctx)`，移除 UI 层对事务记录 API 的依赖
- 增加/调整测试覆盖 Application 新成员访问与 UI 构造签名变化，`pixi run test` 结果为 39/39 通过

**已知限制**:
- 无