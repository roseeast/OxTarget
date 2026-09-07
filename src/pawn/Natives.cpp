#include "Natives.hpp"
#include "Runtime.hpp"
#include "core/PlayerTargetState.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace ox {
namespace {

float toFloat(cell value) { float out; std::memcpy(&out, &value, sizeof(out)); return out; }
cell toCell(float value) { cell out; std::memcpy(&out, &value, sizeof(out)); return out; }
Handle toHandle(cell value) { return static_cast<Handle>(value); }
const char* viewName(ViewSource source) {
    switch (source) { case ViewSource::Camera: return "CAMERA"; case ViewSource::Aim: return "AIM";
        case ViewSource::Facing: return "FACING"; default: return "AUTO"; }
}
const char* entityName(EntityType type) {
    switch (type) { case EntityType::Player: return "PLAYER"; case EntityType::Vehicle: return "VEHICLE";
        case EntityType::Object: return "OBJECT"; case EntityType::PlayerObject: return "PLAYER_OBJECT";
        case EntityType::Actor: return "ACTOR"; case EntityType::DynamicObject: return "DYNAMIC_OBJECT";
        case EntityType::DynamicActor: return "DYNAMIC_ACTOR"; case EntityType::DynamicPickup: return "DYNAMIC_PICKUP";
        case EntityType::Point: return "POINT"; default: return "NONE"; }
}

bool writeCell(AMX* amx, cell parameter, cell value) {
    cell* address = nullptr;
    if (!amx || amx_GetAddr(amx, parameter, &address) != AMX_ERR_NONE || !address) return false;
    *address = value;
    return true;
}

bool writeVec(AMX* amx, cell* params, int first, const Vec3& value) {
    return writeCell(amx, params[first], toCell(value.x)) && writeCell(amx, params[first + 1], toCell(value.y)) &&
           writeCell(amx, params[first + 2], toCell(value.z));
}

std::string readString(AMX* amx, cell parameter) {
    cell* address = nullptr; int length = 0;
    if (amx_GetAddr(amx, parameter, &address) != AMX_ERR_NONE || !address || amx_StrLen(address, &length) != AMX_ERR_NONE || length < 0 || length > 4096) return {};
    std::vector<char> buffer(static_cast<std::size_t>(length) + 1u);
    if (amx_GetString(buffer.data(), address, 0, buffer.size()) != AMX_ERR_NONE) return {};
    return buffer.data();
}

bool writeString(AMX* amx, cell parameter, const std::string& value, int size) {
    cell* address = nullptr;
    if (size <= 0 || size > 65535 || amx_GetAddr(amx, parameter, &address) != AMX_ERR_NONE || !address) return false;
    return amx_SetString(address, value.c_str(), 0, 0, static_cast<std::size_t>(size)) == AMX_ERR_NONE;
}

Target* ownedTarget(AMX* amx, Handle handle) {
    Runtime* rt = runtime(); if (!rt) return nullptr;
    Target* target = rt->manager.get(handle);
    return target && target->owner == amx ? target : nullptr;
}

StoredRay* ownedRay(AMX* amx, Handle handle) {
    Runtime* rt = runtime(); if (!rt) return nullptr;
    StoredRay* ray = rt->rays.get(handle);
    return ray && ray->owner == amx ? ray : nullptr;
}

cell AMX_NATIVE_CALL nCreate(AMX* amx, cell* p) {
    Runtime* rt = runtime(); if (!rt) return 0;
    const auto type = static_cast<EntityType>(p[1]);
    if (type < EntityType::Player || type > EntityType::DynamicPickup) return 0;
    return static_cast<cell>(rt->manager.create(type, static_cast<int>(p[2]), -1, amx));
}
cell AMX_NATIVE_CALL nAddPlayer(AMX* a, cell* p) { return runtime() ? static_cast<cell>(runtime()->manager.create(EntityType::Player, p[1], -1, a)) : 0; }
cell AMX_NATIVE_CALL nAddVehicle(AMX* a, cell* p) { return runtime() ? static_cast<cell>(runtime()->manager.create(EntityType::Vehicle, p[1], -1, a)) : 0; }
cell AMX_NATIVE_CALL nAddObject(AMX* a, cell* p) { return runtime() ? static_cast<cell>(runtime()->manager.create(EntityType::Object, p[1], -1, a)) : 0; }
cell AMX_NATIVE_CALL nAddPlayerObject(AMX* a, cell* p) { return runtime() ? static_cast<cell>(runtime()->manager.create(EntityType::PlayerObject, p[2], p[1], a)) : 0; }
cell AMX_NATIVE_CALL nAddActor(AMX* a, cell* p) { return runtime() ? static_cast<cell>(runtime()->manager.create(EntityType::Actor, p[1], -1, a)) : 0; }
cell AMX_NATIVE_CALL nAddPoint(AMX* a, cell* p) { return runtime() ? static_cast<cell>(runtime()->manager.addPoint({toFloat(p[1]), toFloat(p[2]), toFloat(p[3])}, toFloat(p[4]), a)) : 0; }
cell AMX_NATIVE_CALL nDestroy(AMX* a, cell* p) { return ownedTarget(a, toHandle(p[1])) && runtime()->manager.destroy(toHandle(p[1])); }
cell AMX_NATIVE_CALL nIsValid(AMX*, cell* p) { return runtime() && runtime()->manager.get(toHandle(p[1])) != nullptr; }
cell AMX_NATIVE_CALL nSetDistance(AMX* a, cell* p) { Target* t = ownedTarget(a, toHandle(p[1])); float v = toFloat(p[2]); if (!t || !std::isfinite(v) || v <= 0 || v > 1000) return 0; t->distance = v; return runtime()->manager.sync(toHandle(p[1])); }
cell AMX_NATIVE_CALL nGetDistance(AMX*, cell* p) { const Target* t = runtime() ? runtime()->manager.get(toHandle(p[1])) : nullptr; return toCell(t ? t->distance : 0.0f); }
cell AMX_NATIVE_CALL nSetEnabled(AMX* a, cell* p) { Target* t = ownedTarget(a, toHandle(p[1])); if (!t) return 0; t->enabled = p[2] != 0; return runtime()->manager.sync(toHandle(p[1])); }
cell AMX_NATIVE_CALL nIsEnabled(AMX*, cell* p) { const Target* t = runtime() ? runtime()->manager.get(toHandle(p[1])) : nullptr; return t && t->enabled; }
cell AMX_NATIVE_CALL nSetPriority(AMX* a, cell* p) { Target* t = ownedTarget(a, toHandle(p[1])); if (!t) return 0; t->priority = p[2]; return runtime()->manager.sync(toHandle(p[1])); }
cell AMX_NATIVE_CALL nGetPriority(AMX*, cell* p) { const Target* t = runtime() ? runtime()->manager.get(toHandle(p[1])) : nullptr; return t ? t->priority : 0; }
cell AMX_NATIVE_CALL nGetType(AMX*, cell* p) { const Target* t = runtime() ? runtime()->manager.get(toHandle(p[1])) : nullptr; return t ? static_cast<cell>(t->entity.type) : 0; }
cell AMX_NATIVE_CALL nGetEntity(AMX*, cell* p) { const Target* t = runtime() ? runtime()->manager.get(toHandle(p[1])) : nullptr; return t ? t->entity.id : -1; }
cell AMX_NATIVE_CALL nSetInterior(AMX* a, cell* p) { Target* t = ownedTarget(a, toHandle(p[1])); if (!t || p[2] < -1) return 0; t->interior = p[2]; return runtime()->manager.sync(toHandle(p[1])); }
cell AMX_NATIVE_CALL nSetWorld(AMX* a, cell* p) { Target* t = ownedTarget(a, toHandle(p[1])); if (!t || p[2] < -1) return 0; t->world = p[2]; return runtime()->manager.sync(toHandle(p[1])); }
cell AMX_NATIVE_CALL nSetMask(AMX* a, cell* p) { Target* t = ownedTarget(a, toHandle(p[1])); auto m = static_cast<std::uint32_t>(p[2]); if (!t || (m & ~MaskAll) != 0u) return 0; t->rayMask = m; return runtime()->manager.sync(toHandle(p[1])); }

cell AMX_NATIVE_CALL nAddOption(AMX* a, cell* p) {
    if (!runtime() || !ownedTarget(a, toHandle(p[1]))) return 0;
    const int numParams = static_cast<int>(p[0] / sizeof(cell));
    if (numParams >= 5) {
        return static_cast<cell>(runtime()->manager.addOption(toHandle(p[1]), readString(a, p[2]), readString(a, p[3]), p[4], static_cast<std::uint8_t>(p[5])));
    }
    if (numParams == 4) {
        return static_cast<cell>(runtime()->manager.addOption(toHandle(p[1]), readString(a, p[2]), readString(a, p[3]), p[4], 0));
    }
    return static_cast<cell>(runtime()->manager.addOption(toHandle(p[1]), readString(a, p[2]), {}, p[3], 0));
}
cell AMX_NATIVE_CALL nAddVehicleOption(AMX* a, cell* p) {
    if (!runtime() || !ownedTarget(a, toHandle(p[1]))) return 0;
    const int numParams = static_cast<int>(p[0] / sizeof(cell));
    const auto zone = static_cast<std::uint8_t>(p[2]);
    const std::string name = readString(a, p[3]);
    const std::string icon = numParams >= 4 ? readString(a, p[4]) : "";
    const int data = numParams >= 5 ? p[5] : 0;
    return static_cast<cell>(runtime()->manager.addOption(toHandle(p[1]), name, icon, data, zone));
}
cell AMX_NATIVE_CALL nGetOptionZone(AMX*, cell* p) { return runtime() ? static_cast<cell>(runtime()->manager.getOptionZone(toHandle(p[1]))) : 0; }
cell AMX_NATIVE_CALL nSetOptionZone(AMX* a, cell* p) { Option* o = runtime() ? runtime()->manager.getOption(toHandle(p[1])) : nullptr; if (!o || !ownedTarget(a, o->target)) return 0; return runtime()->manager.setOptionZone(toHandle(p[1]), static_cast<std::uint8_t>(p[2])) ? 1 : 0; }
cell AMX_NATIVE_CALL nGetPlayerZone(AMX*, cell* p) { return runtime() ? static_cast<cell>(runtime()->manager.getPlayerZone(p[1])) : 0; }
cell AMX_NATIVE_CALL nSetZoneFiltering(AMX* a, cell* p) { if (!runtime() || !ownedTarget(a, toHandle(p[1]))) return 0; return runtime()->manager.setZoneFiltering(toHandle(p[1]), p[2] != 0) ? 1 : 0; }
cell AMX_NATIVE_CALL nIsZoneFiltering(AMX*, cell* p) { return runtime() && runtime()->manager.isZoneFiltering(toHandle(p[1])) ? 1 : 0; }
cell AMX_NATIVE_CALL nRemoveOption(AMX* a, cell* p) { return runtime() && ownedTarget(a, toHandle(p[1])) && runtime()->manager.removeOption(toHandle(p[1]), toHandle(p[2])); }
cell AMX_NATIVE_CALL nSetOptionEnabled(AMX* a, cell* p) { Runtime* rt = runtime(); Option* o = rt ? rt->manager.getOption(toHandle(p[1])) : nullptr; if (!o || !ownedTarget(a, o->target)) return 0; o->enabled = p[2] != 0; return 1; }
cell AMX_NATIVE_CALL nIsOptionEnabled(AMX*, cell* p) { Option* o = runtime() ? runtime()->manager.getOption(toHandle(p[1])) : nullptr; return o && o->enabled; }
cell AMX_NATIVE_CALL nOptionCount(AMX*, cell* p) { return runtime() ? static_cast<cell>(runtime()->manager.optionCount(toHandle(p[1]))) : 0; }
cell AMX_NATIVE_CALL nGetOption(AMX*, cell* p) { return runtime() && p[2] >= 0 ? static_cast<cell>(runtime()->manager.optionAt(toHandle(p[1]), static_cast<std::size_t>(p[2]))) : 0; }
cell AMX_NATIVE_CALL nGetOptionName(AMX* a, cell* p) { Option* o = runtime() ? runtime()->manager.getOption(toHandle(p[1])) : nullptr; return o && writeString(a, p[2], o->name, p[3]); }
cell AMX_NATIVE_CALL nGetOptionIcon(AMX* a, cell* p) { Option* o = runtime() ? runtime()->manager.getOption(toHandle(p[1])) : nullptr; return o && writeString(a, p[2], o->icon, p[3]); }
cell AMX_NATIVE_CALL nGetOptionData(AMX*, cell* p) { Option* o = runtime() ? runtime()->manager.getOption(toHandle(p[1])) : nullptr; return o ? o->data : 0; }

cell AMX_NATIVE_CALL nEnablePlayer(AMX*, cell* p) { Runtime* rt = runtime(); if (!rt || !rt->adapter.playerConnected(p[1])) return 0; if (p[2] == 0) { rt->manager.disconnectPlayer(p[1]); rt->manager.playerState(p[1]).enabled = false; } else rt->manager.playerState(p[1]).enabled = true; return 1; }
cell AMX_NATIVE_CALL nPlayerEnabled(AMX*, cell* p) { const PlayerState* s = runtime() ? runtime()->manager.findPlayerState(p[1]) : nullptr; return s ? s->enabled : 1; }
cell AMX_NATIVE_CALL nPlayerTarget(AMX*, cell* p) { const PlayerState* s = runtime() ? runtime()->manager.findPlayerState(p[1]) : nullptr; return s ? static_cast<cell>(s->current) : 0; }
cell AMX_NATIVE_CALL nSetUpdateRate(AMX*, cell* p) { Runtime* rt = runtime(); if (!rt || !rt->adapter.playerConnected(p[1]) || !validUpdateRate(p[2])) return 0; rt->manager.playerState(p[1]).updateRateMs = static_cast<std::uint32_t>(p[2]); return 1; }
cell AMX_NATIVE_CALL nSetViewSource(AMX*, cell* p) { Runtime* rt = runtime(); if (!rt || !rt->adapter.playerConnected(p[1]) || p[2] < 0 || p[2] > 3) return 0; rt->manager.playerState(p[1]).viewSource = static_cast<ViewSource>(p[2]); return 1; }
cell AMX_NATIVE_CALL nGetViewSource(AMX*, cell* p) { const PlayerState* s = runtime() ? runtime()->manager.findPlayerState(p[1]) : nullptr; return s ? static_cast<cell>(s->viewSource) : 0; }
cell AMX_NATIVE_CALL nSetHysteresis(AMX*, cell* p) { Runtime* rt = runtime(); float v = toFloat(p[2]); if (!rt || !rt->adapter.playerConnected(p[1]) || !validHysteresis(v)) return 0; rt->manager.playerState(p[1]).hysteresis = v; return 1; }
cell AMX_NATIVE_CALL nGetPlayerView(AMX* a, cell* p) { Runtime* rt = runtime(); Vec3 o,d; ViewSource used{}; return rt && rt->manager.getView(p[1], o, d, used) && writeVec(a,p,2,o) && writeVec(a,p,5,d); }
cell AMX_NATIVE_CALL nSelect(AMX*, cell* p) { return runtime() && runtime()->manager.select(p[1], toHandle(p[2])); }
cell AMX_NATIVE_CALL nSelectIndex(AMX*, cell* p) { return runtime() && p[2] >= 0 && runtime()->manager.selectIndex(p[1], static_cast<std::size_t>(p[2])); }
cell AMX_NATIVE_CALL nSelectTargetOption(AMX*, cell* p) { return runtime() && p[3] >= 0 && runtime()->manager.selectTargetOption(p[1], toHandle(p[2]), static_cast<std::size_t>(p[3])); }

cell storeRay(AMX* amx, cell output, RayResult result) {
    Runtime* rt = runtime(); if (!rt || result.hitType == HitType::None) { writeCell(amx, output, 0); return 0; }
    Handle handle = rt->rays.emplace(StoredRay{result, amx});
    if (handle == InvalidHandle || !writeCell(amx, output, static_cast<cell>(handle))) { if (handle) rt->rays.erase(handle); return 0; }
    return 1;
}
cell AMX_NATIVE_CALL nCastCamera(AMX* a, cell* p) { Runtime* rt=runtime(); if(!rt) return 0; return storeRay(a,p[4],rt->manager.castCamera(p[1],toFloat(p[2]),static_cast<std::uint32_t>(p[3]))); }
cell AMX_NATIVE_CALL nCastRay(AMX* a, cell* p) { Runtime* rt=runtime(); if(!rt) return 0; Vec3 s{toFloat(p[1]),toFloat(p[2]),toFloat(p[3])}, e{toFloat(p[4]),toFloat(p[5]),toFloat(p[6])}; return storeRay(a,p[8],rt->manager.castRay({s,e-s,(e-s).length()},static_cast<std::uint32_t>(p[7]))); }
cell AMX_NATIVE_CALL nCastDirection(AMX* a, cell* p) { Runtime* rt=runtime(); if(!rt) return 0; return storeRay(a,p[9],rt->manager.castRay({{toFloat(p[1]),toFloat(p[2]),toFloat(p[3])},{toFloat(p[4]),toFloat(p[5]),toFloat(p[6])},toFloat(p[7])},static_cast<std::uint32_t>(p[8]))); }
cell AMX_NATIVE_CALL nHitType(AMX* a, cell* p) { StoredRay* r=ownedRay(a,toHandle(p[1])); return r?static_cast<cell>(r->result.hitType):0; }
cell AMX_NATIVE_CALL nHitEntityType(AMX* a, cell* p) { StoredRay* r=ownedRay(a,toHandle(p[1])); return r?static_cast<cell>(r->result.entityType):0; }
cell AMX_NATIVE_CALL nHitEntity(AMX* a, cell* p) { StoredRay* r=ownedRay(a,toHandle(p[1])); return r?r->result.entity:-1; }
cell AMX_NATIVE_CALL nHitPos(AMX* a, cell* p) { StoredRay* r=ownedRay(a,toHandle(p[1])); return r&&writeVec(a,p,2,r->result.position); }
cell AMX_NATIVE_CALL nHitNormal(AMX* a, cell* p) { StoredRay* r=ownedRay(a,toHandle(p[1])); return r&&writeVec(a,p,2,r->result.normal); }
cell AMX_NATIVE_CALL nHitDistance(AMX* a, cell* p) { StoredRay* r=ownedRay(a,toHandle(p[1])); return toCell(r?r->result.distance:0.0f); }
cell AMX_NATIVE_CALL nHitModel(AMX* a, cell* p) { StoredRay* r=ownedRay(a,toHandle(p[1])); return r?r->result.model:-1; }
cell AMX_NATIVE_CALL nHitMaterial(AMX* a, cell* p) { StoredRay* r=ownedRay(a,toHandle(p[1])); return r?r->result.material:-1; }
cell AMX_NATIVE_CALL nHitZone(AMX* a, cell* p) { StoredRay* r=ownedRay(a,toHandle(p[1])); return r?static_cast<cell>(r->result.zone):0; }
cell AMX_NATIVE_CALL nHitLocalPos(AMX* a, cell* p) { StoredRay* r=ownedRay(a,toHandle(p[1])); return r&&writeVec(a,p,2,r->result.localPosition); }
cell AMX_NATIVE_CALL nDestroyRay(AMX* a, cell* p) { return ownedRay(a,toHandle(p[1])) && runtime()->rays.erase(toHandle(p[1])); }

cell AMX_NATIVE_CALL nSetDebug(AMX*, cell* p) { Runtime* rt=runtime(); if(!rt||!rt->adapter.playerConnected(p[1])) return 0; rt->manager.playerState(p[1]).debug=p[2]!=0; return 1; }
cell AMX_NATIVE_CALL nDebugInfo(AMX* a, cell* p) { Runtime* rt=runtime(); const PlayerState* s=rt?rt->manager.findPlayerState(p[1]):nullptr; if(!s) return 0; char b[384]; const Target* t=rt->manager.get(s->current); std::snprintf(b,sizeof(b),"OxTarget Debug\nView Source: %s\nCandidates: %zu\nTarget Type: %s\nEntity ID: %d\nTarget Distance: %.3f\nRay Time: %.3f ms",viewName(s->lastViewSource),s->candidates,entityName(t?t->entity.type:EntityType::None),t?t->entity.id:-1,s->lastDistance,s->rayMilliseconds); return writeString(a,p[2],b,p[3]); }
cell AMX_NATIVE_CALL nActiveTargets(AMX*, cell*) { return runtime()?static_cast<cell>(runtime()->manager.targets().size()):0; }
cell AMX_NATIVE_CALL nRaycasts(AMX*, cell*) { return runtime()?static_cast<cell>(runtime()->manager.rayEngine().stats().raycasts):0; }
cell AMX_NATIVE_CALL nCandidates(AMX*, cell*) { return runtime()?static_cast<cell>(runtime()->manager.rayEngine().stats().candidates):0; }
cell AMX_NATIVE_CALL nAverageTime(AMX*, cell*) { return toCell(runtime()?static_cast<float>(runtime()->manager.rayEngine().stats().averageMilliseconds()):0.0f); }

} // namespace

