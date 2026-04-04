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

