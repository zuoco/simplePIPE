# Phase 4 / lib::platform::occt — OCCT 几何封装
# T59: geometry 源文件已迁入 geometry/ 子目录，保持 #include "geometry/..." 路径兼容

add_library(lib_platform_occt STATIC
    geometry/BooleanOps.cpp
    geometry/ShapeBuilder.cpp
    geometry/ShapeTransform.cpp
    geometry/ShapeMesher.cpp
    geometry/StepIO.cpp
    geometry/ShapeProperties.cpp
)
add_library(lib::platform::occt ALIAS lib_platform_occt)

target_include_directories(lib_platform_occt
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(lib_platform_occt PUBLIC cxx_std_17)

target_link_libraries(lib_platform_occt
    PUBLIC
        lib::base
    PRIVATE
        ${OpenCASCADE_FoundationClasses_LIBRARIES}
        ${OpenCASCADE_ModelingData_LIBRARIES}
        ${OpenCASCADE_ModelingAlgorithms_LIBRARIES}
        ${OpenCASCADE_DataExchange_LIBRARIES}
)
