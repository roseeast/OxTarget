#include "core/HandlePool.hpp"
#include "core/Intersection.hpp"
#include "core/SpatialIndex.hpp"
#include "core/TargetManager.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}
bool near(float a, float b, float eps = 0.001f) { return std::abs(a - b) < eps; }

struct Provider final : ox::IEntityProvider {
    std::unordered_map<int, ox::EntitySnapshot> entities;
    ox::Vec3 viewOrigin{0,0,0}; ox::Vec3 viewDirection{1,0,0};
    ox::Vec3 playerPos{0,0,0};
    bool connected{true}; int interior{}; int world{};
    std::optional<ox::EntitySnapshot> snapshot(const ox::EntityKey& key) override {
        auto it = entities.find(key.id); return it == entities.end() ? std::nullopt : std::optional<ox::EntitySnapshot>(it->second);
    }
    bool playerConnected(int player) override { return player == 0 && connected; }
    std::optional<ox::Vec3> playerPosition(int player) override { return player == 0 ? std::optional<ox::Vec3>(playerPos) : std::nullopt; }
    int playerInterior(int) override { return interior; }
    int playerWorld(int) override { return world; }
    bool playerView(int player, ox::ViewSource, ox::Vec3& o, ox::Vec3& d, ox::ViewSource& used) override {
        if (player != 0 || !connected) return false; o=viewOrigin; d=viewDirection; used=ox::ViewSource::Camera; return true;
    }
};

struct Events final : ox::ITargetEvents {
    int enters{}, leaves{}, changes{}, selects{}; bool allowTarget{true}, allowOption{true};
    bool checkTarget(int, ox::Handle, void*) override { return allowTarget; }
    bool checkOption(int, ox::Handle, ox::Handle, void*) override { return allowOption; }
    void enter(int, ox::Handle, void*) override { ++enters; }
    void leave(int, ox::Handle, void*) override { ++leaves; }
    void change(int, ox::Handle, ox::Handle, void*) override { ++changes; }
    void select(int, ox::Handle, ox::Handle, int, void*) override { ++selects; }
};

struct WallBackend final : ox::ICollisionBackend {
    std::optional<ox::Intersection> castRay(const ox::Ray& ray) override {
        const float t=4.0f; return ox::Intersection{t,ray.origin+ray.direction*t,{-1,0,0}};
    }
    std::string_view name() const override { return "test wall"; }
};

ox::EntitySnapshot objectAt(int id, float x) {
    ox::EntitySnapshot e; e.key={ox::EntityType::Object,id,-1}; e.position={x,0,0};
    e.localBounds={{-0.5f,-0.5f,-0.5f},{0.5f,0.5f,0.5f}}; return e;
}

ox::EntitySnapshot vehicleAt(int id, float x, float y) {
    ox::EntitySnapshot e; e.key={ox::EntityType::Vehicle,id,-1}; e.position={x,y,0};
    e.localBounds={{-1.0f,-2.5f,-1.0f},{1.0f,2.5f,1.0f}}; return e;
}
}

