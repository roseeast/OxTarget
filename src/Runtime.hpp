#pragma once

#include "core/HandlePool.hpp"
#include "core/TargetManager.hpp"
#include "platform/samp/SampAdapter.hpp"
#include "adapters/ColAndreasAdapter.hpp"
#include <amx/amx.h>
#include <chrono>
#include <memory>

namespace ox {

struct StoredRay {
    RayResult result;
    AMX* owner{};
    std::chrono::steady_clock::time_point created{std::chrono::steady_clock::now()};
};

class Runtime {
public:
    Runtime() : adapter(), manager(adapter, adapter, std::make_unique<ColAndreasAdapter>(adapter)) {}
    void pruneRays() {
        std::vector<Handle> expired;
        const auto now = std::chrono::steady_clock::now();
        rays.forEach([&](Handle h, StoredRay& ray) { if (now - ray.created > std::chrono::seconds(10)) expired.push_back(h); });
        for (Handle h : expired) rays.erase(h);
    }
    void unloadAmx(AMX* amx) {
        adapter.removeAmx(amx);
        manager.unloadOwner(amx);
        std::vector<Handle> owned;
        rays.forEach([&](Handle h, StoredRay& ray) { if (ray.owner == amx) owned.push_back(h); });
        for (Handle h : owned) rays.erase(h);
    }
    SampAdapter adapter;
    TargetManager manager;
    HandlePool<StoredRay> rays;
};

Runtime* runtime();
void setRuntime(std::unique_ptr<Runtime> value);

} // namespace ox
