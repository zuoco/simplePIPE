# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）和 Phase 2（T30-T45）已全部完成（45/45）。
Phase 3 命令模式 T0、T1、T2、T3、T4、T5、T6 已完成。

**T6 完成内容**：
- `src/app/Application.h/.cpp`：新增 `CommandStack`、`CommandRegistry`、`TopologyManager` 成员与访问器
- `Application::createCommandContext()`：统一构建 `CommandContext`
- `src/main.cpp`：调用 `commandRegistry.registerBuiltins()`，并完成 `CommandStack` 四类信号连线（完成/撤销/重做/场景删除）
- `src/ui/AppController.h/.cpp`：构造函数改为接收 `CommandStack&`；undo/redo/canUndo/canRedo 全迁移到命令栈
- `src/ui/PipePointTableModel.*`、`src/ui/PipeSpecModel.*`：属性编辑改为 `SetPropertyCommand::createWithOldValue()` + `commandStack.execute()`
- `tests/test_app_core.cpp`、`tests/test_workbench_bridge.cpp`、`tests/test_qml_models.cpp`、`tests/test_qml_ui_panels.cpp`：按新接口与行为同步调整
- 全量验证：`pixi run test` 通过，**39/39** 测试通过

**关键上下文**：
- UI 层（`AppController`、`PipePointTableModel`、`PipeSpecModel`）已不再依赖 `TransactionManager`
- 命令执行后的重算路径已统一为：`CommandStack` 信号 -> `DependencyGraph` 脏标记 -> `RecomputeEngine`
- 删除对象的场景同步由 `CommandStack::sceneRemoveRequested` 统一触发

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T7 |
| **任务名** | UI 原子迁移 (AppController/TableModel) |
| **推荐模型** | Opus 4.6 |
| **前置依赖** | T4, T6（均已完成） |

### 具体工作

1. 对照 `docs/command-pattern-design.md` 的 Phase 2 要求，逐项复核 UI 层迁移是否完整（避免半迁移状态）。
2. 重点确认 `AppController`、`PipePointTableModel`、`PipeSpecModel` 的所有写操作都通过 `CommandStack + CommandContext` 路径执行。
3. 确认 UI 侧不再调用 `TransactionManager`（包括直接调用和间接封装调用）。
4. 补充或收敛测试断言，确保 undo/redo、栈状态信号、表格编辑行为稳定。
5. 运行全量测试并完成 T7 状态更新与日志追加。

## 需要读取的文件

1. `docs/tasks/status.md`（确认 T7 状态与依赖）
2. `docs/command-pattern-design.md` §8（Phase 2 迁移要求）
3. `src/ui/AppController.h`
4. `src/ui/AppController.cpp`
5. `src/ui/PipePointTableModel.h`
6. `src/ui/PipePointTableModel.cpp`
7. `src/ui/PipeSpecModel.h`
8. `src/ui/PipeSpecModel.cpp`
9. `tests/test_app_core.cpp`
10. `tests/test_workbench_bridge.cpp`
11. `tests/test_qml_models.cpp`
12. `tests/test_qml_ui_panels.cpp`
13. `src/main.cpp`
14. `src/app/Application.h`
