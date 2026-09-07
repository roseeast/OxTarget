#pragma once

#include "EntityRegistry.hpp"
#include "HandlePool.hpp"
#include "RayEngine.hpp"
#include "SpatialIndex.hpp"
#include "Types.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ox {

struct Target {
    EntityKey entity;
    void* owner{};
    float distance{4.0f};
    bool enabled{true};
    bool zoneFiltering{false};
    int priority{};
    int interior{-1};
    int world{-1};
    std::uint32_t rayMask{MaskAll};
    std::vector<Handle> options;
};

struct PlayerState {
    bool enabled{true};
    bool debug{};
    std::uint32_t updateRateMs{100};
    ViewSource viewSource{ViewSource::Auto};
    ViewSource lastViewSource{ViewSource::Facing};
    Handle current{InvalidHandle};
    float hysteresis{0.25f};
    float lastDistance{};
    std::size_t candidates{};
    double rayMilliseconds{};
    std::chrono::steady_clock::time_point lastUpdate{};
    std::uint8_t currentZone{0};
    Vec3 currentLocalPos{};
};

class ITargetEvents {
public:
    virtual ~ITargetEvents() = default;
    virtual bool checkTarget(int player, Handle target, void* owner) = 0;
    virtual bool checkOption(int player, Handle target, Handle option, void* owner) = 0;
    virtual void enter(int player, Handle target, void* owner) = 0;
    virtual void leave(int player, Handle target, void* owner) = 0;
    virtual void change(int player, Handle oldTarget, Handle newTarget, void* owner) = 0;
    virtual void select(int player, Handle target, Handle option, int data, void* owner) = 0;
    virtual void zoneChange(int player, Handle target, std::uint8_t oldZone, std::uint8_t newZone, void* owner) {
        (void)player; (void)target; (void)oldZone; (void)newZone; (void)owner;
    }
};

class TargetManager {
public:
    TargetManager(IEntityProvider& provider, ITargetEvents& events, std::unique_ptr<ICollisionBackend> backend);
    Handle create(EntityType type, int entity, int ownerPlayer, void* owner);
    Handle addPoint(const Vec3& position, float radius, void* owner);
    bool destroy(Handle handle);
    Target* get(Handle handle) { return targets_.get(handle); }
    const Target* get(Handle handle) const { return targets_.get(handle); }
    bool sync(Handle handle);
    Handle addOption(Handle target, std::string name, std::string icon, int data, std::uint8_t zone = 0);
    Handle addOption(Handle target, std::string name, int data) { return addOption(target, std::move(name), {}, data, 0); }
    bool removeOption(Handle target, Handle option);
    Option* getOption(Handle option) { return options_.get(option); }
    const Option* getOption(Handle option) const { return options_.get(option); }
    Handle optionAt(Handle target, std::size_t index) const;
    std::size_t optionCount(Handle target) const;
    std::uint8_t getOptionZone(Handle option) const;
    bool setOptionZone(Handle option, std::uint8_t zone);
    bool setZoneFiltering(Handle target, bool enabled);
    bool isZoneFiltering(Handle target) const;
    std::uint8_t getPlayerZone(int player) const;
    void tick();
    void updatePlayer(int player, bool force = false);
    void disconnectPlayer(int player);
    void unloadOwner(void* owner);
    bool select(int player, Handle option);
    bool selectIndex(int player, std::size_t index);
    bool selectTargetOption(int player, Handle target, std::size_t index);
    PlayerState& playerState(int player) { return players_[player]; }
    const PlayerState* findPlayerState(int player) const;
    bool getView(int player, Vec3& origin, Vec3& direction, ViewSource& used);
    RayResult castCamera(int player, float distance, std::uint32_t mask);
    RayResult castRay(const Ray& ray, std::uint32_t mask);
    RayEngine& rayEngine() { return rayEngine_; }
    HandlePool<Target>& targets() { return targets_; }
private:
    bool refreshTarget(Handle handle, Target& target);
    void recalculateMaxDistance();
    bool targetAllowedForPlayer(int player, Handle handle, const Target& target, const RayResult& hit, const Vec3* playerPos = nullptr);
    IEntityProvider& provider_;
    ITargetEvents& events_;
    EntityRegistry registry_;
    SpatialIndex spatial_;
    RayEngine rayEngine_;
    HandlePool<Target> targets_;
    HandlePool<Option> options_;
    std::unordered_map<int, PlayerState> players_;
    std::unordered_set<Handle> destroying_;
    std::chrono::steady_clock::time_point lastSpatialRefresh_{};
    std::chrono::steady_clock::time_point lastPlayerScan_{};
    float maxTargetDistance_{};
};

} // namespace ox
