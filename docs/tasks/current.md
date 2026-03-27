# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T03 / T04 |
| **任务名** | OCCT 几何封装 / OCCT 网格化+STEP I/O |
| **推荐模型** | Sonnet |
| **前置依赖** | T01 ✅ |
| **前置状态** | ✅ 所有依赖已满足（T03/T04 可并行） |

## 项目进度

- 已完成: 2/22 个任务
- 当前阶段: Phase 1 — 基础设施

## 上一个完成的任务

**T02 — Foundation 层 (2026-03-28)**
- 产出: Types.h / Math.h / Signal.h / Log.h + test_foundation.cpp
- 验证: `pixi run test` ✅ 2/2 通过
- 关键接口:
  - `foundation::UUID::generate()` — 随机 v4 UUID
  - `foundation::Variant = std::variant<double,int,std::string>`
  - `foundation::math::Vec3` + 向量运算 + `lineLineIntersect`
  - `foundation::Signal<Args...>` 轻量信号槽
  - `LOG_DEBUG/INFO/WARN/ERROR(msg)` 日志宏

## 给 AI 的指令

**做 T03（OCCT 几何封装）**：

1. 读取 `docs/development-plan.md` 中 **T03** 章节
2. 读取 `lib/occt/AGENTS.md` 获取 OCCT API 使用指南
3. 前置头文件: `src/foundation/Types.h` (UUID/Variant)
4. 主要文件路径:
   - `src/geometry/OcctTypes.h` — Handle 别名、前置声明
   - `src/geometry/ShapeBuilder.h/cpp` — makeCylinder/makeTorus/makeCone/makePipeShell
   - `src/geometry/BooleanOps.h/cpp` — cut/fuse
   - `src/geometry/ShapeTransform.h/cpp` — translate/rotate/transform
   - `tests/test_geometry.cpp`
5. OCCT include 路径: `${OpenCASCADE_INCLUDE_DIR}` → `lib/occt/include/opencascade/`
6. OCCT 目标库已在 `src/geometry/CMakeLists.txt` 中链接好
7. 完成后运行 `pixi run build-debug && pixi run test`
8. 验证通过后更新 `docs/tasks/status.md` 和本文件
