#include "SampAdapter.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace ox {

namespace {
cell floatCell(float value) { cell out; static_assert(sizeof(out) == sizeof(value)); std::memcpy(&out, &value, sizeof(out)); return out; }
float cellFloat(cell value) { float out; static_assert(sizeof(out) == sizeof(value)); std::memcpy(&out, &value, sizeof(out)); return out; }
}

void SampAdapter::addAmx(AMX* amx) { if (amx && !hasAmx(amx)) amx_.push_back(amx); }
void SampAdapter::removeAmx(AMX* amx) { amx_.erase(std::remove(amx_.begin(), amx_.end(), amx), amx_.end()); }
bool SampAdapter::hasAmx(AMX* amx) const { return std::find(amx_.begin(), amx_.end(), amx) != amx_.end(); }
AMX* SampAdapter::primaryAmx() const { return amx_.empty() ? nullptr : amx_.front(); }

bool SampAdapter::call(AMX* amx, const char* name, const std::vector<cell>& args, cell* result) const {
    if (!amx || !hasAmx(amx)) return false;
    int index = -1;
    if (amx_FindPublic(amx, name, &index) != AMX_ERR_NONE) return false;
    const cell saved_stk = amx->stk;
    amx->paramcount = 0;
    for (auto it = args.rbegin(); it != args.rend(); ++it) {
        if (amx_Push(amx, *it) != AMX_ERR_NONE) {
            amx->stk = saved_stk;
            amx->paramcount = 0;
            return false;
        }
    }
    cell ignored{};
    const int err = amx_Exec(amx, result ? result : &ignored, index);
    amx->stk = saved_stk;
    amx->paramcount = 0;
    return err == AMX_ERR_NONE;
}

bool SampAdapter::callRefs(AMX* amx, const char* name, const std::vector<cell>& args,
                           std::size_t refCount, std::vector<cell>& refs, cell* result) const {
    if (!amx || !hasAmx(amx) || refCount == 0) return false;
    const cell saved_stk = amx->stk;
    const cell saved_hea = amx->hea;
    amx->paramcount = 0;
    cell amxAddress{};
    cell* physical = nullptr;
    if (amx_Allot(amx, static_cast<int>(refCount), &amxAddress, &physical) != AMX_ERR_NONE || !physical) {
        amx->stk = saved_stk;
        amx->paramcount = 0;
        return false;
    }
    std::fill(physical, physical + refCount, 0);
    int index = -1;
    bool ok = amx_FindPublic(amx, name, &index) == AMX_ERR_NONE;
    if (ok) {
        for (std::size_t i = refCount; i > 0; --i) {
            if (amx_Push(amx, amxAddress + static_cast<cell>((i - 1) * sizeof(cell))) != AMX_ERR_NONE) {
                ok = false;
                break;
            }
        }
        if (ok) {
            for (auto it = args.rbegin(); it != args.rend(); ++it) {
                if (amx_Push(amx, *it) != AMX_ERR_NONE) {
                    ok = false;
                    break;
                }
            }
        }
        cell ignored{};
        if (ok) {
            ok = amx_Exec(amx, result ? result : &ignored, index) == AMX_ERR_NONE;
            if (ok) refs.assign(physical, physical + refCount);
        }
    }
    amx_Release(amx, amxAddress);
    amx->hea = saved_hea;
    amx->stk = saved_stk;
    amx->paramcount = 0;
    return ok;
}

bool SampAdapter::callPrimary(const char* name, const std::vector<cell>& args, cell* result) const {
    for (AMX* amx : amx_) if (call(amx, name, args, result)) return true;
    return false;
}

void SampAdapter::broadcast(const char* name, const std::vector<cell>& args) const {
    const auto copy = amx_;
    for (AMX* amx : copy) call(amx, name, args);
}

bool SampAdapter::playerConnected(int player) {
    cell result{};
    return player >= 0 && player < 1000 && callPrimary("OxT_InternalConnected", {player}, &result) && result != 0;
}

std::optional<Vec3> SampAdapter::playerPosition(int player) {
    std::vector<cell> refs;
    cell result{};
    for (AMX* amx : amx_) if (callRefs(amx, "OxT_InternalPlayerPos", {player}, 3, refs, &result) && result != 0) {
        Vec3 value{cellFloat(refs[0]), cellFloat(refs[1]), cellFloat(refs[2])};
        if (value.finite()) return value;
    }
    return std::nullopt;
}

int SampAdapter::playerInterior(int player) {
    cell result{-1}; callPrimary("OxT_InternalInterior", {player}, &result); return static_cast<int>(result);
}
int SampAdapter::playerWorld(int player) {
    cell result{-1}; callPrimary("OxT_InternalWorld", {player}, &result); return static_cast<int>(result);
}

bool SampAdapter::playerView(int player, ViewSource source, Vec3& origin, Vec3& direction, ViewSource& used) {
    auto attempt = [&](ViewSource requested) {
        std::vector<cell> refs; cell result{};
        for (AMX* amx : amx_) if (callRefs(amx, "OxT_InternalView", {player, static_cast<cell>(requested)}, 7, refs, &result) && result != 0) {
            Vec3 candidateOrigin{cellFloat(refs[0]), cellFloat(refs[1]), cellFloat(refs[2])};
            Vec3 candidateDirection{cellFloat(refs[3]), cellFloat(refs[4]), cellFloat(refs[5])};
            const auto normalized = candidateDirection.normalized();
            if (!candidateOrigin.finite() || !normalized) continue;
            const auto actualPosition = playerPosition(player);
            if (requested == ViewSource::Camera && actualPosition && distance(candidateOrigin, *actualPosition) > 1000.0f) continue;
            origin = candidateOrigin;
            direction = *normalized;
            used = static_cast<ViewSource>(refs[6]);
            return true;
        }
        return false;
    };
    if (source != ViewSource::Auto) return attempt(source);
    return attempt(ViewSource::Camera) || attempt(ViewSource::Aim) || attempt(ViewSource::Facing);
}

