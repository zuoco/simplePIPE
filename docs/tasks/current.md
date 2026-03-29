# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | T38 |
| **任务名** | VTK 场景管理 |
| **推荐模型** | **Codex** |
| **前置依赖** | T37 |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: 30/38 个任务
- 当前阶段: Phase 2 — 二期开发

## 当前 ready 任务

| ID | 任务名 | 推荐模型 |
|----|--------|---------|
| T38 | VTK 场景管理 | **Codex** |
| T39 | 工作台切换 + QML 面板动态加载 | **Gemini** |
| T40 | StatusBar + 右键菜单 + 框选 | Sonnet |
| T41 | ComponentToolStrip 元件插入 | Sonnet |
| T43 | 序列化扩展 (Load/LoadCase) | **Codex** |

## 上一个完成的任务

T37 — OCCT→VTK 网格转换 (2026-03-29)
- 产出: `src/vtk-visualization/OcctToVtk.h/.cpp`, `src/vtk-visualization/BeamMeshBuilder.h/.cpp`, `src/vtk-visualization/CMakeLists.txt`, `tests/test_occt_to_vtk.cpp`
- 关键接口: `vtk_vis::toVtkPolyData(const TopoDS_Shape&, double)`, `vtk_vis::buildBeamMesh(const std::vector<gp_Pnt>&)`
- 注意事项: T38 已解锁并可直接复用上述 PolyData 构建接口创建 Solid/Beam Actor

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **T38** 章节
2. 读取 `docs/architecture.md` 中 VTK 可视化相关章节（Step 20、Analysis 渲染模式）
3. 读取前置代码: `src/vtk-visualization/OcctToVtk.h`, `src/vtk-visualization/BeamMeshBuilder.h`, `src/visualization/ViewManager.h`
4. 读取 `lib/vtk/AGENTS.md` 和 `.github/skills/industrial-software-dev/SKILL.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
