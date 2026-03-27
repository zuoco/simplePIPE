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
