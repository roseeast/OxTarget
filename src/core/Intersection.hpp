#pragma once
#include "Math/AABB.hpp"
#include "Math/Capsule.hpp"
#include "Math/Ray.hpp"
#include <optional>

namespace ox {
struct Intersection { float distance{}; Vec3 position; Vec3 normal; };
std::optional<Intersection> intersectAABB(const Ray& ray, const AABB& box);
std::optional<Intersection> intersectSphere(const Ray& ray, const Vec3& center, float radius);
std::optional<Intersection> intersectCapsule(const Ray& ray, const Capsule& capsule);
}

