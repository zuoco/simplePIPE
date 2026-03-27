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
