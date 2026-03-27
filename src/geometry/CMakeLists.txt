add_library(geometry STATIC placeholder.cpp)

target_include_directories(geometry
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(geometry PUBLIC cxx_std_17)

target_link_libraries(geometry
    PUBLIC
        foundation
    PRIVATE
        ${OpenCASCADE_FoundationClasses_LIBRARIES}
        ${OpenCASCADE_ModelingData_LIBRARIES}
        ${OpenCASCADE_ModelingAlgorithms_LIBRARIES}
        ${OpenCASCADE_DataExchange_LIBRARIES}
)
