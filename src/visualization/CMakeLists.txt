add_library(visualization STATIC
    placeholder.cpp
    OcctToVsg.cpp
    PipePointNode.cpp
    ComponentNode.cpp
    LodStrategy.cpp
    SceneManager.cpp
    CameraController.cpp
    SceneFurniture.cpp
)

target_include_directories(visualization
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(visualization PUBLIC cxx_std_17)

target_link_libraries(visualization
    PUBLIC
        engine
        geometry
        vsg::vsg
)
