# Phase 4 / lib::runtime — 运行时核心
# T61: Document、DependencyGraph、SelectionManager + 全部 command/ 已迁入

add_library(lib_runtime STATIC
    # 文档层核心对象
    app/Document.cpp
    app/DependencyGraph.cpp
    app/SelectionManager.cpp
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

target_include_directories(lib_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(lib_runtime PUBLIC cxx_std_17)

target_link_libraries(lib_runtime
    PUBLIC
        engine
        nlohmann_json::nlohmann_json
)
