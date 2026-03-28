find_package(GTest REQUIRED)

# 构建系统占位测试 — T01 验证
add_executable(test_build_system test_build_system.cpp)
target_link_libraries(test_build_system
    PRIVATE
        foundation
        GTest::GTest
        GTest::Main
)
target_compile_features(test_build_system PRIVATE cxx_std_17)
add_test(NAME BuildSystem COMMAND test_build_system)

# Foundation 层测试 — T02
add_executable(test_foundation test_foundation.cpp)
target_link_libraries(test_foundation
    PRIVATE
        foundation
        GTest::GTest
        GTest::Main
)
target_compile_features(test_foundation PRIVATE cxx_std_17)
add_test(NAME Foundation COMMAND test_foundation)

# Geometry 层测试 — T03
add_executable(test_geometry test_geometry.cpp)
target_link_libraries(test_geometry
    PRIVATE
        geometry
        GTest::GTest
        GTest::Main
)
target_include_directories(test_geometry PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_geometry PRIVATE cxx_std_17)
add_test(NAME Geometry COMMAND test_geometry)

# Mesh + STEP I/O 测试 — T04
add_executable(test_mesh_step test_mesh_step.cpp)
target_link_libraries(test_mesh_step
    PRIVATE
        geometry
        GTest::GTest
        GTest::Main
)
target_include_directories(test_mesh_step PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_mesh_step PRIVATE cxx_std_17)
add_test(NAME MeshStep COMMAND test_mesh_step)

# Model 层测试 — T05
add_executable(test_model test_model.cpp)
target_link_libraries(test_model
    PRIVATE
        model
        GTest::GTest
        GTest::Main
)
target_include_directories(test_model PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_model PRIVATE cxx_std_17)
add_test(NAME Model COMMAND test_model)

# Engine 层测试 — T07
add_executable(test_engine test_engine.cpp)
target_link_libraries(test_engine
    PRIVATE
        engine
        GTest::GTest
        GTest::Main
)
target_include_directories(test_engine PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_engine PRIVATE cxx_std_17)
add_test(NAME Engine COMMAND test_engine)

# 管件几何测试 — T08
add_executable(test_pipe_geometry test_pipe_geometry.cpp)
target_link_libraries(test_pipe_geometry
    PRIVATE
        engine
        GTest::GTest
        GTest::Main
)
target_include_directories(test_pipe_geometry PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_pipe_geometry PRIVATE cxx_std_17)
add_test(NAME PipeGeometry COMMAND test_pipe_geometry)

# 阀门/柔性接头/型钢梁/附件测试 — T09
add_executable(test_valve_flex_beam test_valve_flex_beam.cpp)
target_link_libraries(test_valve_flex_beam
    PRIVATE
        engine
        GTest::GTest
        GTest::Main
)
target_include_directories(test_valve_flex_beam PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_valve_flex_beam PRIVATE cxx_std_17)
add_test(NAME ValveFlexBeam COMMAND test_valve_flex_beam)

# 附属对象与梁测试 — T06
add_executable(test_accessory_beam test_accessory_beam.cpp)
target_link_libraries(test_accessory_beam
    PRIVATE
        model
        GTest::GTest
        GTest::Main
)
target_include_directories(test_accessory_beam PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_accessory_beam PRIVATE cxx_std_17)
add_test(NAME AccessoryBeam COMMAND test_accessory_beam)
