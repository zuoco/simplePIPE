#pragma once

#include <array>
#include <cmath>
#include <optional>

namespace foundation {
namespace math {

// ============================================================
// 常量
// ============================================================

constexpr double PI = 3.14159265358979323846;

// ============================================================
// 角度 / 弧度 转换
// ============================================================

inline double degToRad(double deg) { return deg * PI / 180.0; }
inline double radToDeg(double rad) { return rad * 180.0 / PI; }

// ============================================================
// 3D 向量 (轻量实现，不依赖 OCCT)
// ============================================================

struct Vec3 {
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(double s) const { return {x / s, y / s, z / s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }

    bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
};

inline double length(const Vec3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vec3 normalize(const Vec3& v) {
    double len = length(v);
    if (len < 1e-12) return {0, 0, 0};
    return v / len;
}

inline double dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// 两向量夹角 (rad)，范围 [0, π]
inline double angleBetween(const Vec3& a, const Vec3& b) {
    double la = length(a), lb = length(b);
    if (la < 1e-12 || lb < 1e-12) return 0.0;
    double c = dot(a, b) / (la * lb);
    // clamp 防止数值误差导致 acos 越界
    if (c > 1.0) c = 1.0;
    if (c < -1.0) c = -1.0;
    return std::acos(c);
}

// ============================================================
// 两直线最近点 / 交点计算
//
// 直线1: P1 + t * D1
// 直线2: P2 + s * D2
//
// 返回两最近点的中点；若两线几乎平行 (无交点) 返回 nullopt
// 精度要求 < 1e-6 mm
// ============================================================

inline std::optional<Vec3> lineLineIntersect(
    const Vec3& p1, const Vec3& d1,
    const Vec3& p2, const Vec3& d2,
    double parallelTol = 1e-10)
{
    Vec3 w  = {p1.x - p2.x, p1.y - p2.y, p1.z - p2.z};
    double a = dot(d1, d1);
    double b = dot(d1, d2);
    double c = dot(d2, d2);
    double d = dot(d1, w);
    double e = dot(d2, w);

    double denom = a * c - b * b;
    if (std::abs(denom) < parallelTol) return std::nullopt; // 平行

    double t = (b * e - c * d) / denom;
    double s = (a * e - b * d) / denom;

    Vec3 q1 = {p1.x + d1.x * t, p1.y + d1.y * t, p1.z + d1.z * t};
    Vec3 q2 = {p2.x + d2.x * s, p2.y + d2.y * s, p2.z + d2.z * s};

    // 返回两最近点的中点（若真相交，两点重合）
    return Vec3{(q1.x + q2.x) * 0.5, (q1.y + q2.y) * 0.5, (q1.z + q2.z) * 0.5};
}

} // namespace math
} // namespace foundation
