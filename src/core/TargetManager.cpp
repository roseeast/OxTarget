#include "TargetManager.hpp"
#include <algorithm>
#include <cmath>

namespace ox {

TargetManager::TargetManager(IEntityProvider& provider, ITargetEvents& events, std::unique_ptr<ICollisionBackend> backend)
    : provider_(provider), events_(events), registry_(provider), spatial_(16.0f),
      rayEngine_(registry_, spatial_, std::move(backend)) {}

Handle TargetManager::create(EntityType type, int entity, int ownerPlayer, void* owner) {
    if (type == EntityType::None || type == EntityType::Point || entity < 0) return InvalidHandle;
    Target target;
    target.entity = {type, entity, ownerPlayer};
    target.owner = owner;
    const Handle handle = targets_.emplace(std::move(target));
    Target* stored = targets_.get(handle);
    if (!stored || !refreshTarget(handle, *stored)) { targets_.erase(handle); return InvalidHandle; }
    maxTargetDistance_ = std::max(maxTargetDistance_, stored->distance);
    return handle;
}

Handle TargetManager::addPoint(const Vec3& position, float radius, void* owner) {
    if (!position.finite() || !std::isfinite(radius) || radius <= 0.0f || radius > 1000.0f) return InvalidHandle;
    Target target;
    target.entity = {EntityType::Point, -1, -1};
    target.owner = owner;
    target.distance = std::max(4.0f, radius);
    const Handle handle = targets_.emplace(std::move(target));
    EntitySnapshot snapshot;
    snapshot.key = {EntityType::Point, static_cast<int>(handle), -1};
    snapshot.position = position;
    snapshot.localBounds = {{-radius, -radius, -radius}, {radius, radius, radius}};
    registry_.set(handle, snapshot);
    spatial_.upsert(handle, registry_.worldBounds(snapshot));
    maxTargetDistance_ = std::max(maxTargetDistance_, targets_.get(handle)->distance);
    return handle;
}

bool TargetManager::destroy(Handle handle) {
    if (destroying_.find(handle) != destroying_.end()) return false;
    Target* target = targets_.get(handle);
    if (!target) return false;
    destroying_.insert(handle);
    const auto ownedOptions = target->options;
    void* owner = target->owner;
    for (Handle option : ownedOptions) options_.erase(option);
    std::vector<int> affectedPlayers;
    for (auto& pair : players_) {
        if (pair.second.current == handle) {
            pair.second.current = InvalidHandle;
            affectedPlayers.push_back(pair.first);
        }
    }
    for (int player : affectedPlayers) {
        events_.leave(player, handle, owner);
        events_.change(player, handle, InvalidHandle, owner);
    }
    registry_.remove(handle);
    spatial_.remove(handle);
    const bool erased = targets_.erase(handle);
    destroying_.erase(handle);
    recalculateMaxDistance();
    return erased;
}

Handle TargetManager::addOption(Handle targetHandle, std::string name, std::string icon, int data, std::uint8_t zone) {
    Target* target = targets_.get(targetHandle);
    if (!target || name.empty() || name.size() > 63) return InvalidHandle;
    const Handle option = options_.emplace(Option{targetHandle, std::move(name), std::move(icon), data, true, zone});
    if (option != InvalidHandle) target->options.push_back(option);
    return option;
}

bool TargetManager::removeOption(Handle targetHandle, Handle option) {
    Target* target = targets_.get(targetHandle);
    Option* value = options_.get(option);
    if (!target || !value || value->target != targetHandle) return false;
    target->options.erase(std::remove(target->options.begin(), target->options.end(), option), target->options.end());
    return options_.erase(option);
}

Handle TargetManager::optionAt(Handle target, std::size_t index) const {
    const Target* value = targets_.get(target);
    return !value || index >= value->options.size() ? InvalidHandle : value->options[index];
}

std::size_t TargetManager::optionCount(Handle target) const {
    const Target* value = targets_.get(target);
    return value ? value->options.size() : 0;
}

std::uint8_t TargetManager::getOptionZone(Handle optionHandle) const {
    const Option* option = options_.get(optionHandle);
    return option ? option->zone : 0;
}

bool TargetManager::setOptionZone(Handle optionHandle, std::uint8_t zone) {
    Option* option = options_.get(optionHandle);
    if (!option) return false;
    option->zone = zone;
    return true;
}

std::uint8_t TargetManager::getPlayerZone(int player) const {
    const PlayerState* state = findPlayerState(player);
    return state ? state->currentZone : 0;
}

bool TargetManager::setZoneFiltering(Handle handle, bool enabled) {
    Target* target = targets_.get(handle);
    if (!target) return false;
    target->zoneFiltering = enabled;
    return true;
}

bool TargetManager::isZoneFiltering(Handle handle) const {
    const Target* target = targets_.get(handle);
    return target ? target->zoneFiltering : false;
}

bool TargetManager::refreshTarget(Handle handle, Target& target) {
    std::optional<EntitySnapshot> snapshot;
    if (target.entity.type == EntityType::Point) {
        const EntitySnapshot* current = registry_.get(handle);
        if (current) snapshot = *current;
    } else snapshot = registry_.refresh(handle, target.entity);
    if (!snapshot) { spatial_.remove(handle); return false; }
    if (target.entity.type == EntityType::Point) {
        snapshot->interior = target.interior;
        snapshot->world = target.world;
    } else {
        if (target.interior >= 0) snapshot->interior = target.interior;
        if (target.world >= 0) snapshot->world = target.world;
    }
    snapshot->priority = target.priority;
    snapshot->enabled = target.enabled;
    snapshot->targetDistance = target.distance;
    snapshot->targetMask = target.rayMask;
    registry_.set(handle, *snapshot);
    spatial_.upsert(handle, registry_.worldBounds(*snapshot));
    return true;
}

bool TargetManager::sync(Handle handle) {
    Target* target = targets_.get(handle);
    const bool result = target && refreshTarget(handle, *target);
    recalculateMaxDistance();
    return result;
}

void TargetManager::recalculateMaxDistance() {
    maxTargetDistance_ = 0.0f;
    targets_.forEach([&](Handle, Target& target) { if (target.enabled) maxTargetDistance_ = std::max(maxTargetDistance_, target.distance); });
    maxTargetDistance_ = std::min(maxTargetDistance_, 1000.0f);
}

void TargetManager::tick() {
    const auto now = std::chrono::steady_clock::now();
    if (lastSpatialRefresh_.time_since_epoch().count() == 0 || now - lastSpatialRefresh_ >= std::chrono::milliseconds(50)) {
        std::vector<Handle> invalid;
        targets_.forEach([&](Handle handle, Target& target) { if (!refreshTarget(handle, target)) invalid.push_back(handle); });
        for (Handle handle : invalid) destroy(handle);
        lastSpatialRefresh_ = now;
    }
    if (lastPlayerScan_.time_since_epoch().count() != 0 && now - lastPlayerScan_ < std::chrono::milliseconds(50)) return;
    lastPlayerScan_ = now;
    if (players_.empty()) return;
    std::vector<int> toDisconnect;
    for (auto& pair : players_) {
        const int player = pair.first;
        if (provider_.playerConnected(player)) {
            if (pair.second.enabled) updatePlayer(player);
        } else {
            toDisconnect.push_back(player);
        }
    }
    for (int player : toDisconnect) {
        disconnectPlayer(player);
    }
}

bool TargetManager::getView(int player, Vec3& origin, Vec3& direction, ViewSource& used) {
    PlayerState& state = players_[player];
    if (!provider_.playerConnected(player)) return false;
    if (!provider_.playerView(player, state.viewSource, origin, direction, used)) return false;
    auto normalized = direction.normalized();
    if (!normalized || !origin.finite()) return false;
    direction = *normalized;
    state.lastViewSource = used;
    return true;
}

RayResult TargetManager::castCamera(int player, float maxDistance, std::uint32_t mask) {
    Vec3 origin, direction; ViewSource used{};
    if (!std::isfinite(maxDistance) || maxDistance <= 0.0f || maxDistance > 1000.0f || !getView(player, origin, direction, used)) return {};
    const auto playerPosOpt = provider_.playerPosition(player);
    const Vec3* playerPos = playerPosOpt ? &*playerPosOpt : nullptr;
    const float camToPlayer = playerPos ? distance(origin, *playerPos) : 0.0f;
    const float rayLength = std::min(1000.0f, maxDistance + camToPlayer);
    return rayEngine_.cast({origin, direction, rayLength}, mask, provider_.playerInterior(player), provider_.playerWorld(player),
                           playerPos, player);
}

RayResult TargetManager::castRay(const Ray& ray, std::uint32_t mask) { return rayEngine_.cast(ray, mask, -1, -1); }

bool TargetManager::targetAllowedForPlayer(int player, Handle handle, const Target& target, const RayResult& hit, const Vec3* playerPos) {
    if (!target.enabled) return false;
    const float checkDist = playerPos ? distance(*playerPos, hit.position) : hit.distance;
    if (checkDist > target.distance) return false;
    if ((target.rayMask & maskFor(target.entity.type)) == 0u) return false;
    if (target.interior >= 0 && target.interior != provider_.playerInterior(player)) return false;
    if (target.world >= 0 && target.world != provider_.playerWorld(player)) return false;
    return events_.checkTarget(player, handle, target.owner);
}

void TargetManager::updatePlayer(int player, bool force) {
    PlayerState& state = players_[player];
    if (!state.enabled || !provider_.playerConnected(player)) return;
    const auto now = std::chrono::steady_clock::now();
    if (!force && state.lastUpdate.time_since_epoch().count() != 0 &&
        now - state.lastUpdate < std::chrono::milliseconds(state.updateRateMs)) return;
    state.lastUpdate = now;
    const auto begin = std::chrono::steady_clock::now();
    Vec3 origin, direction; ViewSource used{};
    Handle selected = InvalidHandle;
    float selectedDistance = 0.0f;
    std::uint8_t selectedZone = ZoneAll;
    Vec3 selectedLocalPos{};

    const auto playerPosOpt = provider_.playerPosition(player);
    const Vec3* playerPos = playerPosOpt ? &*playerPosOpt : nullptr;

    if (getView(player, origin, direction, used)) {
        const float maxDistance = maxTargetDistance_;
        if (maxDistance > 0.0f) {
            const float camToPlayer = playerPos ? distance(origin, *playerPos) : 0.0f;
            const float rayLength = std::min(100.0f, camToPlayer + maxDistance + 3.0f);
            RayResult hit = rayEngine_.cast({origin, direction, rayLength}, MaskAll,
                                            provider_.playerInterior(player), provider_.playerWorld(player),
                                            playerPos, player);
            state.candidates = rayEngine_.lastCandidateCount();
            if (hit.hitType == HitType::Entity) {
                if (const Target* target = targets_.get(hit.target); target && targetAllowedForPlayer(player, hit.target, *target, hit, playerPos) && targets_.get(hit.target)) {
                    selected = hit.target;
                    selectedDistance = playerPos ? distance(*playerPos, hit.position) : hit.distance;
                    selectedZone = hit.zone;
                    selectedLocalPos = hit.localPosition;
                }
            }
            if (selected == InvalidHandle && state.current != InvalidHandle) {
                const Target* current = targets_.get(state.current);
                if (current && current->enabled) {
                    const float releaseRayLen = std::min(100.0f, camToPlayer + current->distance + 3.0f);
                    auto release = rayEngine_.castTarget({origin, direction, releaseRayLen}, state.current, state.hysteresis);
                    if (release) {
                        RayResult synthetic{HitType::Entity, current->entity.type, current->entity.id, -1, -1,
                                            state.current, release->position, release->normal, release->distance,
                                            state.currentZone, state.currentLocalPos};
                        if (targetAllowedForPlayer(player, state.current, *current, synthetic, playerPos)) {
                            selected = state.current;
                            selectedDistance = playerPos ? distance(*playerPos, release->position) : release->distance;
                            selectedZone = state.currentZone;
                            selectedLocalPos = state.currentLocalPos;
                        }
                    }
                }
            }
        }
    }
    state.rayMilliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
    state.lastDistance = selectedDistance;

    const std::uint8_t oldZone = state.currentZone;
    state.currentZone = selectedZone;
    state.currentLocalPos = selectedLocalPos;

    if (selected == state.current) {
        if (selected != InvalidHandle && selectedZone != oldZone) {
            if (const Target* target = targets_.get(selected)) {
                events_.zoneChange(player, selected, oldZone, selectedZone, target->owner);
            }
        }
        return;
    }
    const Handle old = state.current;
    void* owner = nullptr;
    state.current = selected;
    if (const Target* oldTarget = targets_.get(old)) { owner = oldTarget->owner; events_.leave(player, old, owner); }
    if (state.current == selected) {
        if (const Target* newTarget = targets_.get(selected)) { owner = newTarget->owner; events_.enter(player, selected, owner); }
    }
    if (state.current == selected) events_.change(player, old, selected, owner);
}

bool TargetManager::select(int player, Handle optionHandle) {
    if (!provider_.playerConnected(player)) return false;
    const PlayerState* state = findPlayerState(player);
    const Option* option = options_.get(optionHandle);
    const Target* target = option ? targets_.get(option->target) : nullptr;
    if (!state || !option || !target || !option->enabled || state->current != option->target) return false;
    if (target->zoneFiltering && option->zone != ZoneAll) {
        if (option->zone == ZoneSides || option->zone == ZoneDoors) {
            if (state->currentZone != ZoneLeft && state->currentZone != ZoneRight &&
                state->currentZone != ZoneDoorDriver && state->currentZone != ZoneDoorPassenger) return false;
        } else if (option->zone != state->currentZone) {
            return false;
        }
    }
    const Handle targetHandle = option->target;
    if (!events_.checkOption(player, targetHandle, optionHandle, target->owner)) return false;
    option = options_.get(optionHandle);
    target = option ? targets_.get(option->target) : nullptr;
    state = findPlayerState(player);
    if (!state || !option || !target || !option->enabled || state->current != targetHandle) return false;
    events_.select(player, targetHandle, optionHandle, option->data, target->owner);
    return true;
}

bool TargetManager::selectIndex(int player, std::size_t index) {
    const PlayerState* state = findPlayerState(player);
    if (!state || state->current == InvalidHandle) return false;
    const Handle option = optionAt(state->current, index);
    return option != InvalidHandle && select(player, option);
}

bool TargetManager::selectTargetOption(int player, Handle targetHandle, std::size_t index) {
    if (!provider_.playerConnected(player)) return false;
    const Target* target = targets_.get(targetHandle);
    if (!target || !target->enabled) return false;
    const Handle optionHandle = optionAt(targetHandle, index);
    if (optionHandle == InvalidHandle) return false;
    const Option* option = options_.get(optionHandle);
    if (!option || !option->enabled) return false;
    if (target->zoneFiltering && option->zone != ZoneAll) {
        const PlayerState* state = findPlayerState(player);
        if (!state) return false;
        if (option->zone == ZoneSides || option->zone == ZoneDoors) {
            if (state->currentZone != ZoneLeft && state->currentZone != ZoneRight &&
                state->currentZone != ZoneDoorDriver && state->currentZone != ZoneDoorPassenger) return false;
        } else if (option->zone != state->currentZone) {
            return false;
        }
    }
    if (!events_.checkOption(player, targetHandle, optionHandle, target->owner)) return false;
    option = options_.get(optionHandle);
    target = targets_.get(targetHandle);
    if (!option || !target || !option->enabled) return false;
    events_.select(player, targetHandle, optionHandle, option->data, target->owner);
    return true;
}

void TargetManager::disconnectPlayer(int player) {
    const auto found = players_.find(player);
    if (found == players_.end()) return;
    if (found->second.current != InvalidHandle) {
        if (const Target* target = targets_.get(found->second.current)) events_.leave(player, found->second.current, target->owner);
    }
    players_.erase(found);
}

void TargetManager::unloadOwner(void* owner) {
    std::vector<Handle> remove;
    targets_.forEach([&](Handle handle, Target& target) { if (target.owner == owner) remove.push_back(handle); });
    for (Handle handle : remove) destroy(handle);
}

const PlayerState* TargetManager::findPlayerState(int player) const {
    const auto found = players_.find(player);
    return found == players_.end() ? nullptr : &found->second;
}

} // namespace ox
