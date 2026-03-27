# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T04 / T05 |
| **任务名** | OCCT 网格化+STEP I/O / 核心文档对象 |
| **推荐模型** | Sonnet (T04) / **Opus** (T05) |
| **前置依赖** | T04←T01 ✅ / T05←T02 ✅ |
| **前置状态** | ✅ 两个任务均可立即开始（可并行） |

## 项目进度

- 已完成: 3/22 个任务
- 当前阶段: Phase 1 — 基础设施

## 上一个完成的任务

**T03 — OCCT 几何封装 (2026-03-28)**
- 产出: OcctTypes.h / ShapeBuilder / BooleanOps / ShapeTransform + test_geometry.cpp
- 验证: `pixi run test` ✅ 3/3 套件通过，13/13 测试通过
- 关键接口:
  - `geometry::ShapeBuilder::makeCylinder/makeTorus/makeCone/makePipeShell`
  - `geometry::BooleanOps::cut/fuse`
  - `geometry::ShapeTransform::translate/rotate/transform`
  - OCCT 头文件路径: `lib/occt/include/opencascade/`，用 `<BRepXxx.hxx>` 风格包含

## 给 AI 的指令

**做 T04（OCCT 网格化 + STEP I/O）**：

1. 读取 `docs/development-plan.md` 中 **T04** 章节
2. 读取 `lib/occt/AGENTS.md`
3. 前置代码: `src/geometry/ShapeBuilder.h` / `BooleanOps.h` / `ShapeTransform.h`
4. 主要文件路径:
   - `src/geometry/Tessellator.h/cpp` — tessellateMesh(shape, deflection)
   - `src/geometry/StepIO.h/cpp` — exportStep / importStep
5. OCCT 网格化: `BRepMesh_IncrementalMesh`, 遍历用 `TopExp_Explorer`
6. STEP I/O: `STEPControl_Writer` / `STEPControl_Reader`
7. 完成后运行 `pixi run build-debug && pixi run test`
8. 验证通过后更新 `docs/tasks/status.md` 和本文件

**做 T05（核心文档对象）**（Opus 推荐）：

1. 读取 `docs/development-plan.md` 中 **T05** 章节
2. 读取 `docs/architecture.md` §3 数据模型相关章节
3. 前置代码: `src/foundation/Types.h` / `foundation/Signal.h`
4. 主要文件路径:
   - `src/model/DocumentObject.h/cpp` — 基类, UUID, properties map
   - `src/model/PipeSpec.h/cpp` — 管道规格数据
5. 完成后运行 `pixi run build-debug && pixi run test`
8. 验证通过后更新 `docs/tasks/status.md` 和本文件
