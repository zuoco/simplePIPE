add_library(app STATIC
    placeholder.cpp
    Document.cpp
    DependencyGraph.cpp
    TransactionManager.cpp
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
        visualization
        nlohmann_json::nlohmann_json
)
