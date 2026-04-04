# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）和 Phase 2（T30-T45）已全部完成（45/45）。
Phase 3 T0、T1、T2、T3 已完成。

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

