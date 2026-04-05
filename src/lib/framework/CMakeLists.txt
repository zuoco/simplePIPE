# Phase 4 / lib::framework — 框架层
# T61: Application、WorkbenchManager、全部 Workbench、ProjectSerializer、StepExporter 已迁入

add_library(lib_framework STATIC
    app/Application.cpp
    app/WorkbenchManager.cpp
    app/CadWorkbench.cpp
    app/DesignWorkbench.cpp
    app/SpecWorkbench.cpp
    app/AnalysisWorkbench.cpp
    app/ProjectSerializer.cpp
    app/StepExporter.cpp
)
add_library(lib::framework ALIAS lib_framework)

add_library(lib_framework_modules STATIC)
add_library(lib::framework::modules ALIAS lib_framework_modules)

target_sources(lib_framework_modules
    PUBLIC
    FILE_SET CXX_MODULES FILES
        frameworkMod/pipecad.framework.application.cppm
        frameworkMod/pipecad.framework.workbench.cppm
        frameworkMod/pipecad.framework.scene.cppm
        frameworkMod/pipecad.framework.cppm
)

target_include_directories(lib_framework_modules
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(lib_framework_modules PUBLIC cxx_std_20)

target_include_directories(lib_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(lib_framework PUBLIC cxx_std_17)

target_link_libraries(lib_framework
    PUBLIC
        lib_runtime
        lib_framework_modules
        visualization
        nlohmann_json::nlohmann_json
    PRIVATE
        ${OpenCASCADE_ApplicationFramework_LIBRARIES}
        ${OpenCASCADE_DataExchange_LIBRARIES}
)
