#include "Intersection.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ox {

std::optional<Intersection> intersectAABB(const Ray& ray, const AABB& box) {
    if (!box.valid() || !ray.origin.finite() || !ray.direction.finite() || ray.maxDistance <= 0.0f) return std::nullopt;
    float tmin = 0.0f, tmax = ray.maxDistance;
    Vec3 enterNormal{};
    const std::array<float, 3> o{ray.origin.x, ray.origin.y, ray.origin.z};
    const std::array<float, 3> d{ray.direction.x, ray.direction.y, ray.direction.z};
    const std::array<float, 3> lo{box.min.x, box.min.y, box.min.z};
    const std::array<float, 3> hi{box.max.x, box.max.y, box.max.z};
    for (std::size_t axis = 0; axis < 3u; ++axis) {
        if (std::abs(d[axis]) < 1.0e-7f) {
            if (o[axis] < lo[axis] || o[axis] > hi[axis]) return std::nullopt;
            continue;
        }
        float t1 = (lo[axis] - o[axis]) / d[axis];
        float t2 = (hi[axis] - o[axis]) / d[axis];
        float sign = -1.0f;
        if (t1 > t2) { std::swap(t1, t2); sign = 1.0f; }
        if (t1 > tmin) {
            tmin = t1;
            enterNormal = {};
            if (axis == 0u) enterNormal.x = sign;
            else if (axis == 1u) enterNormal.y = sign;
            else enterNormal.z = sign;
        }
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return std::nullopt;
    }
    if (tmax < 0.0f || tmin > ray.maxDistance) return std::nullopt;
    const float t = std::max(0.0f, tmin);
    return Intersection{t, ray.origin + ray.direction * t, enterNormal};
}

std::optional<Intersection> intersectSphere(const Ray& ray, const Vec3& center, float radius) {
    if (!center.finite() || radius <= 0.0f || !std::isfinite(radius)) return std::nullopt;
    const Vec3 oc = ray.origin - center;
    const float b = oc.dot(ray.direction);
    const float c = oc.dot(oc) - radius * radius;
    const float disc = b * b - c;
    if (disc < 0.0f) return std::nullopt;
    const float root = std::sqrt(disc);
    float t = -b - root;
    if (t < 0.0f) t = -b + root;
    if (t < 0.0f || t > ray.maxDistance) return std::nullopt;
    const Vec3 p = ray.origin + ray.direction * t;
    const auto n = (p - center).normalized();
    return Intersection{t, p, n.value_or(Vec3{})};
}

std::optional<Intersection> intersectCapsule(const Ray& ray, const Capsule& capsule) {
    if (capsule.radius <= 0.0f || !capsule.a.finite() || !capsule.b.finite()) return std::nullopt;
    const Vec3 ba = capsule.b - capsule.a;
    const Vec3 oa = ray.origin - capsule.a;
    const float baba = ba.dot(ba);
    const float bard = ba.dot(ray.direction);
    const float baoa = ba.dot(oa);
    const float rdoa = ray.direction.dot(oa);
    const float oaoa = oa.dot(oa);
    const float a = baba - bard * bard;
    const float b = baba * rdoa - baoa * bard;
    const float c = baba * oaoa - baoa * baoa - capsule.radius * capsule.radius * baba;
    if (std::abs(a) > 1.0e-7f) {
        const float h = b * b - a * c;
        if (h >= 0.0f) {
            const float t = (-b - std::sqrt(h)) / a;
            const float y = baoa + t * bard;
            if (t >= 0.0f && t <= ray.maxDistance && y > 0.0f && y < baba) {
                const Vec3 p = ray.origin + ray.direction * t;
                const Vec3 axisPoint = capsule.a + ba * (y / baba);
                return Intersection{t, p, (p - axisPoint).normalized().value_or(Vec3{})};
            }
        }
    }
    auto first = intersectSphere(ray, capsule.a, capsule.radius);
    auto second = intersectSphere(ray, capsule.b, capsule.radius);
    if (!first) return second;
    if (!second) return first;
    return first->distance <= second->distance ? first : second;
}

} // namespace ox
