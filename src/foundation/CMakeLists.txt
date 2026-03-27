add_library(foundation STATIC placeholder.cpp)

target_include_directories(foundation
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(foundation PUBLIC cxx_std_17)
