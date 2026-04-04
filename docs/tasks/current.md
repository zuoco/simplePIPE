# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）和 Phase 2（T30-T45）已全部完成（45/45）。
Phase 3 T0、T1、T2 已完成。

**T2 完成内容**：
- 新建 `src/command/` 层（位于 `model` 与 `engine` 之间）
- `CommandResult` — success/errorMessage/createdIds/deletedIds/affectedIds + toJson()
- `CommandType` — 14 个枚举值（SetProperty…Macro）
- `CommandContext` — 前向声明 + 指针（app::Document*、app::DependencyGraph*、engine::TopologyManager*），无循环依赖
- `Command` — 抽象基类，含 execute/undo/description/type/toJson 纯虚方法 + tryMerge/id/lastResult/timestamp
- `MacroCommand` — 回滚语义：顺序执行，失败时逆序 undo 已执行的子命令
- `PropertyApplier` — header-only 薄转发层，apply() 对未知 key 抛 std::invalid_argument
- `engine` 的 `target_link_libraries` 加入 `command`
- 29 个测试全部通过

**关键上下文**：
- `CommandContext` 使用指针（不是引用），可以全 nullptr 构造
- `Command` 抽象类的 protected 成员：`id_`（UUID）、`lastResult_`（CommandResult）、`timestamp_`（steady_clock）
- `MacroCommand` 储存 `std::vector<std::unique_ptr<Command>> children_`
- T3 将实现 `CommandStack`（undo/redo 栈管理器），加入 `command` 层（`MacroCommand.cpp` 已在该 library）

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T3 |
| **任务名** | CommandStack 命令栈管理器 |
| **推荐模型** | Opus 4.6 |
| **前置依赖** | T0、T1、T2（均已完成） |

### 具体工作

1. **`src/command/CommandStack.h` / `CommandStack.cpp`**（加入 `command` 层）：
   - `execute(unique_ptr<Command>, CommandContext&)` → tryMerge → cmd→execute → 入栈 → emit 信号
   - `undo(CommandContext&)` / `redo(CommandContext&)` → 从栈取出→执行→emit 信号
   - `openMacro/closeMacro/abortMacro(CommandContext&)` → 交互式宏
   - `markClean/isClean` → 文档保存状态跟踪
   - `setMaxSize/trimToMaxSize` → 栈深度限制（默认 1000）
   - 信号：`stackChanged`、`commandCompleted`、`commandUndone`、`commandRedone`、`cleanStateChanged`、`sceneRemoveRequested`

2. **`tests/test_command_stack.cpp`**：
   - execute + canUndo/canRedo
   - undo/redo 循环
   - openMacro/closeMacro 整体入栈（子命令不重复执行）
   - abortMacro 逆序回滚
   - markClean/isClean 边界行为
   - setMaxSize 截断最旧命令
   - sceneRemoveRequested 信号触发
   - tryMerge 合并（使用可合并的 Stub）

3. **`tests/CMakeLists.txt`**：添加 `test_command_stack` 目标，链接 `command`

### 验收标准

- `pixi run test` 全部通过（在 T2 的 29 个基础上新增 CommandStack 测试）
- `openMacro + execute(子命令) + closeMacro` 整体推入 undoStack，子命令不重复执行
- `abortMacro` 对 pendingMacro 中已执行子命令逆序 undo
- `isClean` 在 markClean 后的 undo/新命令场景下行为正确

## 需要读取的文件

1. `docs/command-pattern-design.md` §2.6 — CommandStack 完整规格（含行为表）
2. `src/command/Command.h` — Command 基类（T2 产出）
3. `src/command/MacroCommand.h` — MacroCommand（T2 产出）
4. `src/command/CommandContext.h` — CommandContext（T2 产出）
5. `src/command/CommandResult.h` — CommandResult（T2 产出）
6. `src/foundation/Signal.h` — Signal<T> 模板定义（CommandStack 信号类型）
7. `src/command/CMakeLists.txt` — 追加 CommandStack.cpp 到 library 的格式参考
8. `tests/CMakeLists.txt` 末尾 — 追加新测试格式参考

