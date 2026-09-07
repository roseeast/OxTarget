# Changelog

## 1.0.0 - 2026-09-08

- Initial SA-MP 0.3.7/0.3.DL and open.mp legacy plugin implementation.
- Added view fallback (camera -> aim -> facing), spatial hash broad phase, and geometric entity intersections.
- Added 3rd-person raycast distance compensation measured from player character position (`playerPos`) with camera ray extension.
- Added centered on-object style interactive TextDraw UI (`330.0, 226.0`) with cyan reticle dot (`320, 240`), option cards, number badges, and sprite icon support.
- Added Universal Sub-Part Zones (`OX_ZONE_FRONT`, `OX_ZONE_REAR`, `OX_ZONE_SIDES`, `OX_ZONE_LEFT`, `OX_ZONE_RIGHT`, `OX_ZONE_TOP`, `OX_ZONE_BOTTOM`) with vehicle semantic aliases (`OX_ZONE_HOOD`, `OX_ZONE_TRUNK`, `OX_ZONE_DOORS`, `OX_ZONE_DOOR_DRIVER`, `OX_ZONE_DOOR_PASSENGER`).
- Added configurable zone filtering toggle via `OxTarget_SetZoneFiltering` and `OxTarget_IsZoneFiltering` (defaulting to false so all options appear at once unless filtered).
- Added `OxTarget_AddZoneOption` native.
- Added automatic mouse cursor on PC and touch mode on Android upon targeting, with clean auto-cancel.
- Added deterministic direct selection native (`OxTarget_SelectTargetOption`) alongside full ray-revalidated selection (`OxTarget_Select`).
- Added standard ALS hook integration for multi-gamemode compatibility and `OXTARGET_CUSTOM_UI` toggle.
- Added target/options/state callbacks, per-player validators, temporary ray engine, debug statistics, Streamer bridges, and optional ColAndreas occlusion.
- Added complete Linux & Windows x86/x64 CI, working Pawn examples, unit test suite, benchmark utility, and release archives.
