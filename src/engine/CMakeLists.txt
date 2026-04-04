add_library(engine STATIC
    BendCalculator.cpp
    RunBuilder.cpp
    BendBuilder.cpp
    ReducerBuilder.cpp
    TeeBuilder.cpp
    ValveBuilder.cpp
    FlexJointBuilder.cpp
    BeamBuilder.cpp
    AccessoryBuilder.cpp
    GeometryDeriver.cpp
    ComponentCatalog.cpp
    TopologyManager.cpp
    ConstraintSolver.cpp
    PipelineValidator.cpp
)

target_include_directories(engine
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(engine PUBLIC cxx_std_17)

target_link_libraries(engine
    PUBLIC
        model
        geometry
        command
    PRIVATE
        ${OpenCASCADE_ModelingAlgorithms_LIBRARIES}
)
