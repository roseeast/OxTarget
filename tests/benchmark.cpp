#include "core/TargetManager.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <unordered_map>

namespace {
struct Provider final : ox::IEntityProvider {
    std::unordered_map<int, ox::EntitySnapshot> data;
    std::optional<ox::EntitySnapshot> snapshot(const ox::EntityKey& key) override {
        auto it=data.find(key.id); return it==data.end()?std::nullopt:std::optional<ox::EntitySnapshot>(it->second);
    }
    bool playerConnected(int) override { return false; }
    std::optional<ox::Vec3> playerPosition(int) override { return std::nullopt; }
    int playerInterior(int) override { return 0; }
    int playerWorld(int) override { return 0; }
    bool playerView(int,ox::ViewSource,ox::Vec3&,ox::Vec3&,ox::ViewSource&) override { return false; }
};
struct Events final : ox::ITargetEvents {
    bool checkTarget(int,ox::Handle,void*) override{return true;} bool checkOption(int,ox::Handle,ox::Handle,void*) override{return true;}
    void enter(int,ox::Handle,void*) override{} void leave(int,ox::Handle,void*) override{}
    void change(int,ox::Handle,ox::Handle,void*) override{} void select(int,ox::Handle,ox::Handle,int,void*) override{}
};
}

int main() {
    constexpr int side=100, targetCount=side*side, casts=100000;
    Provider provider; Events events;
    for(int y=0;y<side;++y) for(int x=0;x<side;++x) {
        int id=y*side+x; ox::EntitySnapshot s; s.key={ox::EntityType::Object,id,-1};
        s.position={static_cast<float>(x*4),static_cast<float>(y*4),0};
        s.localBounds={{-0.5f,-0.5f,-0.5f},{0.5f,0.5f,0.5f}};
        provider.data.emplace(id,s);
    }
    ox::TargetManager manager(provider,events,std::make_unique<ox::FallbackCollisionBackend>());
    int owner{}; for(int id=0;id<targetCount;++id) manager.create(ox::EntityType::Object,id,-1,&owner);
    const auto begin=std::chrono::steady_clock::now();
    for(int i=0;i<casts;++i) {
        float y=static_cast<float>((i%side)*4);
        manager.castRay({{-2,y,0},{1,0,0},500},ox::MaskObject);
    }
    const double elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
    const auto& stats=manager.rayEngine().stats();
    std::cout<<"targets="<<targetCount<<" casts="<<casts<<" candidates="<<stats.candidates
             <<" wall_ms="<<elapsed<<" mean_ray_ms="<<stats.averageMilliseconds()<<'\n';
}
