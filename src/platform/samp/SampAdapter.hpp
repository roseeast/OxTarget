#pragma once

#include "core/EntityRegistry.hpp"
#include "core/TargetManager.hpp"
#include <amx/amx.h>
#include <optional>
#include <string>
#include <vector>

namespace ox {

class SampAdapter final : public IEntityProvider, public ITargetEvents {
public:
    void addAmx(AMX* amx);
    void removeAmx(AMX* amx);
    bool hasAmx(AMX* amx) const;
    AMX* primaryAmx() const;

    std::optional<EntitySnapshot> snapshot(const EntityKey& key) override;
    bool playerConnected(int player) override;
    std::optional<Vec3> playerPosition(int player) override;
    int playerInterior(int player) override;
    int playerWorld(int player) override;
    bool playerView(int player, ViewSource source, Vec3& origin, Vec3& direction, ViewSource& used) override;
    std::optional<Intersection> worldRay(const Ray& ray);

    bool checkTarget(int player, Handle target, void* owner) override;
    bool checkOption(int player, Handle target, Handle option, void* owner) override;
    void enter(int player, Handle target, void* owner) override;
    void leave(int player, Handle target, void* owner) override;
    void change(int player, Handle oldTarget, Handle newTarget, void* owner) override;
    void select(int player, Handle target, Handle option, int data, void* owner) override;
    void zoneChange(int player, Handle target, std::uint8_t oldZone, std::uint8_t newZone, void* owner) override;

private:
    bool call(AMX* amx, const char* name, const std::vector<cell>& args, cell* result = nullptr) const;
    bool callRefs(AMX* amx, const char* name, const std::vector<cell>& args,
                  std::size_t refCount, std::vector<cell>& refs, cell* result = nullptr) const;
    bool callPrimary(const char* name, const std::vector<cell>& args, cell* result = nullptr) const;
    void broadcast(const char* name, const std::vector<cell>& args) const;
    std::vector<AMX*> amx_;
};

} // namespace ox
