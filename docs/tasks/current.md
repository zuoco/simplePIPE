# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T02 / T03 / T04 |
| **任务名** | Foundation 层 / OCCT 几何封装 / OCCT 网格化+STEP I/O |
| **推荐模型** | Sonnet |
| **前置依赖** | T01 ✅ |
| **前置状态** | ✅ 所有依赖已满足（T02/T03/T04 可并行） |

## 项目进度

- 已完成: 1/22 个任务
- 当前阶段: Phase 1 — 基础设施

## 上一个完成的任务

**T01 — 构建系统搭建 (2026-03-28)**
- 产出: pixi.toml, CMakeLists.txt, 7层 src/库骨架, tests/, .gitignore
- 验证: `pixi run build-debug` ✅ 编译通过, `pixi run test` ✅ 1/1 测试通过
- 关键接口:
  - 7个 static lib target: `foundation` / `geometry` / `model` / `engine` / `visualization` / `app` / `ui`
  - include 路径: `${CMAKE_SOURCE_DIR}/src`，头文件引用方式 `"layer/Header.h"`
  - OCCT: `${OpenCASCADE_INCLUDE_DIR}`, 各模块库变量见 status.md
  - VSG: `vsg::vsg`, Qt6: `Qt6::Quick Qt6::Qml`, JSON: `nlohmann_json::nlohmann_json`

## 给 AI 的指令

**建议优先做 T02**（其他任务依赖 T02 的 Types.h/Math.h）：

1. 读取 `docs/development-plan.md` 中 **T02** 章节
2. 读取 `docs/architecture.md` **§1、§3** 中基础类型相关章节
3. 主要文件路径:
   - `src/foundation/Types.h` — UUID、Variant、单位枚举
   - `src/foundation/Math.h` — 向量运算、角度转换、两线交点
   - `src/foundation/Signal.h` — 轻量信号/槽
   - `src/foundation/Log.h` — 日志宏
   - `tests/test_foundation.cpp` — 单元测试
4. 将 `src/foundation/placeholder.cpp` 替换为真实实现
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
