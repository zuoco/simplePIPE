# Phase 4 / lib::runtime — 运行时核心
# T61: Document、DependencyGraph、SelectionManager + 全部 command/ 已迁入

add_library(lib_runtime STATIC
    # 文档层核心对象
    app/Document.cpp
    app/DependencyGraph.cpp
    app/DocumentSnapshot.cpp
    app/SelectionManager.cpp
    # 并发基础设施
    task/TaskQueue.cpp
    task/ResultChannel.cpp
    task/SceneUpdateAdapter.cpp
    # 命令层
    command/MacroCommand.cpp
    command/CommandStack.cpp
    command/SetPropertyCommand.cpp
    command/BatchSetPropertyCommand.cpp
    command/CreatePipePointCommand.cpp
    command/DeletePipePointCommand.cpp
    command/InsertComponentCommand.cpp
    command/CommandRegistry.cpp
)
add_library(lib::runtime ALIAS lib_runtime)

add_library(lib_runtime_modules STATIC)
add_library(lib::runtime::modules ALIAS lib_runtime_modules)

target_sources(lib_runtime_modules
    PUBLIC
    FILE_SET CXX_MODULES FILES
        runtimeMod/pipecad.runtime.document.cppm
        runtimeMod/pipecad.runtime.graph.cppm
        runtimeMod/pipecad.runtime.command.cppm
        runtimeMod/pipecad.runtime.serialize.cppm
        runtimeMod/pipecad.runtime.task.cppm
)

target_include_directories(lib_runtime_modules
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(lib_runtime_modules PUBLIC cxx_std_20)

target_link_libraries(lib_runtime_modules
    PUBLIC
        nlohmann_json::nlohmann_json
)

target_include_directories(lib_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(lib_runtime PUBLIC cxx_std_20)

target_link_libraries(lib_runtime
    PUBLIC
        engine
    lib_runtime_modules
        nlohmann_json::nlohmann_json
)
