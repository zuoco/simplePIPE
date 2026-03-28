add_library(engine STATIC
    BendCalculator.cpp
    RunBuilder.cpp
    BendBuilder.cpp
    ReducerBuilder.cpp
    TeeBuilder.cpp
    GeometryDeriver.cpp
)

target_include_directories(engine
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(engine PUBLIC cxx_std_17)

target_link_libraries(engine
    PUBLIC
        model
        geometry
)
