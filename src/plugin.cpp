#include "Runtime.hpp"
#include "core/CollisionBackend.hpp"
#include "pawn/Natives.hpp"
#include <plugincommon.h>
#include <amx/amx.h>
#include <memory>

extern void* pAMXFunctions;

namespace {
using LogFn = void (*)(const char*, ...);
LogFn gLog = nullptr;
std::unique_ptr<ox::Runtime> gRuntime;
}

namespace ox {
Runtime* runtime() { return gRuntime.get(); }
void setRuntime(std::unique_ptr<Runtime> value) { gRuntime = std::move(value); }
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** data) {
    if (!data || !data[PLUGIN_DATA_AMX_EXPORTS]) return false;
    pAMXFunctions = data[PLUGIN_DATA_AMX_EXPORTS];
    gLog = reinterpret_cast<LogFn>(data[PLUGIN_DATA_LOGPRINTF]);
    if (gLog) {
        gLog("[OxTarget] Loading...");
        gLog("[OxTarget] Version: 1.0.0");
#if defined(_WIN32)
        gLog("[OxTarget] Platform: Windows");
#else
        gLog("[OxTarget] Platform: Linux");
#endif
        gLog("[OxTarget] Server: SA-MP/open.mp legacy adapter");
        gLog("[OxTarget] Collision backend: ColAndreas Pawn bridge (optional; safe fallback active)");
        gLog("[OxTarget] Streamer integration: detected per Pawn script include");
    }
    ox::setRuntime(std::make_unique<ox::Runtime>());
    if (gLog) gLog("[OxTarget] Loaded successfully.");
    return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
    ox::setRuntime(nullptr);
    if (gLog) gLog("[OxTarget] Unloaded.");
    gLog = nullptr;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx) {
    if (!ox::runtime() || !amx) return AMX_ERR_INIT;
    ox::runtime()->adapter.addAmx(amx);
    return amx_Register(amx, ox::NativeList, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx) {
    if (ox::runtime() && amx) ox::runtime()->unloadAmx(amx);
    return AMX_ERR_NONE;
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
    if (!ox::runtime()) return;
    ox::runtime()->manager.tick();
    ox::runtime()->pruneRays();
}
