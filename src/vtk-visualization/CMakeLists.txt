# T60: vtk-visualization 已拆分——算法层(无Qt)迁入 lib_platform_vtk
# 此处仅保留 VtkViewport（Qt集成层）

add_library(vtk_visualization STATIC
    VtkViewport.cpp
)

target_include_directories(vtk_visualization
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(vtk_visualization PUBLIC cxx_std_17)

target_link_libraries(vtk_visualization
    PUBLIC
        lib_platform_vtk
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
)
