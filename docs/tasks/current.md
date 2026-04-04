# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 当前状态

Phase 1（T01-T25）和 Phase 2（T30-T45）已全部完成（45/45）。
Phase 3 T0 已完成。

**T0 完成内容**：
- `foundation::Variant` 已扩展为 `std::variant<double, int, std::string, bool, math::Vec3>`
- 新增 `variantToBool()` 和 `variantToVec3()` 辅助函数（`src/foundation/Types.h`）
- `ProjectSerializer` 中 `variantToJson()`/`jsonToVariant()` 已支持 bool 和 Vec3 序列化
- JSON 格式：bool → `{"type":"bool","value":true}`；Vec3 → `{"type":"vec3","x":...,"y":...,"z":...}`
- 新增 9 个 Variant 测试（test_foundation）+ 1 个 round-trip 序列化测试（test_project_serializer）

**关键上下文**：
- `src/foundation/Types.h` 现在 `#include "foundation/Math.h"`（无循环依赖）
- `PropertyApplier`（T2 将会创建）通过 `DocumentObject` 虚方法分派，不再用集中式 dynamic_cast 链
- 现有约 35 个测试全部通过

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T1 |
| **任务名** | DocumentObject setProperty/getProperty 虚方法 |
| **推荐模型** | Sonnet 4.6 |
| **前置依赖** | T0（已完成） |

### 具体工作

1. **`src/model/DocumentObject.h`**：新增两个纯虚方法（或有默认实现的虚方法）：
   ```cpp
   virtual bool setProperty(const std::string& key, const foundation::Variant& value);
   virtual foundation::Variant getProperty(const std::string& key) const;
   ```
   默认实现：`setProperty` 返回 false，`getProperty` 抛出 `std::out_of_range`

2. **各 model 子类** 覆写 `setProperty`/`getProperty`，覆盖属于该类型的属性 key：
   - `PipePoint`：name, position (Vec3), pipeSpecId, param (typeParams), bendMultiplier 等
   - `PipeSpec`：od, wallThickness, material, field (PipeSpec::fields)
   - `ThermalLoad`：installTemp, operatingTemp
   - `PressureLoad`：pressure, isExternal (bool)
   - `WindLoad`：speed, direction (Vec3)
   - `SeismicLoad`：acceleration, direction (Vec3)
   - `DisplacementLoad`：translation (Vec3), rotation (Vec3)
   - `UserDefinedLoad`：force (Vec3), moment (Vec3)
   - `ProjectConfig`：projectName, author, standard
   - 其他常用类按需覆写

3. **`tests/test_model.cpp`**（或新建 `tests/test_property_dispatch.cpp`）：
   - 验证各对象通过 setProperty/getProperty 正确读写
   - 验证不支持的 key 返回 false / 抛异常
   - 验证 bool 和 Vec3 类型属性的 round-trip

### 验收标准

- `pixi run test` 全部通过
- 所有主要模型类覆写 setProperty/getProperty
- 未知 key 行为确定（setProperty 返回 false，getProperty 抛 std::out_of_range）

## 需要读取的文件

1. `src/model/DocumentObject.h` — 当前基类定义，在此新增虚方法
2. `src/model/PipePoint.h` — 主要模型类，需覆写（有 typeParams、name、position 等）
3. `src/model/PipeSpec.h` — 需覆写（od、wallThickness、material、fields）
4. `src/model/ThermalLoad.h` / `src/model/PressureLoad.h` / `src/model/WindLoad.h` — 载荷类层次，需抽样覆写
5. `tests/test_model.cpp` — 已有测试结构，在此追加新测试
6. `docs/command-pattern-design.md` §2.7 — PropertyApplier 和 DocumentObject 虚方法设计规格