std::optional<Intersection> SampAdapter::worldRay(const Ray& ray) {
    std::vector<cell> refs; cell result{};
    const Vec3 end = ray.origin + ray.direction * ray.maxDistance;
    const std::vector<cell> args{floatCell(ray.origin.x), floatCell(ray.origin.y), floatCell(ray.origin.z),
                                 floatCell(end.x), floatCell(end.y), floatCell(end.z)};
    for (AMX* amx : amx_) if (callRefs(amx, "OxT_InternalWorldRay", args, 6, refs, &result) && result != 0) {
        const Vec3 position{cellFloat(refs[0]), cellFloat(refs[1]), cellFloat(refs[2])};
        const Vec3 normal{cellFloat(refs[3]), cellFloat(refs[4]), cellFloat(refs[5])};
        const float hitDistance = distance(ray.origin, position);
        if (position.finite() && hitDistance >= 0.0f && hitDistance <= ray.maxDistance + 0.01f)
            return Intersection{hitDistance, position, normal};
    }
    return std::nullopt;
}

std::optional<EntitySnapshot> SampAdapter::snapshot(const EntityKey& key) {
    std::vector<cell> refs; cell result{};
    for (AMX* amx : amx_) if (callRefs(amx, "OxT_InternalEntity",
            {static_cast<cell>(key.type), static_cast<cell>(key.id), static_cast<cell>(key.owner)}, 11, refs, &result) && result != 0) {
        EntitySnapshot out;
        out.key = key;
        out.position = {cellFloat(refs[0]), cellFloat(refs[1]), cellFloat(refs[2])};
        out.rotation = {cellFloat(refs[3]), cellFloat(refs[4]), cellFloat(refs[5])};
        out.model = static_cast<int>(refs[6]);
        out.interior = static_cast<int>(refs[7]);
        out.world = static_cast<int>(refs[8]);
        float hx = std::max(0.1f, cellFloat(refs[9]));
        float hy = std::max(0.1f, cellFloat(refs[10]));
        if (key.type == EntityType::Player || key.type == EntityType::Actor || key.type == EntityType::DynamicActor) {
            out.capsule = true; out.capsuleRadius = hx; out.capsuleHeight = std::max(0.2f, hy);
            out.localBounds = {{-hx, -hx, 0.0f}, {hx, hx, out.capsuleHeight}};
        } else {
            float hz = key.type == EntityType::Vehicle ? 1.0f : std::max(hx, hy);
            out.localBounds = {{-hx, -hy, -hz}, {hx, hy, hz}};
        }
        return out;
    }
    return std::nullopt;
}

bool SampAdapter::checkTarget(int player, Handle target, void* owner) {
    AMX* amx = static_cast<AMX*>(owner); cell result{1};
    if (!hasAmx(amx)) return false;
    int index{};
    if (amx_FindPublic(amx, "OnPlayerOxTargetCheck", &index) != AMX_ERR_NONE) return true;
    return call(amx, "OnPlayerOxTargetCheck", {player, static_cast<cell>(target)}, &result) && result != 0;
}

bool SampAdapter::checkOption(int player, Handle target, Handle option, void* owner) {
    AMX* amx = static_cast<AMX*>(owner); cell result{1};
    if (!hasAmx(amx)) return false;
    int index{};
    if (amx_FindPublic(amx, "OnPlayerOxTargetOptionCheck", &index) != AMX_ERR_NONE) return true;
    return call(amx, "OnPlayerOxTargetOptionCheck", {player, static_cast<cell>(target), static_cast<cell>(option)}, &result) && result != 0;
}

void SampAdapter::enter(int player, Handle target, void*) {
    broadcast("OxT_InternalEnter", {player, static_cast<cell>(target)});
    broadcast("OnPlayerOxTargetEnter", {player, static_cast<cell>(target)});
}
void SampAdapter::leave(int player, Handle target, void*) {
    broadcast("OxT_InternalLeave", {player, static_cast<cell>(target)});
    broadcast("OnPlayerOxTargetLeave", {player, static_cast<cell>(target)});
}
void SampAdapter::change(int player, Handle oldTarget, Handle newTarget, void*) {
    broadcast("OnPlayerOxTargetChange", {player, static_cast<cell>(oldTarget), static_cast<cell>(newTarget)});
}
void SampAdapter::select(int player, Handle target, Handle option, int data, void* /*owner*/) {
    broadcast("OxT_InternalSelect", {player, static_cast<cell>(target), static_cast<cell>(option), data});
    broadcast("OnPlayerOxTargetSelect", {player, static_cast<cell>(target), static_cast<cell>(option), data});
}
void SampAdapter::zoneChange(int player, Handle target, std::uint8_t oldZone, std::uint8_t newZone, void* /*owner*/) {
    broadcast("OxT_InternalZoneChange", {player, static_cast<cell>(target), static_cast<cell>(oldZone), static_cast<cell>(newZone)});
    broadcast("OnPlayerOxTargetZoneChange", {player, static_cast<cell>(target), static_cast<cell>(oldZone), static_cast<cell>(newZone)});
}

} // namespace ox
