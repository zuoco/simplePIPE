add_library(model STATIC placeholder.cpp)

target_include_directories(model
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(model PUBLIC cxx_std_17)

target_link_libraries(model
    PUBLIC
        foundation
    PRIVATE
        ${OpenCASCADE_FoundationClasses_LIBRARIES}
        ${OpenCASCADE_ModelingData_LIBRARIES}
)
