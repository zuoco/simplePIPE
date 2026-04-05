# Phase 4 / lib::platform::vtk — VTK 分析封装（纯算法层，无 Qt 依赖）
# T60: vtk-visualization 中算法部分（OcctToVtk、BeamMeshBuilder、VtkSceneManager）
#      已迁入 vtk-visualization/ 子目录; Qt 集成层（VtkViewport）保留在 src/vtk-visualization/

add_library(lib_platform_vtk STATIC
    vtk-visualization/OcctToVtk.cpp
    vtk-visualization/BeamMeshBuilder.cpp
    vtk-visualization/VtkSceneManager.cpp
)
add_library(lib::platform::vtk ALIAS lib_platform_vtk)

target_include_directories(lib_platform_vtk
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(lib_platform_vtk PUBLIC cxx_std_17)

target_link_libraries(lib_platform_vtk
    PUBLIC
        lib::platform::occt
        VTK::CommonCore
        VTK::CommonDataModel
        VTK::RenderingCore
        VTK::InteractionStyle
        VTK::RenderingOpenGL2
)