AMX_NATIVE_INFO NativeList[] = {
    {"OxTarget_Create", nCreate}, {"OxTarget_AddPlayer", nAddPlayer}, {"OxTarget_AddVehicle", nAddVehicle},
    {"OxTarget_AddObject", nAddObject}, {"OxTarget_AddPlayerObject", nAddPlayerObject}, {"OxTarget_AddActor", nAddActor},
    {"OxTarget_AddPoint", nAddPoint}, {"OxTarget_Destroy", nDestroy}, {"OxTarget_IsValid", nIsValid},
    {"OxTarget_SetDistance", nSetDistance}, {"OxTarget_GetDistance", nGetDistance}, {"OxTarget_SetEnabled", nSetEnabled},
    {"OxTarget_IsEnabled", nIsEnabled}, {"OxTarget_SetPriority", nSetPriority}, {"OxTarget_GetPriority", nGetPriority},
    {"OxTarget_GetType", nGetType}, {"OxTarget_GetEntity", nGetEntity}, {"OxTarget_SetInterior", nSetInterior},
    {"OxTarget_SetVirtualWorld", nSetWorld}, {"OxTarget_SetRayMask", nSetMask}, {"OxTarget_AddOption", nAddOption},
    {"OxTarget_AddVehicleOption", nAddVehicleOption}, {"OxTarget_AddZoneOption", nAddVehicleOption},
    {"OxTarget_GetOptionZone", nGetOptionZone}, {"OxTarget_SetOptionZone", nSetOptionZone},
    {"OxTarget_SetZoneFiltering", nSetZoneFiltering}, {"OxTarget_IsZoneFiltering", nIsZoneFiltering},
    {"OxTarget_GetPlayerZone", nGetPlayerZone},
    {"OxTarget_RemoveOption", nRemoveOption}, {"OxTarget_SetOptionEnabled", nSetOptionEnabled},
    {"OxTarget_IsOptionEnabled", nIsOptionEnabled}, {"OxTarget_GetOptionCount", nOptionCount}, {"OxTarget_GetOption", nGetOption},
    {"OxTarget_GetOptionName", nGetOptionName}, {"OxTarget_GetOptionIcon", nGetOptionIcon}, {"OxTarget_GetOptionData", nGetOptionData},
    {"OxTarget_EnablePlayer", nEnablePlayer}, {"OxTarget_IsPlayerEnabled", nPlayerEnabled},
    {"OxTarget_GetPlayerTarget", nPlayerTarget}, {"OxTarget_SetPlayerUpdateRate", nSetUpdateRate},
    {"OxTarget_SetPlayerViewSource", nSetViewSource}, {"OxTarget_GetPlayerViewSource", nGetViewSource},
    {"OxTarget_SetPlayerHysteresis", nSetHysteresis}, {"OxTarget_GetPlayerView", nGetPlayerView},
    {"OxTarget_Select", nSelect}, {"OxTarget_SelectIndex", nSelectIndex},
    {"OxTarget_SelectTargetOption", nSelectTargetOption}, {"OxTarget_CastCamera", nCastCamera},
    {"OxTarget_CastRay", nCastRay}, {"OxTarget_CastDirection", nCastDirection}, {"OxTarget_GetHitType", nHitType},
    {"OxTarget_GetHitEntityType", nHitEntityType}, {"OxTarget_GetHitEntity", nHitEntity},
    {"OxTarget_GetHitPosition", nHitPos}, {"OxTarget_GetHitNormal", nHitNormal},
    {"OxTarget_GetHitDistance", nHitDistance}, {"OxTarget_GetHitModel", nHitModel},
    {"OxTarget_GetHitMaterial", nHitMaterial}, {"OxTarget_GetHitZone", nHitZone},
    {"OxTarget_GetHitLocalPosition", nHitLocalPos}, {"OxTarget_DestroyRay", nDestroyRay},
    {"OxTarget_SetDebug", nSetDebug}, {"OxTarget_GetDebugInfo", nDebugInfo},
    {"OxTarget_GetActiveTargetCount", nActiveTargets}, {"OxTarget_GetRaycastCount", nRaycasts},
    {"OxTarget_GetCandidateTestCount", nCandidates}, {"OxTarget_GetAverageRayTime", nAverageTime},
    {nullptr, nullptr}
};

} // namespace ox
