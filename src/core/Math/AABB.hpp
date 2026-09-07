#pragma once
#include "Vec3.hpp"

namespace ox {
struct AABB {
    Vec3 min;
    Vec3 max;
    bool valid() const {
        return min.finite() && max.finite() && min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }
    Vec3 center() const { return (min + max) * 0.5f; }
};
}