int main() {
    using namespace ox;
    Vec3 v{3,4,0}; require(near(v.length(),5), "Vec3 length");
    auto n=v.normalized(); require(n && near(n->length(),1), "Vec3 normalization");
    require(!Vec3{}.normalized(), "zero vector rejected");

    Ray ray{{0,0,0},{1,0,0},20};
    auto box=intersectAABB(ray,{{4,-1,-1},{6,1,1}}); require(box && near(box->distance,4), "ray/AABB");
    auto sphere=intersectSphere(ray,{5,0,0},1); require(sphere && near(sphere->distance,4), "ray/sphere");
    auto capsule=intersectCapsule(ray,{{5,0,-1},{5,0,1},0.5f}); require(capsule && near(capsule->distance,4.5f), "ray/capsule");

    HandlePool<int> pool; Handle first=pool.emplace(7); require(first && *pool.get(first)==7,"handle create");
    require(pool.erase(first) && !pool.get(first),"invalidated handle"); Handle second=pool.emplace(9);
    require(second != first && !pool.get(first) && *pool.get(second)==9,"generation on reuse");

    SpatialIndex grid(10); grid.upsert(1,{{1,1,1},{2,2,2}}); grid.upsert(2,{{21,1,1},{22,2,2}});
    auto found=grid.query({{0,0,0},{9,9,9}}); require(found.size()==1 && found[0]==1,"spatial query");
    grid.remove(1); require(grid.query({{0,0,0},{9,9,9}}).empty(),"spatial remove");

    Provider provider; Events events;
    provider.entities.emplace(10, objectAt(10,3));
    provider.entities.emplace(11, objectAt(11,6));
    TargetManager manager(provider,events,std::make_unique<FallbackCollisionBackend>());
    int owner{};
    Handle nearTarget=manager.create(EntityType::Object,10,-1,&owner);
    Handle farTarget=manager.create(EntityType::Object,11,-1,&owner);
    require(nearTarget && farTarget,"create targets");
    manager.get(nearTarget)->distance=10; manager.get(farTarget)->distance=10; manager.sync(nearTarget); manager.sync(farTarget);
    Handle option=manager.addOption(nearTarget,"Use",42); require(option,"add option");
    manager.updatePlayer(0,true);
    require(manager.playerState(0).current==nearTarget,"nearest intersection");
    require(events.enters==1 && events.changes==1,"enter/change callbacks");
    require(manager.select(0,option) && events.selects==1,"selection revalidation");
    require(manager.selectTargetOption(0, nearTarget, 0) && events.selects==2,"selectTargetOption direct execution");

    provider.viewDirection={0,1,0}; manager.playerState(0).hysteresis=0; manager.updatePlayer(0,true);
    require(manager.playerState(0).current==InvalidHandle && events.leaves==1,"leave callback");
    require(!manager.select(0,option),"selection rejects stale target");

    provider.viewDirection={1,0,0}; manager.get(nearTarget)->enabled=false; manager.sync(nearTarget); manager.updatePlayer(0,true);
    require(manager.playerState(0).current==farTarget,"disabled target skipped");
    manager.get(nearTarget)->enabled=true; manager.get(nearTarget)->distance=1; manager.sync(nearTarget); manager.updatePlayer(0,true);
    require(manager.playerState(0).current==farTarget,"distance checked");

    provider.entities.erase(11); manager.tick(); require(!manager.get(farTarget),"destroyed entity target removed");
    require((manager.castRay(ray,MaskVehicle).hitType==HitType::None),"ray mask");

    Provider priorityProvider; Events priorityEvents;
    priorityProvider.entities.emplace(20, objectAt(20,3.00f)); priorityProvider.entities.emplace(21,objectAt(21,3.10f));
    TargetManager priorityManager(priorityProvider,priorityEvents,std::make_unique<FallbackCollisionBackend>());
    Handle low=priorityManager.create(EntityType::Object,20,-1,&owner); Handle high=priorityManager.create(EntityType::Object,21,-1,&owner);
    priorityManager.get(low)->priority=0; priorityManager.get(high)->priority=10; priorityManager.sync(low); priorityManager.sync(high);
    priorityManager.tick(); priorityManager.updatePlayer(0,true);
    require(priorityManager.playerState(0).current==high,"priority wins near-equal hits");

    Provider wallProvider; Events wallEvents; wallProvider.entities.emplace(30,objectAt(30,6));
    TargetManager wallManager(wallProvider,wallEvents,std::make_unique<WallBackend>());
    Handle behindWall=wallManager.create(EntityType::Object,30,-1,&owner);
    wallManager.get(behindWall)->distance=10; wallManager.sync(behindWall); wallManager.updatePlayer(0,true);
    require(wallManager.playerState(0).current==InvalidHandle,"world occludes target");
    wallManager.get(behindWall)->rayMask=MaskObject; wallManager.sync(behindWall); wallManager.updatePlayer(0,true);
    require(wallManager.playerState(0).current==behindWall,"per-target mask can disable world occlusion");

    // Test 3rd-person camera distance: camera is 3.8m behind player
    Provider camProvider; Events camEvents;
    camProvider.playerPos = {0, 0, 0};
    camProvider.viewOrigin = {-3.8f, 0, 0}; // 3.8m behind player
    camProvider.viewDirection = {1, 0, 0};
    camProvider.entities.emplace(40, objectAt(40, 2.5f)); // Object is 2.5m in front of player
    TargetManager camManager(camProvider, camEvents, std::make_unique<FallbackCollisionBackend>());
    Handle camTarget = camManager.create(EntityType::Object, 40, -1, &owner);
    camManager.get(camTarget)->distance = 3.0f; // Player distance is 2.5m <= 3.0m (camera distance is 6.3m > 3.0m)
    camManager.sync(camTarget);
    camManager.updatePlayer(0, true);
    require(camManager.playerState(0).current == camTarget, "3rd-person camera distance measured from player character");

    // Test Vehicle Zones (Hood vs Trunk vs Doors) & Zone Filtering toggle
    Provider vehProvider; Events vehEvents;
    vehProvider.entities.emplace(50, vehicleAt(50, 0.0f, 10.0f)); // Vehicle at (0, 10, 0)
    TargetManager vehManager(vehProvider, vehEvents, std::make_unique<FallbackCollisionBackend>());
    Handle vehTarget = vehManager.create(EntityType::Vehicle, 50, -1, &owner);
    vehManager.get(vehTarget)->distance = 5.0f;
    vehManager.sync(vehTarget);

    Handle optAll = vehManager.addOption(vehTarget, "Unlock", "", 10, ZoneAll);
    Handle optHood = vehManager.addOption(vehTarget, "Open Hood", "", 11, ZoneHood);
    Handle optTrunk = vehManager.addOption(vehTarget, "Open Trunk", "", 12, ZoneTrunk);

    // 1. By default, zoneFiltering is false: all options are selectable from anywhere
    require(!vehManager.isZoneFiltering(vehTarget), "zone filtering default is false");
    vehProvider.playerPos = {0.0f, 14.0f, 0.0f}; // Standing in front of vehicle
    vehProvider.viewOrigin = {0.0f, 16.0f, 0.0f};
    vehProvider.viewDirection = {0.0f, -1.0f, 0.0f}; // Looking back towards vehicle front (+Y local)
    vehManager.updatePlayer(0, true);
    require(vehManager.playerState(0).current == vehTarget, "vehicle targeted from front");
    require(vehManager.getPlayerZone(0) == ZoneHood, "vehicle hood zone identified");
    require(vehManager.select(0, optHood), "hood option selectable at hood");
    require(vehManager.select(0, optAll), "general option selectable at hood");
    require(vehManager.select(0, optTrunk), "trunk option selectable when zone filtering is false");

    // 2. Enable zone filtering: strict zone matching applies
    vehManager.setZoneFiltering(vehTarget, true);
    require(vehManager.isZoneFiltering(vehTarget), "zone filtering enabled");
    require(vehManager.select(0, optHood), "hood option selectable at hood with filtering");
    require(vehManager.select(0, optAll), "general option selectable at hood with filtering");
    require(!vehManager.select(0, optTrunk), "trunk option rejected at hood with filtering");

    // 3. Aiming at vehicle rear / trunk from behind
    vehProvider.playerPos = {0.0f, 6.0f, 0.0f}; // Standing behind vehicle
    vehProvider.viewOrigin = {0.0f, 4.0f, 0.0f};
    vehProvider.viewDirection = {0.0f, 1.0f, 0.0f}; // Looking forward towards vehicle rear (-Y local)
    vehManager.updatePlayer(0, true);
    require(vehManager.playerState(0).current == vehTarget, "vehicle targeted from rear");
    require(vehManager.getPlayerZone(0) == ZoneTrunk, "vehicle trunk zone identified");
    require(vehManager.select(0, optTrunk), "trunk option selectable at trunk with filtering");
    require(vehManager.select(0, optAll), "general option selectable at trunk with filtering");
    require(!vehManager.select(0, optHood), "hood option rejected at trunk with filtering");

    // 4. Universal Object Zones (Object at 20, 0, 0)
    Provider objProvider; Events objEvents;
    objProvider.entities.emplace(60, objectAt(60, 20.0f)); // Object at (20, 0, 0)
    TargetManager objManager(objProvider, objEvents, std::make_unique<FallbackCollisionBackend>());
    Handle objTarget = objManager.create(EntityType::Object, 60, -1, &owner);
    objManager.get(objTarget)->distance = 5.0f;
    objManager.setZoneFiltering(objTarget, true);
    objManager.sync(objTarget);

    Handle objFront = objManager.addOption(objTarget, "Front Switch", "", 20, ZoneFront);
    Handle objRear = objManager.addOption(objTarget, "Rear Cable", "", 21, ZoneRear);
    Handle objTop = objManager.addOption(objTarget, "Top Hatch", "", 22, ZoneTop);

    // Aim from +Y looking along -Y towards object (20, 0, 0) -> Front (+Y)
    objProvider.playerPos = {20.0f, 2.0f, 0.0f};
    objProvider.viewOrigin = {20.0f, 3.0f, 0.0f};
    objProvider.viewDirection = {0.0f, -1.0f, 0.0f};
    objManager.updatePlayer(0, true);
    require(objManager.playerState(0).current == objTarget, "object targeted at front");
    require(objManager.getPlayerZone(0) == ZoneFront, "object front zone identified");
    require(objManager.select(0, objFront), "object front option selectable");
    require(!objManager.select(0, objRear), "object rear option rejected at front");

    // Aim from top (+Z) looking down -> Top (+Z)
    objProvider.playerPos = {20.0f, 0.0f, 2.0f};
    objProvider.viewOrigin = {20.0f, 0.0f, 3.0f};
    objProvider.viewDirection = {0.0f, 0.0f, -1.0f};
    objManager.updatePlayer(0, true);
    require(objManager.getPlayerZone(0) == ZoneTop, "object top zone identified");
    require(objManager.select(0, objTop), "object top option selectable");
    require(!objManager.select(0, objFront), "object front option rejected at top");

    std::cout << "All OxTarget tests passed\n";
    return 0;
}
