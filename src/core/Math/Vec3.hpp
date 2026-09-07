#pragma once

#include <cmath>
#include <optional>

namespace ox {

struct Vec3 {
    float x{}, y{}, z{};

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    float lengthSquared() const { return dot(*this); }
    float length() const { return std::sqrt(lengthSquared()); }
    bool finite() const { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z); }
    std::optional<Vec3> normalized() const {
        const float len = length();
        if (!finite() || !std::isfinite(len) || len < 1.0e-5f) return std::nullopt;
        return *this / len;
    }
};

inline float distance(const Vec3& a, const Vec3& b) { return (a - b).length(); }

} // namespace ox

