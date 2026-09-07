#include "EntityRegistry.hpp"
#include <algorithm>
#include <cmath>

namespace ox {

std::optional<EntitySnapshot> EntityRegistry::refresh(Handle target, const EntityKey& key) {
    auto value = provider_.snapshot(key);
    if (!value || !value->position.finite() || !value->localBounds.valid()) {
        snapshots_.erase(target);
        return std::nullopt;
    }
    snapshots_[target] = *value;
    return value;
}

void EntityRegistry::set(Handle target, const EntitySnapshot& snapshot) { snapshots_[target] = snapshot; }

const EntitySnapshot* EntityRegistry::get(Handle target) const {
    const auto found = snapshots_.find(target);
    return found == snapshots_.end() ? nullptr : &found->second;
}

void EntityRegistry::remove(Handle target) { snapshots_.erase(target); }

AABB EntityRegistry::worldBounds(const EntitySnapshot& e) const {
    if (e.capsule) {
        return {{e.position.x - e.capsuleRadius, e.position.y - e.capsuleRadius, e.position.z},
                {e.position.x + e.capsuleRadius, e.position.y + e.capsuleRadius, e.position.z + e.capsuleHeight}};
    }
    const float angle = e.rotation.z * 0.01745329251994329577f;
    const float c = std::abs(std::cos(angle)), s = std::abs(std::sin(angle));
    const Vec3 half = (e.localBounds.max - e.localBounds.min) * 0.5f;
    const Vec3 center = e.position + (e.localBounds.min + e.localBounds.max) * 0.5f;
    const Vec3 extent{c * half.x + s * half.y, s * half.x + c * half.y, half.z};
    return {center - extent, center + extent};
}

} // namespace ox
