# OxTarget

OxTarget is a high-performance native C++17 targeting and interaction foundation for SA-MP 0.3.7 / 0.3.DL and open.mp. It resolves registered gameplay targets directly from the player's camera line-of-sight in 3D world space, maintains authoritative server-side interaction state, renders an optional built-in style interactive TextDraw UI with auto-touch support on Android, and exposes a platform-neutral Pawn API.

No client-side mods (ASI, CLEO, MoonLoader, DirectX hooks, memory hacks, or custom APKs) are required. It works out-of-the-box on standard PC and Android mobile clients.

---

## Table of Contents

- [Features](#features)
- [Supported Platforms & Architectures](#supported-platforms--architectures)
- [Installation Guide](#installation-guide)
- [Quick Start Example](#quick-start-example)
- [Built-in Interactive Style UI](#built-in-interactive-style-ui)
  - [Auto Cursor (PC) & Touch Mode (Android)](#auto-cursor-pc--touch-mode-android)
  - [Keyboard Shortcuts](#keyboard-shortcuts)
  - [Customizing UI Coordinates](#customizing-ui-coordinates)
  - [Disabling Built-in UI (Custom UI Mode)](#disabling-built-in-ui-custom-ui-mode)
- [Universal Sub-Part Zones & Zone Filtering](#universal-sub-part-zones--zone-filtering)
  - [Universal Zones Enum](#universal-zones-enum)
  - [Zone Filtering Toggle (`OxTarget_SetZoneFiltering`)](#zone-filtering-toggle-oxtarget_setzonefiltering)
  - [Registering Zone Options](#registering-zone-options)
- [Target Types & Geometries](#target-types--geometries)
  - [Streamer Integration](#streamer-integration)
  - [ColAndreas World Occlusion](#colandreas-world-occlusion)
- [Pawn API Reference](#pawn-api-reference)
  - [Target Management Natives](#target-management-natives)
  - [Target Properties & Filtering](#target-properties--filtering)
  - [Option Management Natives](#option-management-natives)
  - [Player Targeting Controls](#player-targeting-controls)
  - [Selection API Natives](#selection-api-natives)
  - [General Raycasting Natives](#general-raycasting-natives)
  - [Built-in UI Helper Functions](#built-in-ui-helper-functions)
  - [Debug & Profiling Natives](#debug--profiling-natives)
- [Callbacks Reference](#callbacks-reference)
- [Enums & Constants](#enums--constants)
- [Included Examples](#included-examples)
- [Building from Source](#building-from-source)
- [License](#license)

---

## Features

- **Authoritative 3D Spatial Engine**: Built in modern C++17 using a uniform spatial hash grid (16-metre cells); no costly full-entity per-tick loops.
- **True 3rd-Person Camera Compensation**: Target distance validation is measured from the player character's body (`playerPos`), while the camera line-of-sight ray is extended accordingly. No more needing to physically collide with an entity to interact.
- **Camera Fallback Hierarchy**: Smooth automatic fallback (`Camera Front Vector` → `Aim Vector` → `Facing Angle`) with finite/zero-vector sanitization.
- **Universal Sub-Part Zones**: Detects 3D directional hit sectors (`Front`, `Rear`, `Left`, `Right`, `Top`, `Bottom`) on any entity with oriented bounding boxes (Vehicles, Objects, Dynamic Objects, Player Objects).
- **Configurable Zone Filtering**: Choose per-target whether options appear all at once (`zoneFiltering = false`, default) or filtered dynamically per zone (`zoneFiltering = true`).
- **Built-in Style Centered UI**: Visually attached near the crosshair (`330.0, 226.0`) with a cyan reticle dot at `(320, 240)` on the targeted entity.
- **Mobile / Android Touch Support**: Automatically enables mouse cursor on PC and touch/tap mode on SA-MP Mobile (Android) clients.
- **Fast Keyboard Selection**: Press `Y` (`KEY_YES`) to instantly trigger option 1 without mouse clicking. Press `H` / `Ctrl+Back` to toggle cursor.
- **Anti-Cheat & Validation**: Revalidates distance, world, interior, target state, and user-defined permission callbacks before triggering selections.
- **Standalone Raycasting Utility**: Cast rays from camera, position, or direction and retrieve hit type, entity, coordinates, surface normal, distance, and zone.
- **ColAndreas & Incognito Streamer Ready**: Automatic compile-time integration without forced external dependencies.
- **Cross-Platform**: Binaries provided for Linux x86/x64 and Windows x86/x64.

---

## Supported Platforms & Architectures

| Platform | Architecture | Server Environment | Binary Artifact |
|---|---|---|---|
| **Linux** | **x86 (32-bit)** | SA-MP 0.3.7 / 0.3.DL / open.mp (32-bit) | `plugins/oxtarget.so` |
| **Linux** | **x64 (64-bit)** | open.mp (64-bit) | `plugins/oxtarget.so` |
| **Windows** | **x86 (32-bit)** | SA-MP 0.3.7 / 0.3.DL / open.mp (32-bit) | `plugins/oxtarget.dll` |
| **Windows** | **x64 (64-bit)** | open.mp (64-bit) | `plugins/oxtarget.dll` |
| **Client** | **PC & Android** | SA-MP standard client, open.mp client, SA-MP Mobile APKs | *No client mod needed* |

> [!NOTE]
> Always match the architecture of your server executable: 32-bit server requires 32-bit plugin; 64-bit open.mp server requires 64-bit plugin.

---

## Installation Guide

1. Download the archive matching your operating system and server architecture from [Releases](https://github.com/your-repo/OxTarget/releases).
2. Copy the plugin binary to your server's `plugins/` folder:
   - Linux: `plugins/oxtarget.so`
   - Windows: `plugins/oxtarget.dll`
3. Copy `pawno/include/oxtarget.inc` to your server's `pawno/include/` (or `qawno/include/`).
4. Enable the plugin in your server configuration:

**For SA-MP (`server.cfg`):**
```text
# Windows:
plugins oxtarget

# Linux:
plugins oxtarget.so
```

**For open.mp (`config.json`):**
```json
{
  "pawn": {
    "legacy_plugins": [
      "oxtarget"
    ]
  }
}
```

---

## Quick Start Example

Add OxTarget to your gamemode or filterscript:

```pawn
#include <a_samp> // or <open.mp>
#include <oxtarget>

new OxTarget:gVehicleTarget;

public OnGameModeInit()
{
    // Create a vehicle (Infernus)
    new vehicleid = CreateVehicle(411, 1480.0, -1720.0, 13.5, 90.0, -1, -1, -1);
    
    // Register as an OxTarget with 4.0 meters interaction range
    gVehicleTarget = OxTarget_AddVehicle(vehicleid);
    OxTarget_SetDistance(gVehicleTarget, 4.0);

    // Add interactive options with icons and integer option IDs:
    OxTarget_AddOption(gVehicleTarget, "Lock / Unlock", "HUD:radar_impound", 1);
    OxTarget_AddOption(gVehicleTarget, "Inspect Vehicle", "HUD:radar_light", 2);
    return 1;
}

public OnPlayerOxTargetSelect(playerid, OxTarget:target, OxTargetOption:option, option_id)
{
    #pragma unused option
    new vehicleid = OxTarget_GetEntity(target);

    switch (option_id)
    {
        case 1:
        {
            // Lock / unlock logic
            SendClientMessage(playerid, 0x00FF00FF, "Vehicle door lock toggled!");
        }
        case 2:
        {
            new msg[64];
            format(msg, sizeof(msg), "Vehicle ID: %d | Model: %d", vehicleid, GetVehicleModel(vehicleid));
            SendClientMessage(playerid, 0x00FFFFFF, msg);
        }
    }
    return 1;
}
```

When a player approaches the vehicle and looks at it:
1. The cyan reticle dot targets the vehicle.
2. The style option cards pop up right in their crosshair.
3. The mouse cursor appears on PC; touch/tap mode activates on Android.
4. Clicking card 1 or pressing `Y` triggers `OnPlayerOxTargetSelect`.

---

## Built-in Interactive Style UI

By default, `oxtarget.inc` comes with a built-in player TextDraw menu modeled after interaction widget.

- **Centered On-Target Placement**: Option cards are located at screen center (`X = 330.0, Y = 226.0`) right next to the cyan crosshair dot at `(320, 240)`.
- **Dynamic Card Mapping**: When an option is clicked or hotkey `Y` is pressed, OxTarget deterministically selects the displayed option.
- **Contextual Zone Refresh**: Shifting camera aim dynamically refreshes visible option cards if zone filtering is enabled.

### Auto Cursor (PC) & Touch Mode (Android)
- **PC**: The mouse cursor appears automatically when locking onto a target (`SelectTextDraw`), and automatically disappears when looking away (`CancelSelectTextDraw`).
- **Android Mobile**: Players on SA-MP Mobile can directly **tap** on option cards on the touchscreen.

### Keyboard Shortcuts
- **`Y` Key (`KEY_YES`)**: Instantly executes option card 1 without requiring mouse clicks.
- **`H` / `Ctrl+Back` (`KEY_CTRL_BACK`)**: Toggles the mouse cursor on/off while a target is active.

### Customizing UI Coordinates
You can override the widget position before including `oxtarget`:

```pawn
#define OXTARGET_UI_X (330.0)
#define OXTARGET_UI_Y (226.0)
#include <oxtarget>
```

### Disabling Built-in UI (Custom UI Mode)
If your gamemode provides its own custom UI (e.g. SA-MP Dialog list, custom TextDraws, Radial Menus, or CEF browser widgets):

```pawn
#define OXTARGET_CUSTOM_UI
#include <a_samp>
#include <oxtarget>
```
When `OXTARGET_CUSTOM_UI` is defined, the built-in TextDraws and default ALS input hooks are completely omitted, leaving full UI control to your script callbacks.

---

## Universal Sub-Part Zones & Zone Filtering

OxTarget calculates the exact local 3D intersection coordinates on an entity's Oriented Bounding Box (OBB) to detect which side is being targeted.

### Universal Zones Enum

```pawn
enum OxZone {
    OX_ZONE_ALL = 0,            // Available on all parts of the entity
    OX_ZONE_FRONT = 1,          // Front face (+Y)
    OX_ZONE_REAR = 2,           // Rear face (-Y)
    OX_ZONE_SIDES = 3,          // Any side (Left or Right)
    OX_ZONE_LEFT = 4,           // Left face (-X)
    OX_ZONE_RIGHT = 5,          // Right face (+X)
    OX_ZONE_TOP = 6,            // Top face (+Z)
    OX_ZONE_BOTTOM = 7,         // Bottom face (-Z)

    // Vehicle semantic aliases:
    OX_ZONE_HOOD = 1,           // Front / engine hood
    OX_ZONE_TRUNK = 2,          // Rear / trunk
    OX_ZONE_DOORS = 3,          // Any door (left or right)
    OX_ZONE_DOOR_DRIVER = 4,    // Driver door (left side)
    OX_ZONE_DOOR_PASSENGER = 5  // Passenger door (right side)
};
```

### Zone Filtering Toggle (`OxTarget_SetZoneFiltering`)

By default, **zone filtering is disabled (`false`)**, meaning all options registered on a target appear at once in the target menu.

Developers can enable per-target zone filtering whenever desired:

```pawn
// Enable zone filtering so options only appear when aiming at their respective zone:
OxTarget_SetZoneFiltering(target, true);

// Check if zone filtering is active:
new bool:isFiltering = OxTarget_IsZoneFiltering(target);
```

### Registering Zone Options

Use `OxTarget_AddZoneOption` (or `OxTarget_AddVehicleOption`):

```pawn
new OxTarget:veh = OxTarget_AddVehicle(vehicleid);
OxTarget_SetDistance(veh, 4.5);
OxTarget_SetZoneFiltering(veh, true);

// Only appears when aiming at the vehicle hood:
OxTarget_AddZoneOption(veh, OX_ZONE_HOOD, "Open Hood", "HUD:radar_impound", 1);

// Only appears when aiming at the trunk:
OxTarget_AddZoneOption(veh, OX_ZONE_TRUNK, "Open Trunk", "HUD:radar_impound", 2);

// Appears on both driver and passenger doors:
OxTarget_AddZoneOption(veh, OX_ZONE_DOORS, "Enter Vehicle", "HUD:radar_impound", 3);

// Appears on all parts of the entity regardless of zone:
OxTarget_AddZoneOption(veh, OX_ZONE_ALL, "Inspect Vehicle", "HUD:radar_light", 4);
```

Universal zones work seamlessly across **Vehicles, Static Objects, Streamer Dynamic Objects, and Player Objects**!

---

## Target Types & Geometries

OxTarget supports multiple distinct 3D collision bounding geometries:

| Entity Type | Collision Geometry | Notes |
|---|---|---|
| `OXTARGET_ENTITY_PLAYER` | 3D Capsule (`radius 0.35m, height 1.7m`) | Follows player yaw rotation |
| `OXTARGET_ENTITY_ACTOR` | 3D Capsule (`radius 0.35m, height 1.7m`) | Supports static and dynamic NPC actors |
| `OXTARGET_ENTITY_VEHICLE` | Oriented Bounding Box (OBB) | Automatic yaw rotation and model-category bounding extents |
| `OXTARGET_ENTITY_OBJECT` | Rotated Bounding Box (OBB) | Supports roll, pitch, yaw rotation (`rx, ry, rz`) |
| `OXTARGET_ENTITY_PLAYER_OBJECT` | Rotated Bounding Box (OBB) | Per-player object with 3-axis rotation |
| `OXTARGET_ENTITY_DYNAMIC_OBJECT` | Rotated Bounding Box (OBB) | Incognito Streamer dynamic objects |
| `OXTARGET_ENTITY_DYNAMIC_PICKUP` | 3D Sphere (`radius 0.5m`) | Incognito Streamer dynamic pickups |
| `OXTARGET_ENTITY_POINT` | 3D Sphere | Custom world coordinates with user-defined radius |

### Streamer Integration
Include `<streamer>` before `<oxtarget>`:
```pawn
#include <streamer>
#include <oxtarget>
```

### ColAndreas World Occlusion
If ColAndreas is installed on your server, include `<colandreas>` before `<oxtarget>`:
```pawn
#include <colandreas>
#include <oxtarget>
```
OxTarget automatically tests world occlusion against static buildings and terrain. If ColAndreas is not present, OxTarget functions normally without errors.

---

## Pawn API Reference

### Target Management Natives

| Native | Return | Description |
|---|---|---|
| `OxTarget_Create(OxTargetEntityType:type, entityid)` | `OxTarget` | Creates a manual target by entity type and ID. |
| `OxTarget_AddPlayer(targetplayerid)` | `OxTarget` | Creates an interaction target on a player. |
| `OxTarget_AddVehicle(vehicleid)` | `OxTarget` | Creates an interaction target on a vehicle. |
| `OxTarget_AddObject(objectid)` | `OxTarget` | Creates an interaction target on a static SA-MP object. |
| `OxTarget_AddPlayerObject(playerid, objectid)` | `OxTarget` | Creates an interaction target on a player object. |
| `OxTarget_AddActor(actorid)` | `OxTarget` | Creates an interaction target on an actor / NPC. |
| `OxTarget_AddPoint(Float:x, Float:y, Float:z, Float:radius)` | `OxTarget` | Creates an interaction target sphere at world coordinates. |
| `OxTarget_Destroy(OxTarget:target)` | `bool` | Destroys a target and cleans up all its registered options. |
| `OxTarget_IsValid(OxTarget:target)` | `bool` | Checks whether a target handle is valid and active. |

### Target Properties & Filtering

| Native | Return | Description |
|---|---|---|
| `OxTarget_SetDistance(OxTarget:target, Float:distance)` | `bool` | Sets the maximum interaction distance (in meters). |
| `OxTarget_GetDistance(OxTarget:target)` | `Float` | Gets the maximum interaction distance of the target. |
| `OxTarget_SetEnabled(OxTarget:target, bool:enabled)` | `bool` | Enables or disables a target temporarily. |
| `OxTarget_IsEnabled(OxTarget:target)` | `bool` | Checks whether a target is enabled. |
| `OxTarget_SetPriority(OxTarget:target, priority)` | `bool` | Sets priority resolving overlapping targets (higher wins). |
| `OxTarget_GetPriority(OxTarget:target)` | `int` | Gets the target priority value. |
| `OxTarget_GetType(OxTarget:target)` | `OxTargetEntityType` | Gets the entity type of the target. |
| `OxTarget_GetEntity(OxTarget:target)` | `int` | Gets the entity ID (vehicleid, objectid, playerid, etc.). |
| `OxTarget_SetInterior(OxTarget:target, interiorid)` | `bool` | Restricts target to a specific interior (-1 = all). |
| `OxTarget_SetVirtualWorld(OxTarget:target, worldid)` | `bool` | Restricts target to a specific virtual world (-1 = all). |
| `OxTarget_SetRayMask(OxTarget:target, OxTargetMask:mask)` | `bool` | Sets the raycast bitmask filter for the target. |
| `OxTarget_SetZoneFiltering(OxTarget:target, bool:enabled)` | `bool` | Enables or disables option filtering by targeted zone. |
| `OxTarget_IsZoneFiltering(OxTarget:target)` | `bool` | Checks whether zone filtering is enabled on the target. |

### Option Management Natives

| Native | Return | Description |
|---|---|---|
| `OxTarget_AddOption(OxTarget:target, const name[], ...)` | `OxTargetOption` | Adds an option: `name, option_id` or `name, icon, option_id`. |
| `OxTarget_AddOptionEx(OxTarget:target, const name[], const icon[], option_id, OxZone:zone)` | `OxTargetOption` | Adds an option with explicit zone specification. |
| `OxTarget_AddZoneOption(OxTarget:target, OxZone:zone, const name[], const icon[], option_id)` | `OxTargetOption` | Adds a universal option for a specific directional zone. |
| `OxTarget_AddVehicleOption(OxTarget:target, OxZone:zone, const name[], const icon[], option_id)` | `OxTargetOption` | Compatibility alias for `OxTarget_AddZoneOption`. |
| `OxTarget_GetOptionZone(OxTargetOption:option)` | `OxZone` | Gets the directional zone assigned to an option. |
| `OxTarget_SetOptionZone(OxTargetOption:option, OxZone:zone)` | `bool` | Changes the assigned zone of an option. |
| `OxTarget_RemoveOption(OxTarget:target, OxTargetOption:option)` | `bool` | Removes an option from a target. |
| `OxTarget_SetOptionEnabled(OxTargetOption:option, bool:enabled)` | `bool` | Dynamically enables or disables an option. |
| `OxTarget_IsOptionEnabled(OxTargetOption:option)` | `bool` | Checks whether an option is enabled. |
| `OxTarget_GetOptionCount(OxTarget:target)` | `int` | Gets total options registered on a target. |
| `OxTarget_GetOption(OxTarget:target, index)` | `OxTargetOption` | Retrieves option handle by index (0-indexed). |
| `OxTarget_GetOptionName(OxTargetOption:option, output[], size)` | `bool` | Retrieves the display label text of an option. |
| `OxTarget_GetOptionIcon(OxTargetOption:option, output[], size)` | `bool` | Retrieves the sprite icon name of an option. |
| `OxTarget_GetOptionData(OxTargetOption:option)` | `int` | Retrieves the integer `option_id` of an option. |

### Player Targeting Controls

| Native | Return | Description |
|---|---|---|
| `OxTarget_EnablePlayer(playerid, bool:enabled)` | `bool` | Enables or disables targeting scans for a player. |
| `OxTarget_IsPlayerEnabled(playerid)` | `bool` | Checks whether targeting scans are enabled for a player. |
| `OxTarget_GetPlayerTarget(playerid)` | `OxTarget` | Gets the active target currently in the player's view. |
| `OxTarget_GetPlayerZone(playerid)` | `OxZone` | Gets the local directional zone currently aimed at by the player. |
| `OxTarget_SetPlayerUpdateRate(playerid, rate_ms)` | `bool` | Sets player scan frequency in ms (default 100ms). |
| `OxTarget_SetPlayerViewSource(playerid, OxTargetViewSource:source)` | `bool` | Configures view origin source (`AUTO`, `CAMERA`, `AIM`, `FACING`). |
| `OxTarget_GetPlayerViewSource(playerid)` | `OxTargetViewSource` | Gets the view source currently used by the player. |
| `OxTarget_SetPlayerHysteresis(playerid, Float:margin)` | `bool` | Sets target bounding box release margin (default 0.25m). |
| `OxTarget_GetPlayerView(playerid, &Float:ox, &Float:oy, &Float:oz, &Float:dx, &Float:dy, &Float:dz)` | `bool` | Retrieves the player's current view origin and direction vector. |

### Selection API Natives

| Native | Return | Description |
|---|---|---|
| `OxTarget_SelectTargetOption(playerid, OxTarget:target, index)` | `bool` | Fast deterministic selection without re-casting rays (ideal for UI clicks). |
| `OxTarget_Select(playerid, OxTargetOption:option)` | `bool` | Selection with full real-time ray revalidation from current view. |
| `OxTarget_SelectIndex(playerid, index)` | `bool` | Selects active target option by index with full ray revalidation. |

### General Raycasting Natives

| Native | Return | Description |
|---|---|---|
| `OxTarget_CastCamera(playerid, Float:distance, OxTargetMask:mask, &OxRay:output)` | `bool` | Casts a ray forward from the player's camera. |
| `OxTarget_CastRay(Float:sx, Float:sy, Float:sz, Float:ex, Float:ey, Float:ez, OxTargetMask:mask, &OxRay:output)` | `bool` | Casts a ray between two 3D points. |
| `OxTarget_CastDirection(Float:ox, Float:oy, Float:oz, Float:dx, Float:dy, Float:dz, Float:dist, OxTargetMask:mask, &OxRay:output)` | `bool` | Casts a ray from an origin along a direction vector. |
| `OxTarget_GetHitType(OxRay:ray)` | `OxTargetHitType` | Retrieves hit classification (`NONE`, `WORLD`, `ENTITY`). |
| `OxTarget_GetHitEntityType(OxRay:ray)` | `OxTargetEntityType` | Retrieves the entity type of the hit entity. |
| `OxTarget_GetHitEntity(OxRay:ray)` | `int` | Retrieves the ID of the hit entity. |
| `OxTarget_GetHitPosition(OxRay:ray, &Float:x, &Float:y, &Float:z)` | `bool` | Retrieves the 3D world coordinates of the hit point. |
| `OxTarget_GetHitNormal(OxRay:ray, &Float:nx, &Float:ny, &Float:nz)` | `bool` | Retrieves the surface normal vector at the hit point. |
| `OxTarget_GetHitDistance(OxRay:ray)` | `Float` | Retrieves the distance from origin to hit point. |
| `OxTarget_GetHitZone(OxRay:ray)` | `OxZone` | Retrieves the local directional zone of the hit point. |
| `OxTarget_GetHitLocalPosition(OxRay:ray, &Float:lx, &Float:ly, &Float:lz)` | `bool` | Retrieves local hit coordinates relative to entity center. |
| `OxTarget_DestroyRay(OxRay:ray)` | `bool` | Frees a temporary ray handle after reading. |

### Built-in UI Helper Functions

| Function | Description |
|---|---|
| `OxTarget_ShowUI(playerid, OxTarget:target)` | Displays the target UI widget and enables mouse/touch input. |
| `OxTarget_HideUI(playerid)` | Hides the target UI widget and cancels selection mode. |
| `bool:OxTarget_IsUIOpen(playerid)` | Returns `true` if the UI widget is currently displayed for the player. |
| `OxTarget_ToggleCursor(playerid)` | Toggles cursor selection mode on/off manually. |
| `OxTarget_OnClickPlayerTD(playerid, PlayerText:playertextid)` | Dispatches card clicks (invoked automatically via ALS hooks). |

### Debug & Profiling Natives

| Native | Return | Description |
|---|---|---|
| `OxTarget_SetDebug(playerid, bool:enabled)` | `bool` | Enables on-screen debug profiling for a player. |
| `OxTarget_GetDebugInfo(playerid, output[], size)` | `bool` | Retrieves formatted debug string of active target and timings. |
| `OxTarget_GetActiveTargetCount()` | `int` | Returns total targets registered on the server. |
| `OxTarget_GetRaycastCount()` | `int` | Returns total raycasts performed since server start. |
| `OxTarget_GetCandidateTestCount()` | `int` | Returns total spatial broad-phase candidate tests. |
| `OxTarget_GetAverageRayTime()` | `Float` | Returns average raycast duration in milliseconds. |

---

## Callbacks Reference

### `OnPlayerOxTargetEnter(playerid, OxTarget:target)`
Called when a player's camera first locks onto a valid target within interaction range.

### `OnPlayerOxTargetLeave(playerid, OxTarget:target)`
Called when a player looks away from a target or moves out of interaction range.

### `OnPlayerOxTargetChange(playerid, OxTarget:old_target, OxTarget:new_target)`
Called when a player's camera view transitions directly from one target to another without interruption.

### `OnPlayerOxTargetZoneChange(playerid, OxTarget:target, old_zone, new_zone)`
Called when a player shifts camera aim to a different zone on the same target (e.g. from Hood to Trunk). The built-in UI automatically refreshes displayed cards.

### `OnPlayerOxTargetSelect(playerid, OxTarget:target, OxTargetOption:option, option_id)`
Called when a player clicks an option card or presses `Y`. This is where gameplay logic is executed according to `option_id`.

### `OnPlayerOxTargetCheck(playerid, OxTarget:target)` *(Optional)*
Target permission validator. Return `1` to allow targeting, or `0` to make the target invisible to this player.

### `OnPlayerOxTargetOptionCheck(playerid, OxTarget:target, OxTargetOption:option)` *(Optional)*
Option permission validator. Return `1` to display the option, or `0` to hide it from this player (e.g. faction/job restrictions).

---

## Enums & Constants

### `OxZone` (Universal Zones)
```pawn
enum OxZone {
    OX_ZONE_ALL = 0,
    OX_ZONE_FRONT = 1,
    OX_ZONE_REAR = 2,
    OX_ZONE_SIDES = 3,
    OX_ZONE_LEFT = 4,
    OX_ZONE_RIGHT = 5,
    OX_ZONE_TOP = 6,
    OX_ZONE_BOTTOM = 7,

    // Vehicle semantic aliases:
    OX_ZONE_HOOD = 1,
    OX_ZONE_TRUNK = 2,
    OX_ZONE_DOORS = 3,
    OX_ZONE_DOOR_DRIVER = 4,
    OX_ZONE_DOOR_PASSENGER = 5
};
```

### `OxTargetEntityType`
```pawn
enum OxTargetEntityType {
    OXTARGET_ENTITY_NONE,
    OXTARGET_ENTITY_PLAYER,
    OXTARGET_ENTITY_VEHICLE,
    OXTARGET_ENTITY_OBJECT,
    OXTARGET_ENTITY_PLAYER_OBJECT,
    OXTARGET_ENTITY_ACTOR,
    OXTARGET_ENTITY_DYNAMIC_OBJECT,
    OXTARGET_ENTITY_DYNAMIC_ACTOR,
    OXTARGET_ENTITY_DYNAMIC_PICKUP,
    OXTARGET_ENTITY_POINT
};
```

### `OxTargetMask`
Bitmask flags used to filter targetable entity categories during raycasts:
- `OXTARGET_MASK_WORLD` `(1 << 0)`
- `OXTARGET_MASK_PLAYER` `(1 << 1)`
- `OXTARGET_MASK_VEHICLE` `(1 << 2)`
- `OXTARGET_MASK_OBJECT` `(1 << 3)`
- `OXTARGET_MASK_PLAYER_OBJECT` `(1 << 4)`
- `OXTARGET_MASK_ACTOR` `(1 << 5)`
- `OXTARGET_MASK_DYNAMIC_OBJECT` `(1 << 6)`
- `OXTARGET_MASK_DYNAMIC_ACTOR` `(1 << 7)`
- `OXTARGET_MASK_DYNAMIC_PICKUP` `(1 << 8)`
- `OXTARGET_MASK_ALL` `(0x1FF)`

---

## Included Examples

The [`examples/`](examples/) directory includes 6 complete, production-ready Pawn scripts:

1. **[`vehicle_target.pwn`](examples/vehicle_target.pwn)**:
   Vehicle targeting with `OxTarget_SetZoneFiltering(target, true)`, demonstrating hood (`OX_ZONE_HOOD`), trunk (`OX_ZONE_TRUNK`), doors (`OX_ZONE_DOORS`), and general options.
2. **[`object_target.pwn`](examples/object_target.pwn)**:
   Static ATM object targeting with universal zones (front screen vs rear maintenance panel), plus a Point Target for building entrances.
3. **[`player_target.pwn`](examples/player_target.pwn)**:
   Player-to-player interactions (Handshake, Give Cash, Frisk/Search, Show ID) with `OnPlayerOxTargetOptionCheck` role permissions.
4. **[`target_menu.pwn`](examples/target_menu.pwn)**:
   Custom UI demonstration (`#define OXTARGET_CUSTOM_UI`) rendering a standard SA-MP Dialog list when pressing the `N` key.
5. **[`android_textdraw.pwn`](examples/android_textdraw.pwn)**:
   Mobile/Android dedicated on-screen touch button demonstration using custom UI mode.
6. **[`basic_ray.pwn`](examples/basic_ray.pwn)**:
   Standalone camera raycasting via the `/ray` command with 3D hit position, surface normal, distance, and zone diagnostics.

---

## Building from Source

### Build Prerequisites
- C++17 compliant compiler (GCC 9+, Clang 10+, or MSVC 2019+)
- CMake 3.16+
- Internet access on initial CMake configuration (to clone the SA-MP Plugin SDK)

### Linux x64 (open.mp x64)
```bash
cmake -S . -B build-x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build-x64 -j$(nproc)
./build-x64/oxtarget_tests
```

### Linux x86 (SA-MP 0.3.7 / 0.3.DL)
*(Requires `gcc-multilib` & `g++-multilib`)*
```bash
cmake -S . -B build-x86 -DCMAKE_BUILD_TYPE=Release   -DOXTARGET_BUILD_X86=ON -DOXTARGET_BUILD_X64=OFF
cmake --build build-x86 -j$(nproc)
./build-x86/oxtarget_tests
```

### Windows MSVC / MinGW
```powershell
# 32-bit:
cmake -S . -B build-win32 -A Win32
cmake --build build-win32 --config Release

# 64-bit:
cmake -S . -B build-win64 -A x64
cmake --build build-win64 --config Release
```

---

## License

OxTarget is licensed under the [MIT License](LICENSE). You are free to use, modify, and distribute it in personal or commercial SA-MP and open.mp projects.
