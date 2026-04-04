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
