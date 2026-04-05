# Phase 4 / lib::platform::vsg — VSG 渲染封装
# T60: visualization 源文件已迁入 visualization/ 子目录，保持 #include "visualization/..." 路径兼容

add_library(lib_platform_vsg STATIC
    visualization/placeholder.cpp
    visualization/OcctToVsg.cpp
    visualization/PipePointNode.cpp
    visualization/ComponentNode.cpp
    visualization/LodStrategy.cpp
    visualization/SceneManager.cpp
    visualization/CameraController.cpp
    visualization/SceneFurniture.cpp
    visualization/PickHandler.cpp
    visualization/SelectionHighlight.cpp
    visualization/ViewManager.cpp
)
add_library(lib::platform::vsg ALIAS lib_platform_vsg)

target_include_directories(lib_platform_vsg
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(lib_platform_vsg PUBLIC cxx_std_17)

target_link_libraries(lib_platform_vsg
    PUBLIC
        engine
        lib::platform::occt
        vsg::vsg
        vtk_visualization
)
