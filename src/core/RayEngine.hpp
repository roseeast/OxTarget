#pragma once

#include "CollisionBackend.hpp"
#include "EntityRegistry.hpp"
#include "HandlePool.hpp"
#include "SpatialIndex.hpp"
#include <chrono>
#include <memory>
#include <optional>

namespace ox {

struct RayResult {
    HitType hitType{HitType::None};
    EntityType entityType{EntityType::None};
    int entity{-1};
    int model{-1};
    int material{-1};
    Handle target{InvalidHandle};
    Vec3 position{};
    Vec3 normal{};
    float distance{};
    std::uint8_t zone{0};
    Vec3 localPosition{};
};

struct EntityHit {
    Intersection hit;
    Vec3 localPosition{};
    std::uint8_t zone{0};
};

struct RayStatistics {
    std::uint64_t raycasts{};
    std::uint64_t candidates{};
    std::uint64_t intersections{};
    double totalMilliseconds{};
    double maxMilliseconds{};
    double averageMilliseconds() const { return raycasts ? totalMilliseconds / static_cast<double>(raycasts) : 0.0; }
};

class RayEngine {
public:
    RayEngine(EntityRegistry& registry, SpatialIndex& spatial, std::unique_ptr<ICollisionBackend> backend);
    RayResult cast(const Ray& ray, std::uint32_t mask, int interior, int world,
                   const Vec3* referencePos = nullptr, int ignorePlayer = -1);
    std::optional<Intersection> castTarget(const Ray& ray, Handle target, float expansion = 0.0f) const;
    const RayStatistics& stats() const { return stats_; }
    std::size_t lastCandidateCount() const { return lastCandidates_; }
    ICollisionBackend& collisionBackend() { return *backend_; }
private:
    std::optional<EntityHit> intersectEntity(const Ray& ray, const EntitySnapshot& entity) const;
    EntityRegistry& registry_;
    SpatialIndex& spatial_;
    std::unique_ptr<ICollisionBackend> backend_;
    RayStatistics stats_;
    std::size_t lastCandidates_{};
};

} // namespace ox
