#include <gtest/gtest.h>

// 构建系统验证占位测试 — 将在后续任务中替换为真实测试

TEST(BuildSystem, CanCompile) {
    ASSERT_TRUE(true);
}

TEST(BuildSystem, StaticAssertions) {
    static_assert(sizeof(int) == 4, "int must be 4 bytes");
    static_assert(sizeof(double) == 8, "double must be 8 bytes");
    SUCCEED();
}
