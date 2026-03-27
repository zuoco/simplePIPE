#include <gtest/gtest.h>
#include "foundation/Types.h"
#include "foundation/Math.h"
#include "foundation/Signal.h"
#include "foundation/Log.h"

using namespace foundation;
using namespace foundation::math;

// ============================================================
// UUID Tests
// ============================================================

TEST(UUID, GenerateIsUnique) {
    UUID a = UUID::generate();
    UUID b = UUID::generate();
    EXPECT_NE(a, b);
}

TEST(UUID, FormatIsCorrect) {
    UUID uuid = UUID::generate();
    std::string s = uuid.toString();
    // 格式: 8-4-4-4-12 (含 4 个连字符，共 36 字符)
    ASSERT_EQ(s.size(), 36u);
    EXPECT_EQ(s[8],  '-');
    EXPECT_EQ(s[13], '-');
    EXPECT_EQ(s[18], '-');
    EXPECT_EQ(s[23], '-');
}

TEST(UUID, VersionBits) {
    UUID uuid = UUID::generate();
    // data[6] 高四位应为 0x4 (version 4)
    EXPECT_EQ((uuid.data[6] >> 4), 0x4);
    // data[8] 高两位应为 0b10 (variant)
    EXPECT_EQ((uuid.data[8] >> 6), 0x2);
}

TEST(UUID, NullIsNull) {
    UUID null{};
    EXPECT_TRUE(null.isNull());
    UUID real = UUID::generate();
    EXPECT_FALSE(real.isNull());
}

// ============================================================
// Variant Tests
// ============================================================

TEST(Variant, StoreAndRetrieveDouble) {
    Variant v = 3.14;
    EXPECT_DOUBLE_EQ(variantToDouble(v), 3.14);
}

TEST(Variant, StoreAndRetrieveInt) {
    Variant v = 42;
    EXPECT_EQ(variantToInt(v), 42);
}

TEST(Variant, StoreAndRetrieveString) {
    Variant v = std::string("hello");
    EXPECT_EQ(variantToString(v), "hello");
}

TEST(Variant, TypeSafe_WrongTypeThrows) {
    Variant v = std::string("not a number");
    EXPECT_THROW(variantToDouble(v), std::bad_variant_access);
}

TEST(Variant, IntToDouble) {
    Variant v = 10;
    EXPECT_DOUBLE_EQ(variantToDouble(v), 10.0);
}

// ============================================================
// Math: deg/rad conversion
// ============================================================

TEST(Math, DegToRad) {
    EXPECT_NEAR(degToRad(0.0),   0.0,       1e-12);
    EXPECT_NEAR(degToRad(90.0),  PI / 2.0,  1e-12);
    EXPECT_NEAR(degToRad(180.0), PI,         1e-12);
    EXPECT_NEAR(degToRad(360.0), 2.0 * PI,  1e-12);
}

TEST(Math, RadToDeg) {
    EXPECT_NEAR(radToDeg(0.0),       0.0,   1e-10);
    EXPECT_NEAR(radToDeg(PI / 2.0),  90.0,  1e-10);
    EXPECT_NEAR(radToDeg(PI),        180.0, 1e-10);
    EXPECT_NEAR(radToDeg(2.0 * PI),  360.0, 1e-10);
}

TEST(Math, ConversionRoundTrip) {
    double deg = 37.5;
    EXPECT_NEAR(radToDeg(degToRad(deg)), deg, 1e-12);
}

// ============================================================
// Math: Vec3 operations
// ============================================================

TEST(Math, VecLength) {
    Vec3 v{3, 4, 0};
    EXPECT_NEAR(length(v), 5.0, 1e-12);
}

TEST(Math, VecNormalize) {
    Vec3 v{3, 4, 0};
    Vec3 n = normalize(v);
    EXPECT_NEAR(length(n), 1.0, 1e-12);
    EXPECT_NEAR(n.x, 0.6, 1e-12);
    EXPECT_NEAR(n.y, 0.8, 1e-12);
}

TEST(Math, Dot) {
    Vec3 a{1, 0, 0};
    Vec3 b{0, 1, 0};
    EXPECT_NEAR(dot(a, b), 0.0, 1e-12);

    Vec3 c{1, 0, 0};
    EXPECT_NEAR(dot(a, c), 1.0, 1e-12);
}

TEST(Math, Cross) {
    Vec3 x{1, 0, 0};
    Vec3 y{0, 1, 0};
    Vec3 z = cross(x, y);
    EXPECT_NEAR(z.x, 0.0, 1e-12);
    EXPECT_NEAR(z.y, 0.0, 1e-12);
    EXPECT_NEAR(z.z, 1.0, 1e-12);
}

TEST(Math, AngleBetween_90deg) {
    Vec3 a{1, 0, 0};
    Vec3 b{0, 1, 0};
    EXPECT_NEAR(angleBetween(a, b), PI / 2.0, 1e-12);
}

TEST(Math, AngleBetween_0deg) {
    Vec3 a{1, 0, 0};
    EXPECT_NEAR(angleBetween(a, a), 0.0, 1e-12);
}

