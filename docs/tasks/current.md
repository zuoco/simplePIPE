# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）和 Phase 2（T30-T45）已全部完成（45/45）。
Phase 3 命令模式 T0–T7 已完成。

**T7 完成内容**：
- 逐项复核 T6 完成的 UI 层迁移，确认 `AppController`、`PipePointTableModel`、`PipeSpecModel` 所有写操作全部通过 `CommandStack + SetPropertyCommand` 路径
- 确认 `src/ui/` 中无任何 `TransactionManager` 引用残留
- 新增 12 项 undo/redo 专项测试（8 项 TableModel + 4 项 AppController）
- 全量验证：`pixi run test` — **39/39** 测试通过

**关键上下文**：
- UI 层写操作完全通过 CommandStack，Phase 2 (原子迁移) 验证完成
- 命令执行后的重算路径：`CommandStack` 信号 → `DependencyGraph` 脏标记 → `RecomputeEngine`
- 删除对象的场景同步由 `CommandStack::sceneRemoveRequested` 统一触发
- `TopologyManager` 已在 Application 单例中初始化，可通过 `createCommandContext()` 获取

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T8 |
| **任务名** | 结构命令 (CreatePipePoint/DeletePipePoint) |
| **推荐模型** | Opus 4.6 |
| **前置依赖** | T3, T5（均已完成） |

### 具体工作

1. 实现 `CreatePipePointCommand`：创建管点 + 关联 PipeSpec + 加入 Segment + 注册 DependencyGraph 依赖
2. 实现 `DeletePipePointCommand`：捕获完整 PipePointState → 删除 → undo 时用 `setIdForDeserialization()` 恢复原 UUID
3. 两个命令均需支持 JSON 序列化/反序列化，注册到 `CommandRegistry`
4. Tee 类型管点需处理分支段的创建和恢复
5. Delete 需捕获附属构件状态用于 undo 恢复
6. 编写完整单元测试覆盖 execute/undo/redo、Tee 分支、UUID 稳定性、DependencyGraph 注册、JSON round-trip

### 设计参考

- `docs/command-pattern-design.md` §3.3（CreatePipePointCommand）和 §3.4（DeletePipePointCommand）
- `PipePointState` 快照结构需包含：id、name、type、xyz、pipeSpecId、typeParams、routeId、segmentId、indexInSegment、branchSegmentId、accessories

## 需要读取的文件

1. `docs/tasks/status.md`（确认 T8 状态与依赖）
2. `docs/command-pattern-design.md` §3.3–§3.4（CreatePipePoint/DeletePipePoint 设计规格）
3. `src/command/Command.h`（基类接口）
4. `src/command/CommandContext.h`（上下文结构）
5. `src/command/CommandStack.h`（命令栈信号）
6. `src/command/CommandRegistry.h`（工厂注册）
7. `src/command/SetPropertyCommand.h`（参考属性命令实现模式）
8. `src/command/MacroCommand.h`（宏命令基类）
9. `src/model/PipePoint.h`（管点模型，setIdForDeserialization）
10. `src/model/Segment.h`（段操作：addPoint/removePoint/insertPoint）
11. `src/model/Route.h`（路由操作）
12. `src/engine/TopologyManager.h`（appendPoint/removePoint 拓扑操作）
13. `src/app/DependencyGraph.h`（addDependency/removeObject）
14. `src/app/Document.h`（addObject/removeObject/findObject）
15. `src/app/Application.h`（createCommandContext）
