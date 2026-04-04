add_library(app STATIC
    # --- 原 app 层 ---
    placeholder.cpp
    Application.cpp
    Document.cpp
    ProjectSerializer.cpp
    StepExporter.cpp
    DependencyGraph.cpp
    WorkbenchManager.cpp
    CadWorkbench.cpp
    DesignWorkbench.cpp
    SpecWorkbench.cpp
    AnalysisWorkbench.cpp
    SelectionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/RecomputeEngine.cpp
    # --- 原 command 层（合并，消除循环依赖） ---
    ${CMAKE_SOURCE_DIR}/src/command/MacroCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/command/CommandStack.cpp
    ${CMAKE_SOURCE_DIR}/src/command/SetPropertyCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/command/BatchSetPropertyCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/command/CreatePipePointCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/command/DeletePipePointCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/command/InsertComponentCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/command/CommandRegistry.cpp
)

target_include_directories(app
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(app PUBLIC cxx_std_17)

target_link_libraries(app
    PUBLIC
        engine
        visualization
        nlohmann_json::nlohmann_json
    PRIVATE
        ${OpenCASCADE_ApplicationFramework_LIBRARIES}
        ${OpenCASCADE_DataExchange_LIBRARIES}
)