TEST(Math, AngleBetween_180deg) {
    Vec3 a{1, 0, 0};
    Vec3 b{-1, 0, 0};
    EXPECT_NEAR(angleBetween(a, b), PI, 1e-12);
}

// ============================================================
// Math: 两线交点
// ============================================================

TEST(Math, LineLineIntersect_XY_Plane) {
    // 线1: (0,0,0) → X方向
    // 线2: (1,0,0) → Y方向
    // 交点: (1,0,0)
    Vec3 p1{0, 0, 0}, d1{1, 0, 0};
    Vec3 p2{1, 0, 0}, d2{0, 1, 0};
    auto result = lineLineIntersect(p1, d1, p2, d2);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 1.0, 1e-6);
    EXPECT_NEAR(result->y, 0.0, 1e-6);
    EXPECT_NEAR(result->z, 0.0, 1e-6);
}

TEST(Math, LineLineIntersect_Diagonal) {
    // 线1: (0,0,0) + t*(1,1,0)
    // 线2: (1,0,0) + s*(-1,1,0)
    // 交点: (0.5, 0.5, 0)
    Vec3 p1{0, 0, 0}, d1{1, 1, 0};
    Vec3 p2{1, 0, 0}, d2{-1, 1, 0};
    auto result = lineLineIntersect(p1, d1, p2, d2);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 0.5, 1e-6);
    EXPECT_NEAR(result->y, 0.5, 1e-6);
    EXPECT_NEAR(result->z, 0.0, 1e-6);
}

TEST(Math, LineLineIntersect_3D) {
    // 管道场景：两条管段方向线的交点
    // 线1: (0,0,0) → Z方向
    // 线2: (5,0,3) → X方向
    // 交点: (0,0,3) ... 实际最近点中点 = (2.5,0,3)? 不，真相交
    // 线1 过 (0,0,3), 线2 过 (0,0,3) 同为 (0,0,3)
    Vec3 p1{0, 0, 0}, d1{0, 0, 1};
    Vec3 p2{0, 0, 3}, d2{1, 0, 0};  // 两线在(0,0,3)相交
    auto result = lineLineIntersect(p1, d1, p2, d2);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 0.0, 1e-6);
    EXPECT_NEAR(result->y, 0.0, 1e-6);
    EXPECT_NEAR(result->z, 3.0, 1e-6);
}

TEST(Math, LineLineIntersect_Parallel_ReturnsNullopt) {
    Vec3 p1{0, 0, 0}, d1{1, 0, 0};
    Vec3 p2{0, 1, 0}, d2{1, 0, 0};  // 平行线
    auto result = lineLineIntersect(p1, d1, p2, d2);
    EXPECT_FALSE(result.has_value());
}

// ============================================================
// Signal Tests
// ============================================================

TEST(Signal, ConnectAndEmit) {
    Signal<int> sig;
    int received = -1;
    sig.connect([&](int v){ received = v; });
    sig.emit(42);
    EXPECT_EQ(received, 42);
}

TEST(Signal, MultipleSlots) {
    Signal<> sig;
    int count = 0;
    sig.connect([&]{ ++count; });
    sig.connect([&]{ ++count; });
    sig.connect([&]{ ++count; });
    sig.emit();
    EXPECT_EQ(count, 3);
}

TEST(Signal, Disconnect) {
    Signal<int> sig;
    int received = -1;
    auto id = sig.connect([&](int v){ received = v; });
    sig.disconnect(id);
    sig.emit(99);
    EXPECT_EQ(received, -1);  // 未触发
}

TEST(Signal, DisconnectAll) {
    Signal<> sig;
    int count = 0;
    sig.connect([&]{ ++count; });
    sig.connect([&]{ ++count; });
    sig.disconnectAll();
    sig.emit();
    EXPECT_EQ(count, 0);
}

TEST(Signal, ConnectionCount) {
    Signal<> sig;
    EXPECT_EQ(sig.connectionCount(), 0u);
    auto id1 = sig.connect([]{ });
    auto id2 = sig.connect([]{ });
    EXPECT_EQ(sig.connectionCount(), 2u);
    sig.disconnect(id1);
    EXPECT_EQ(sig.connectionCount(), 1u);
    sig.disconnect(id2);
    EXPECT_EQ(sig.connectionCount(), 0u);
}

// ============================================================
// Log Tests (仅验证不崩溃)
// ============================================================

TEST(Log, DoesNotCrash) {
    LOG_DEBUG("debug message");
    LOG_INFO("info message");
    LOG_WARN("warn message");
    LOG_ERROR("error message");
    SUCCEED();
}

TEST(Log, LevelFiltering) {
    foundation::log::minLevel() = foundation::log::Level::Error;
    // 低级别日志不输出（无法直接断言 stdout，仅验证不崩溃）
    LOG_DEBUG("this should be filtered");
    LOG_INFO("this should be filtered");
    LOG_WARN("this should be filtered");
    LOG_ERROR("this should show");
    foundation::log::minLevel() = foundation::log::Level::Debug;  // 恢复
    SUCCEED();
}
