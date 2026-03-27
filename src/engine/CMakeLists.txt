add_library(engine STATIC placeholder.cpp)

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
