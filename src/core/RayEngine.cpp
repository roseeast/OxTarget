#include "RayEngine.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace ox {

RayEngine::RayEngine(EntityRegistry& registry, SpatialIndex& spatial, std::unique_ptr<ICollisionBackend> backend)
    : registry_(registry), spatial_(spatial), backend_(std::move(backend)) {}

std::optional<EntityHit> RayEngine::intersectEntity(const Ray& ray, const EntitySnapshot& e) const {
    if (e.capsule) {
        auto hit = intersectCapsule(ray, {e.position + Vec3{0, 0, e.capsuleRadius},
                                      e.position + Vec3{0, 0, e.capsuleHeight - e.capsuleRadius}, e.capsuleRadius});
        if (!hit) return std::nullopt;
        return EntityHit{*hit, hit->position - e.position, ZoneAll};
    }
    const float angle = -e.rotation.z * 0.01745329251994329577f;
    const float c = std::cos(angle), s = std::sin(angle);
    const Vec3 relative = ray.origin - e.position;
    Ray local{{relative.x * c - relative.y * s, relative.x * s + relative.y * c, relative.z},
              {ray.direction.x * c - ray.direction.y * s, ray.direction.x * s + ray.direction.y * c, ray.direction.z},
              ray.maxDistance};
    auto hit = intersectAABB(local, e.localBounds);
    if (!hit) return std::nullopt;

    const Vec3 localPos = hit->position;
    std::uint8_t zone = ZoneAll;
    const Vec3 center = (e.localBounds.min + e.localBounds.max) * 0.5f;
    const Vec3 rel = localPos - center;
    const float hx = std::max(0.05f, (e.localBounds.max.x - e.localBounds.min.x) * 0.5f);
    const float hy = std::max(0.05f, (e.localBounds.max.y - e.localBounds.min.y) * 0.5f);
    const float hz = std::max(0.05f, (e.localBounds.max.z - e.localBounds.min.z) * 0.5f);

    if (rel.y > 0.35f * hy) {
        zone = ZoneFront;
    } else if (rel.y < -0.35f * hy) {
        zone = ZoneRear;
    } else if (rel.x < -0.35f * hx) {
        zone = ZoneLeft;
    } else if (rel.x > 0.35f * hx) {
        zone = ZoneRight;
    } else if (rel.z > 0.35f * hz) {
        zone = ZoneTop;
    } else if (rel.z < -0.35f * hz) {
        zone = ZoneBottom;
    } else {
        zone = ZoneAll;
    }

    const float invC = c, invS = -s;
    hit->position = e.position + Vec3{hit->position.x * invC - hit->position.y * invS,
                                      hit->position.x * invS + hit->position.y * invC, hit->position.z};
    hit->normal = {hit->normal.x * invC - hit->normal.y * invS,
                   hit->normal.x * invS + hit->normal.y * invC, hit->normal.z};
    return EntityHit{*hit, localPos, zone};
}

RayResult RayEngine::cast(const Ray& input, std::uint32_t mask, int interior, int world,
                          const Vec3* referencePos, int ignorePlayer) {
    const auto begin = std::chrono::steady_clock::now();
    RayResult result;
    lastCandidates_ = 0;
    ++stats_.raycasts;
    auto direction = input.direction.normalized();
    if (!direction || !input.origin.finite() || !std::isfinite(input.maxDistance) || input.maxDistance <= 0.0f) return result;
    const Ray ray{input.origin, *direction, input.maxDistance};
    float nearest = ray.maxDistance + 1.0f;
    float worldDistance = ray.maxDistance + 1.0f;
    int selectedPriority = 0;
    bool worldSelected = false;

    if ((mask & MaskWorld) != 0u) {
        if (auto hit = backend_->castRay(ray); hit && hit->distance < nearest) {
            nearest = hit->distance;
            worldDistance = hit->distance;
            worldSelected = true;
            result = {HitType::World, EntityType::None, -1, -1, -1, InvalidHandle,
                      hit->position, hit->normal, hit->distance, ZoneAll, {}};
        }
    }

    const Vec3 end = ray.origin + ray.direction * ray.maxDistance;
    const AABB query{{std::min(ray.origin.x, end.x), std::min(ray.origin.y, end.y), std::min(ray.origin.z, end.z)},
                     {std::max(ray.origin.x, end.x), std::max(ray.origin.y, end.y), std::max(ray.origin.z, end.z)}};
    const auto candidates = spatial_.query(query);
    lastCandidates_ = candidates.size();
    stats_.candidates += candidates.size();
    for (Handle handle : candidates) {
        const EntitySnapshot* entity = registry_.get(handle);
        if (!entity || !entity->enabled || (mask & maskFor(entity->key.type)) == 0u ||
            (entity->targetMask & maskFor(entity->key.type)) == 0u) continue;
        if (ignorePlayer >= 0 && entity->key.type == EntityType::Player && entity->key.id == ignorePlayer) continue;
        if (entity->interior >= 0 && interior >= 0 && entity->interior != interior) continue;
        if (entity->world >= 0 && world >= 0 && entity->world != world) continue;
        ++stats_.intersections;
        auto hit = intersectEntity(ray, *entity);
        if (!hit) continue;
        const float checkDistance = referencePos ? distance(*referencePos, hit->hit.position) : hit->hit.distance;
        if (checkDistance > entity->targetDistance) continue;
        const bool respectsWorld = (mask & MaskWorld) != 0u && (entity->targetMask & MaskWorld) != 0u;
        if (respectsWorld && hit->hit.distance >= worldDistance) continue;
        const bool bypassesSelectedWorld = worldSelected && !respectsWorld;
        const bool clearlyNearer = bypassesSelectedWorld || hit->hit.distance < nearest - 0.25f;
        const bool priorityWins = !worldSelected && std::abs(hit->hit.distance - nearest) <= 0.25f && entity->priority > selectedPriority;
        if (!clearlyNearer && !priorityWins) continue;
        nearest = hit->hit.distance;
        selectedPriority = entity->priority;
        worldSelected = false;
        result = {HitType::Entity, entity->key.type, entity->key.id, entity->model, -1, handle,
                  hit->hit.position, hit->hit.normal, hit->hit.distance, hit->zone, hit->localPosition};
    }
    const double elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
    stats_.totalMilliseconds += elapsed;
    stats_.maxMilliseconds = std::max(stats_.maxMilliseconds, elapsed);
    return result;
}

std::optional<Intersection> RayEngine::castTarget(const Ray& input, Handle target, float expansion) const {
    const EntitySnapshot* source = registry_.get(target);
    auto direction = input.direction.normalized();
    if (!source || !direction || input.maxDistance <= 0.0f || !std::isfinite(expansion)) return std::nullopt;
    EntitySnapshot entity = *source;
    const float e = std::max(0.0f, expansion);
    if (entity.capsule) entity.capsuleRadius += e;
    else {
        entity.localBounds.min = entity.localBounds.min - Vec3{e, e, e};
        entity.localBounds.max = entity.localBounds.max + Vec3{e, e, e};
    }
    auto hit = intersectEntity({input.origin, *direction, input.maxDistance}, entity);
    return hit ? std::optional<Intersection>(hit->hit) : std::nullopt;
}

} // namespace ox
