#pragma once

#include "Math/AABB.hpp"
#include "Math/Capsule.hpp"
#include "Types.hpp"
#include <optional>
#include <unordered_map>

namespace ox {

struct EntitySnapshot {
    EntityKey key;
    Vec3 position;
    Vec3 rotation;
    AABB localBounds;
    int model{-1};
    int interior{-1};
    int world{-1};
    int priority{};
    bool enabled{true};
    float targetDistance{4.0f};
    std::uint32_t targetMask{MaskAll};
    bool capsule{};
    float capsuleRadius{0.35f};
    float capsuleHeight{1.7f};
};

class IEntityProvider {
public:
    virtual ~IEntityProvider() = default;
    virtual std::optional<EntitySnapshot> snapshot(const EntityKey& key) = 0;
    virtual bool playerConnected(int player) = 0;
    virtual std::optional<Vec3> playerPosition(int player) = 0;
    virtual int playerInterior(int player) = 0;
    virtual int playerWorld(int player) = 0;
    virtual bool playerView(int player, ViewSource source, Vec3& origin, Vec3& direction, ViewSource& used) = 0;
};

class EntityRegistry {
public:
    explicit EntityRegistry(IEntityProvider& provider) : provider_(provider) {}
    std::optional<EntitySnapshot> refresh(Handle target, const EntityKey& key);
    void set(Handle target, const EntitySnapshot& snapshot);
    const EntitySnapshot* get(Handle target) const;
    void remove(Handle target);
    AABB worldBounds(const EntitySnapshot& entity) const;
private:
    IEntityProvider& provider_;
    std::unordered_map<Handle, EntitySnapshot> snapshots_;
};

} // namespace ox
