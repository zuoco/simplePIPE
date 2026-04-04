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