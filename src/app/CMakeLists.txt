add_library(app STATIC
    placeholder.cpp
    Application.cpp
    Document.cpp
    ProjectSerializer.cpp
    StepExporter.cpp
    DependencyGraph.cpp
    TransactionManager.cpp
    WorkbenchManager.cpp
    CadWorkbench.cpp
    DesignWorkbench.cpp
    SpecWorkbench.cpp
    AnalysisWorkbench.cpp
    SelectionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/RecomputeEngine.cpp
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
        command
        visualization
        nlohmann_json::nlohmann_json
    PRIVATE
        ${OpenCASCADE_ApplicationFramework_LIBRARIES}
        ${OpenCASCADE_DataExchange_LIBRARIES}
)
