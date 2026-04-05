# Phase 4 / lib::platform — 平台封装层
# 子域：occt（几何内核）/ vtk（算法,无Qt）/ vsg（渲染引擎,依赖vtk）
# T59: geometry 迁入 occt; T60: vtk-visualization 算法迁入 vtk, visualization 迁入 vsg

add_subdirectory(occt)  # lib_platform_occt — OCCT 几何封装
add_subdirectory(vtk)   # lib_platform_vtk  — VTK 算法层（无Qt）
add_subdirectory(vsg)   # lib_platform_vsg  — VSG 渲染层（依赖 vtk）
