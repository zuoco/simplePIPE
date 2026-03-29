add_library(vtk_visualization STATIC
    OcctToVtk.cpp
    BeamMeshBuilder.cpp
)

target_include_directories(vtk_visualization
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(vtk_visualization PUBLIC cxx_std_17)

target_link_libraries(vtk_visualization
    PUBLIC
        geometry
        VTK::CommonCore
        VTK::CommonDataModel
)
