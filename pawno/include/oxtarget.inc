#if defined _oxtarget_included
    #endinput
#endif
#define _oxtarget_included

#define OXTARGET_VERSION "1.0.0"
#define OXTARGET_VERSION_NUMBER (10000)

enum OxTarget {};
enum OxTargetOption {};
enum OxRay {};
enum OxTargetMask {};

#define INVALID_OX_TARGET (OxTarget:0)
#define INVALID_OX_OPTION (OxTargetOption:0)
#define INVALID_OX_RAY    (OxRay:0)

enum OxTargetViewSource {
    OXTARGET_VIEW_AUTO,
    OXTARGET_VIEW_CAMERA,
    OXTARGET_VIEW_AIM,
    OXTARGET_VIEW_FACING
};

enum OxTargetHitType {
    OXTARGET_HIT_NONE,
    OXTARGET_HIT_WORLD,
    OXTARGET_HIT_ENTITY
};

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

enum OxZone {
    OX_ZONE_ALL = 0,
    OX_ZONE_FRONT = 1,
    OX_ZONE_REAR = 2,
    OX_ZONE_SIDES = 3,
    OX_ZONE_LEFT = 4,
    OX_ZONE_RIGHT = 5,
    OX_ZONE_TOP = 6,
    OX_ZONE_BOTTOM = 7,

    // Vehicle semantic aliases
    OX_ZONE_HOOD = 1,
    OX_ZONE_TRUNK = 2,
    OX_ZONE_DOORS = 3,
    OX_ZONE_DOOR_DRIVER = 4,
    OX_ZONE_DOOR_PASSENGER = 5
};
#define OxVehicleZone OxZone

#define OXTARGET_MASK_WORLD          (OxTargetMask:(1 << 0))
#define OXTARGET_MASK_PLAYER         (OxTargetMask:(1 << 1))
#define OXTARGET_MASK_VEHICLE        (OxTargetMask:(1 << 2))
#define OXTARGET_MASK_OBJECT         (OxTargetMask:(1 << 3))
#define OXTARGET_MASK_PLAYER_OBJECT  (OxTargetMask:(1 << 4))
#define OXTARGET_MASK_ACTOR          (OxTargetMask:(1 << 5))
#define OXTARGET_MASK_DYNAMIC_OBJECT (OxTargetMask:(1 << 6))
#define OXTARGET_MASK_DYNAMIC_ACTOR  (OxTargetMask:(1 << 7))
#define OXTARGET_MASK_DYNAMIC_PICKUP (OxTargetMask:(1 << 8))
#define OXTARGET_MASK_ALL            (OxTargetMask:0x1FF)

native OxTarget:OxTarget_Create(OxTargetEntityType:type, entityid);
native OxTarget:OxTarget_AddPlayer(targetplayerid);
native OxTarget:OxTarget_AddVehicle(vehicleid);
native OxTarget:OxTarget_AddObject(objectid);
native OxTarget:OxTarget_AddPlayerObject(playerid, objectid);
native OxTarget:OxTarget_AddActor(actorid);
native OxTarget:OxTarget_AddPoint(Float:x, Float:y, Float:z, Float:radius);
native bool:OxTarget_Destroy(OxTarget:target);
native bool:OxTarget_IsValid(OxTarget:target);
native bool:OxTarget_SetDistance(OxTarget:target, Float:distance);
native Float:OxTarget_GetDistance(OxTarget:target);
native bool:OxTarget_SetEnabled(OxTarget:target, bool:enabled);
native bool:OxTarget_IsEnabled(OxTarget:target);
native bool:OxTarget_SetPriority(OxTarget:target, priority);
native OxTarget_GetPriority(OxTarget:target);
native OxTargetEntityType:OxTarget_GetType(OxTarget:target);
native OxTarget_GetEntity(OxTarget:target);
native bool:OxTarget_SetInterior(OxTarget:target, interiorid);
native bool:OxTarget_SetVirtualWorld(OxTarget:target, worldid);
native bool:OxTarget_SetRayMask(OxTarget:target, OxTargetMask:mask);
native bool:OxTarget_SetZoneFiltering(OxTarget:target, bool:enabled);
native bool:OxTarget_IsZoneFiltering(OxTarget:target);

native OxTargetOption:OxTarget_AddOption(OxTarget:target, const name[], {Float, _}:...);
native OxTargetOption:OxTarget_AddOptionEx(OxTarget:target, const name[], const icon[], option_id = 0, OxZone:zone = OX_ZONE_ALL) = OxTarget_AddOption;
native OxTargetOption:OxTarget_AddZoneOption(OxTarget:target, OxZone:zone, const name[], const icon[] = "", option_id = 0);
native OxTargetOption:OxTarget_AddVehicleOption(OxTarget:target, OxZone:zone, const name[], const icon[] = "", option_id = 0);
native OxZone:OxTarget_GetOptionZone(OxTargetOption:option);
native bool:OxTarget_SetOptionZone(OxTargetOption:option, OxZone:zone);
native OxZone:OxTarget_GetPlayerZone(playerid);
native bool:OxTarget_RemoveOption(OxTarget:target, OxTargetOption:option);
native bool:OxTarget_SetOptionEnabled(OxTargetOption:option, bool:enabled);
native bool:OxTarget_IsOptionEnabled(OxTargetOption:option);
native OxTarget_GetOptionCount(OxTarget:target);
native OxTargetOption:OxTarget_GetOption(OxTarget:target, index);
native bool:OxTarget_GetOptionName(OxTargetOption:option, output[], size = sizeof(output));
native bool:OxTarget_GetOptionIcon(OxTargetOption:option, output[], size = sizeof(output));
native OxTarget_GetOptionData(OxTargetOption:option);

native bool:OxTarget_EnablePlayer(playerid, bool:enabled);
native bool:OxTarget_IsPlayerEnabled(playerid);
native OxTarget:OxTarget_GetPlayerTarget(playerid);
native bool:OxTarget_SetPlayerUpdateRate(playerid, milliseconds);
native bool:OxTarget_SetPlayerViewSource(playerid, OxTargetViewSource:source);
native OxTargetViewSource:OxTarget_GetPlayerViewSource(playerid);
native bool:OxTarget_SetPlayerHysteresis(playerid, Float:value);
native bool:OxTarget_GetPlayerView(playerid, &Float:ox, &Float:oy, &Float:oz, &Float:dx, &Float:dy, &Float:dz);
native bool:OxTarget_Select(playerid, OxTargetOption:option);
native bool:OxTarget_SelectIndex(playerid, option_index);
native bool:OxTarget_SelectTargetOption(playerid, OxTarget:target, option_index);

native bool:OxTarget_CastCamera(playerid, Float:distance, OxTargetMask:mask, &OxRay:result);
native bool:OxTarget_CastRay(Float:sx, Float:sy, Float:sz, Float:ex, Float:ey, Float:ez, OxTargetMask:mask, &OxRay:result);
native bool:OxTarget_CastDirection(Float:x, Float:y, Float:z, Float:dx, Float:dy, Float:dz, Float:distance, OxTargetMask:mask, &OxRay:result);
native OxTargetHitType:OxTarget_GetHitType(OxRay:ray);
native OxTargetEntityType:OxTarget_GetHitEntityType(OxRay:ray);
native OxTarget_GetHitEntity(OxRay:ray);
native bool:OxTarget_GetHitPosition(OxRay:ray, &Float:x, &Float:y, &Float:z);
native bool:OxTarget_GetHitNormal(OxRay:ray, &Float:x, &Float:y, &Float:z);
native Float:OxTarget_GetHitDistance(OxRay:ray);
native OxTarget_GetHitModel(OxRay:ray);
native OxTarget_GetHitMaterial(OxRay:ray);
native OxVehicleZone:OxTarget_GetHitZone(OxRay:ray);
native bool:OxTarget_GetHitLocalPosition(OxRay:ray, &Float:x, &Float:y, &Float:z);
native bool:OxTarget_DestroyRay(OxRay:ray);

native bool:OxTarget_SetDebug(playerid, bool:enabled);
native bool:OxTarget_GetDebugInfo(playerid, output[], size = sizeof(output));
native OxTarget_GetActiveTargetCount();
native OxTarget_GetRaycastCount();
native OxTarget_GetCandidateTestCount();
native Float:OxTarget_GetAverageRayTime();

forward OnPlayerOxTargetEnter(playerid, OxTarget:target);
forward OnPlayerOxTargetLeave(playerid, OxTarget:target);
forward OnPlayerOxTargetChange(playerid, OxTarget:old_target, OxTarget:new_target);
forward OnPlayerOxTargetZoneChange(playerid, OxTarget:target, OxVehicleZone:old_zone, OxVehicleZone:new_zone);
forward OnPlayerOxTargetSelect(playerid, OxTarget:target, OxTargetOption:option, option_id);
forward OnPlayerOxTargetCheck(playerid, OxTarget:target);
forward OnPlayerOxTargetOptionCheck(playerid, OxTarget:target, OxTargetOption:option);

// Internal portable server bridge. Do not call these functions from game code.
forward OxT_InternalConnected(playerid);
public OxT_InternalConnected(playerid) return IsPlayerConnected(playerid);

forward OxT_InternalPlayerPos(playerid, &Float:x, &Float:y, &Float:z);
public OxT_InternalPlayerPos(playerid, &Float:x, &Float:y, &Float:z)
{
    if (!IsPlayerConnected(playerid)) return 0;
    return GetPlayerPos(playerid, x, y, z);
}

forward OxT_InternalInterior(playerid);
public OxT_InternalInterior(playerid) return IsPlayerConnected(playerid) ? GetPlayerInterior(playerid) : -1;
forward OxT_InternalWorld(playerid);
public OxT_InternalWorld(playerid) return IsPlayerConnected(playerid) ? GetPlayerVirtualWorld(playerid) : -1;

forward OxT_InternalView(playerid, OxTargetViewSource:source, &Float:ox, &Float:oy, &Float:oz,
    &Float:dx, &Float:dy, &Float:dz, &OxTargetViewSource:used);
public OxT_InternalView(playerid, OxTargetViewSource:source, &Float:ox, &Float:oy, &Float:oz,
    &Float:dx, &Float:dy, &Float:dz, &OxTargetViewSource:used)
{
    if (!IsPlayerConnected(playerid)) return 0;
    if (source == OXTARGET_VIEW_AUTO || source == OXTARGET_VIEW_CAMERA)
    {
        if (GetPlayerCameraPos(playerid, ox, oy, oz) && GetPlayerCameraFrontVector(playerid, dx, dy, dz))
        {
            used = OXTARGET_VIEW_CAMERA;
            return 1;
        }
        if (source == OXTARGET_VIEW_CAMERA) return 0;
    }
    if (source == OXTARGET_VIEW_AUTO || source == OXTARGET_VIEW_AIM)
    {
        if (GetPlayerPos(playerid, ox, oy, oz) && GetPlayerCameraFrontVector(playerid, dx, dy, dz))
        {
            oz += 0.7;
            used = OXTARGET_VIEW_AIM;
            return 1;
        }
        if (source == OXTARGET_VIEW_AIM) return 0;
    }
    new Float:angle;
    if (!GetPlayerPos(playerid, ox, oy, oz) || !GetPlayerFacingAngle(playerid, angle)) return 0;
    oz += 0.7;
    dx = -floatsin(angle, degrees);
    dy = floatcos(angle, degrees);
    dz = 0.0;
    used = OXTARGET_VIEW_FACING;
    return 1;
}

stock OxTarget_InternalVehicleBounds(modelid, &Float:hx, &Float:hy)
{
    hx = 1.05; hy = 2.45;
    switch (modelid)
    {
        case 448, 461, 462, 463, 468, 471, 481, 509, 510, 521, 522, 523, 581, 586: { hx = 0.55; hy = 1.15; }
        case 430, 446, 452, 453, 454, 472, 473, 484, 493, 595: { hx = 1.45; hy = 3.5; }
        case 417, 425, 447, 460, 469, 476, 487, 488, 497, 511, 512, 513, 519, 520, 548, 553, 563, 577, 592, 593: { hx = 5.0; hy = 6.0; }
        case 403, 406, 407, 408, 414, 431, 433, 443, 455, 456, 498, 499, 514, 515, 524, 525, 528, 544, 556, 557, 578, 588, 609: { hx = 1.5; hy = 4.0; }
    }
    return 1;
}

forward OxT_InternalEntity(OxTargetEntityType:type, entityid, ownerid, &Float:x, &Float:y, &Float:z,
    &Float:rx, &Float:ry, &Float:rz, &modelid, &interiorid, &worldid, &Float:hx, &Float:hy);
public OxT_InternalEntity(OxTargetEntityType:type, entityid, ownerid, &Float:x, &Float:y, &Float:z,
    &Float:rx, &Float:ry, &Float:rz, &modelid, &interiorid, &worldid, &Float:hx, &Float:hy)
{
    rx = 0.0; ry = 0.0; rz = 0.0; modelid = -1; interiorid = -1; worldid = -1;
    hx = 0.75; hy = 0.75;
    switch (type)
    {
        case OXTARGET_ENTITY_PLAYER:
        {
            if (!IsPlayerConnected(entityid) || !GetPlayerPos(entityid, x, y, z)) return 0;
            GetPlayerFacingAngle(entityid, rz);
            interiorid = GetPlayerInterior(entityid); worldid = GetPlayerVirtualWorld(entityid);
            hx = 0.35; hy = 1.7;
        }
        case OXTARGET_ENTITY_VEHICLE:
        {
            modelid = GetVehicleModel(entityid);
            if (!modelid || !GetVehiclePos(entityid, x, y, z)) return 0;
            GetVehicleZAngle(entityid, rz); worldid = GetVehicleVirtualWorld(entityid);
            OxTarget_InternalVehicleBounds(modelid, hx, hy);
        }
        case OXTARGET_ENTITY_OBJECT:
        {
            if (!IsValidObject(entityid) || !GetObjectPos(entityid, x, y, z)) return 0;
            GetObjectRot(entityid, rx, ry, rz);
            hx = 0.75; hy = 0.75;
        }
        case OXTARGET_ENTITY_PLAYER_OBJECT:
        {
            if (!IsPlayerConnected(ownerid) || !IsValidPlayerObject(ownerid, entityid) || !GetPlayerObjectPos(ownerid, entityid, x, y, z)) return 0;
            GetPlayerObjectRot(ownerid, entityid, rx, ry, rz);
            interiorid = GetPlayerInterior(ownerid); worldid = GetPlayerVirtualWorld(ownerid);
            hx = 0.75; hy = 0.75;
        }
        case OXTARGET_ENTITY_ACTOR:
        {
            if (!IsValidActor(entityid) || !GetActorPos(entityid, x, y, z)) return 0;
            GetActorFacingAngle(entityid, rz); worldid = GetActorVirtualWorld(entityid);
            hx = 0.35; hy = 1.7;
        }
#if defined _streamer_included
        case OXTARGET_ENTITY_DYNAMIC_OBJECT:
        {
            if (!IsValidDynamicObject(entityid) || !GetDynamicObjectPos(entityid, x, y, z)) return 0;
            GetDynamicObjectRot(entityid, rx, ry, rz);
            modelid = Streamer_GetIntData(STREAMER_TYPE_OBJECT, entityid, E_STREAMER_MODEL_ID);
            interiorid = Streamer_GetIntData(STREAMER_TYPE_OBJECT, entityid, E_STREAMER_INTERIOR_ID);
            worldid = Streamer_GetIntData(STREAMER_TYPE_OBJECT, entityid, E_STREAMER_WORLD_ID);
            hx = 0.75; hy = 0.75;
        }
        case OXTARGET_ENTITY_DYNAMIC_ACTOR:
        {
            if (!IsValidDynamicActor(entityid) || !GetDynamicActorPos(entityid, x, y, z)) return 0;
            GetDynamicActorFacingAngle(entityid, rz);
            modelid = Streamer_GetIntData(STREAMER_TYPE_ACTOR, entityid, E_STREAMER_MODEL_ID);
            interiorid = Streamer_GetIntData(STREAMER_TYPE_ACTOR, entityid, E_STREAMER_INTERIOR_ID);
            worldid = GetDynamicActorVirtualWorld(entityid);
            hx = 0.35; hy = 1.7;
        }
        case OXTARGET_ENTITY_DYNAMIC_PICKUP:
        {
            if (!IsValidDynamicPickup(entityid)) return 0;
            if (!Streamer_GetFloatData(STREAMER_TYPE_PICKUP, entityid, E_STREAMER_X, x) ||
                !Streamer_GetFloatData(STREAMER_TYPE_PICKUP, entityid, E_STREAMER_Y, y) ||
                !Streamer_GetFloatData(STREAMER_TYPE_PICKUP, entityid, E_STREAMER_Z, z)) return 0;
            modelid = Streamer_GetIntData(STREAMER_TYPE_PICKUP, entityid, E_STREAMER_MODEL_ID);
            interiorid = Streamer_GetIntData(STREAMER_TYPE_PICKUP, entityid, E_STREAMER_INTERIOR_ID);
            worldid = Streamer_GetIntData(STREAMER_TYPE_PICKUP, entityid, E_STREAMER_WORLD_ID);
            hx = 0.85; hy = 0.85;
        }
#endif
        default: return 0;
    }
    return 1;
}

forward OxT_InternalWorldRay(Float:sx, Float:sy, Float:sz, Float:ex, Float:ey, Float:ez,
    &Float:x, &Float:y, &Float:z, &Float:nx, &Float:ny, &Float:nz);
public OxT_InternalWorldRay(Float:sx, Float:sy, Float:sz, Float:ex, Float:ey, Float:ez,
    &Float:x, &Float:y, &Float:z, &Float:nx, &Float:ny, &Float:nz)
{
#if defined _colandreas_included
    return CA_RayCastLineNormal(sx, sy, sz, ex, ey, ez, x, y, z, nx, ny, nz);
#else
    #pragma unused sx, sy, sz, ex, ey, ez, x, y, z, nx, ny, nz
    return 0;
#endif
}

// OxTarget Built-in style Interactive TextDraw UI
#if !defined OXTARGET_CUSTOM_UI

#define OXTARGET_MAX_DISPLAY_OPTIONS (5)

#if !defined OXTARGET_UI_X
    #define OXTARGET_UI_X (330.0)
#endif
#if !defined OXTARGET_UI_Y
    #define OXTARGET_UI_Y (226.0)
#endif

#if defined PlayerTextDrawColour
    #define OxT_PlayerTextDrawColor PlayerTextDrawColour
#else
    #define OxT_PlayerTextDrawColor PlayerTextDrawColor
#endif

#if defined PlayerTextDrawBackgroundColour
    #define OxT_PlayerTextDrawBgColor PlayerTextDrawBackgroundColour
#else
    #define OxT_PlayerTextDrawBgColor PlayerTextDrawBackgroundColor
#endif

static bool:s_OxUI_Created[MAX_PLAYERS];
static bool:s_OxUI_Visible[MAX_PLAYERS];
static bool:s_OxUI_CursorActive[MAX_PLAYERS];
static OxTarget:s_OxUI_Target[MAX_PLAYERS];
static s_OxUI_Count[MAX_PLAYERS];
static OxTargetOption:s_OxUI_OptionMap[MAX_PLAYERS][OXTARGET_MAX_DISPLAY_OPTIONS];

static PlayerText:s_OxUI_Reticle[MAX_PLAYERS] = { PlayerText:INVALID_TEXT_DRAW, ... };
static PlayerText:s_OxUI_CardBg[MAX_PLAYERS][OXTARGET_MAX_DISPLAY_OPTIONS];
static PlayerText:s_OxUI_BadgeBox[MAX_PLAYERS][OXTARGET_MAX_DISPLAY_OPTIONS];
static PlayerText:s_OxUI_BadgeText[MAX_PLAYERS][OXTARGET_MAX_DISPLAY_OPTIONS];
static PlayerText:s_OxUI_CardLabel[MAX_PLAYERS][OXTARGET_MAX_DISPLAY_OPTIONS];
static PlayerText:s_OxUI_CardIcon[MAX_PLAYERS][OXTARGET_MAX_DISPLAY_OPTIONS];

stock OxTarget_UI_Create(playerid)
{
    if (s_OxUI_Created[playerid]) return 1;

    // Cyan Reticle Dot centered right on the crosshair/object (320, 240)
    s_OxUI_Reticle[playerid] = CreatePlayerTextDraw(playerid, 316.0, 236.0, "LD_BEAT:chit");
    PlayerTextDrawTextSize(playerid, s_OxUI_Reticle[playerid], 8.0, 8.0);
    PlayerTextDrawAlignment(playerid, s_OxUI_Reticle[playerid], TEXT_DRAW_ALIGN_LEFT);
    OxT_PlayerTextDrawColor(playerid, s_OxUI_Reticle[playerid], 0x38E1FFFF);
    PlayerTextDrawSetShadow(playerid, s_OxUI_Reticle[playerid], 0);
    PlayerTextDrawSetOutline(playerid, s_OxUI_Reticle[playerid], 0);
    OxT_PlayerTextDrawBgColor(playerid, s_OxUI_Reticle[playerid], 255);
    PlayerTextDrawFont(playerid, s_OxUI_Reticle[playerid], TEXT_DRAW_FONT_SPRITE_DRAW);
    PlayerTextDrawSetProportional(playerid, s_OxUI_Reticle[playerid], true);
    PlayerTextDrawSetSelectable(playerid, s_OxUI_Reticle[playerid], false);

    for (new i = 0; i < OXTARGET_MAX_DISPLAY_OPTIONS; i++)
    {
        new Float:y = OXTARGET_UI_Y + (float(i) * 19.0);

        // 1. Background Bar (Selectable)
        s_OxUI_CardBg[playerid][i] = CreatePlayerTextDraw(playerid, OXTARGET_UI_X, y, "LD_SPAC:white");
        PlayerTextDrawTextSize(playerid, s_OxUI_CardBg[playerid][i], 115.0, 16.0);
        PlayerTextDrawAlignment(playerid, s_OxUI_CardBg[playerid][i], TEXT_DRAW_ALIGN_LEFT);
        OxT_PlayerTextDrawColor(playerid, s_OxUI_CardBg[playerid][i], 0x141414E6);
        PlayerTextDrawSetShadow(playerid, s_OxUI_CardBg[playerid][i], 0);
        PlayerTextDrawSetOutline(playerid, s_OxUI_CardBg[playerid][i], 0);
        OxT_PlayerTextDrawBgColor(playerid, s_OxUI_CardBg[playerid][i], 255);
        PlayerTextDrawFont(playerid, s_OxUI_CardBg[playerid][i], TEXT_DRAW_FONT_SPRITE_DRAW);
        PlayerTextDrawSetProportional(playerid, s_OxUI_CardBg[playerid][i], true);
        PlayerTextDrawSetSelectable(playerid, s_OxUI_CardBg[playerid][i], true);

        // 2. Number Badge Box
        s_OxUI_BadgeBox[playerid][i] = CreatePlayerTextDraw(playerid, OXTARGET_UI_X + 2.0, y + 2.0, "LD_SPAC:white");
        PlayerTextDrawTextSize(playerid, s_OxUI_BadgeBox[playerid][i], 12.0, 12.0);
        PlayerTextDrawAlignment(playerid, s_OxUI_BadgeBox[playerid][i], TEXT_DRAW_ALIGN_LEFT);
        OxT_PlayerTextDrawColor(playerid, s_OxUI_BadgeBox[playerid][i], 0x2C2C2CEE);
        PlayerTextDrawSetShadow(playerid, s_OxUI_BadgeBox[playerid][i], 0);
        PlayerTextDrawSetOutline(playerid, s_OxUI_BadgeBox[playerid][i], 0);
        OxT_PlayerTextDrawBgColor(playerid, s_OxUI_BadgeBox[playerid][i], 255);
        PlayerTextDrawFont(playerid, s_OxUI_BadgeBox[playerid][i], TEXT_DRAW_FONT_SPRITE_DRAW);
        PlayerTextDrawSetProportional(playerid, s_OxUI_BadgeBox[playerid][i], true);
        PlayerTextDrawSetSelectable(playerid, s_OxUI_BadgeBox[playerid][i], true);

        // 3. Number Badge Text
        s_OxUI_BadgeText[playerid][i] = CreatePlayerTextDraw(playerid, OXTARGET_UI_X + 8.0, y + 2.5, "Y");
        PlayerTextDrawLetterSize(playerid, s_OxUI_BadgeText[playerid][i], 0.160, 0.850);
        PlayerTextDrawAlignment(playerid, s_OxUI_BadgeText[playerid][i], TEXT_DRAW_ALIGN_CENTRE);
        OxT_PlayerTextDrawColor(playerid, s_OxUI_BadgeText[playerid][i], -1);
        PlayerTextDrawSetShadow(playerid, s_OxUI_BadgeText[playerid][i], 0);
        PlayerTextDrawSetOutline(playerid, s_OxUI_BadgeText[playerid][i], 0);
        OxT_PlayerTextDrawBgColor(playerid, s_OxUI_BadgeText[playerid][i], 255);
        PlayerTextDrawFont(playerid, s_OxUI_BadgeText[playerid][i], TEXT_DRAW_FONT_1);
        PlayerTextDrawSetProportional(playerid, s_OxUI_BadgeText[playerid][i], true);
        PlayerTextDrawSetSelectable(playerid, s_OxUI_BadgeText[playerid][i], true);

        // 4. Label Text
        s_OxUI_CardLabel[playerid][i] = CreatePlayerTextDraw(playerid, OXTARGET_UI_X + 18.0, y + 3.0, "Option");
        PlayerTextDrawLetterSize(playerid, s_OxUI_CardLabel[playerid][i], 0.180, 0.850);
        PlayerTextDrawTextSize(playerid, s_OxUI_CardLabel[playerid][i], OXTARGET_UI_X + 100.0, 14.0);
        PlayerTextDrawAlignment(playerid, s_OxUI_CardLabel[playerid][i], TEXT_DRAW_ALIGN_LEFT);
        OxT_PlayerTextDrawColor(playerid, s_OxUI_CardLabel[playerid][i], -1);
        PlayerTextDrawSetShadow(playerid, s_OxUI_CardLabel[playerid][i], 0);
        PlayerTextDrawSetOutline(playerid, s_OxUI_CardLabel[playerid][i], 0);
        OxT_PlayerTextDrawBgColor(playerid, s_OxUI_CardLabel[playerid][i], 255);
        PlayerTextDrawFont(playerid, s_OxUI_CardLabel[playerid][i], TEXT_DRAW_FONT_1);
        PlayerTextDrawSetProportional(playerid, s_OxUI_CardLabel[playerid][i], true);
        PlayerTextDrawSetSelectable(playerid, s_OxUI_CardLabel[playerid][i], true);

        // 5. Icon Sprite
        s_OxUI_CardIcon[playerid][i] = CreatePlayerTextDraw(playerid, OXTARGET_UI_X + 101.0, y + 3.0, "HUD:radar_light");
        PlayerTextDrawTextSize(playerid, s_OxUI_CardIcon[playerid][i], 10.0, 10.0);
        PlayerTextDrawAlignment(playerid, s_OxUI_CardIcon[playerid][i], TEXT_DRAW_ALIGN_LEFT);
        OxT_PlayerTextDrawColor(playerid, s_OxUI_CardIcon[playerid][i], -1);
        PlayerTextDrawSetShadow(playerid, s_OxUI_CardIcon[playerid][i], 0);
        PlayerTextDrawSetOutline(playerid, s_OxUI_CardIcon[playerid][i], 0);
        OxT_PlayerTextDrawBgColor(playerid, s_OxUI_CardIcon[playerid][i], 255);
        PlayerTextDrawFont(playerid, s_OxUI_CardIcon[playerid][i], TEXT_DRAW_FONT_SPRITE_DRAW);
        PlayerTextDrawSetProportional(playerid, s_OxUI_CardIcon[playerid][i], true);
        PlayerTextDrawSetSelectable(playerid, s_OxUI_CardIcon[playerid][i], true);
    }

    s_OxUI_Created[playerid] = true;
    s_OxUI_Visible[playerid] = false;
    s_OxUI_CursorActive[playerid] = false;
    s_OxUI_Target[playerid] = INVALID_OX_TARGET;
    s_OxUI_Count[playerid] = 0;
    for (new i = 0; i < OXTARGET_MAX_DISPLAY_OPTIONS; i++)
    {
        s_OxUI_OptionMap[playerid][i] = INVALID_OX_OPTION;
    }
    return 1;
}

stock OxTarget_UI_Destroy(playerid)
{
    if (!s_OxUI_Created[playerid]) return 1;

    if (s_OxUI_Reticle[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, s_OxUI_Reticle[playerid]);
        s_OxUI_Reticle[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    for (new i = 0; i < OXTARGET_MAX_DISPLAY_OPTIONS; i++)
    {
        if (s_OxUI_CardBg[playerid][i] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, s_OxUI_CardBg[playerid][i]);
            s_OxUI_CardBg[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
        }
        if (s_OxUI_BadgeBox[playerid][i] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, s_OxUI_BadgeBox[playerid][i]);
            s_OxUI_BadgeBox[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
        }
        if (s_OxUI_BadgeText[playerid][i] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, s_OxUI_BadgeText[playerid][i]);
            s_OxUI_BadgeText[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
        }
        if (s_OxUI_CardLabel[playerid][i] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, s_OxUI_CardLabel[playerid][i]);
            s_OxUI_CardLabel[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
        }
        if (s_OxUI_CardIcon[playerid][i] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, s_OxUI_CardIcon[playerid][i]);
            s_OxUI_CardIcon[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
        }
        s_OxUI_OptionMap[playerid][i] = INVALID_OX_OPTION;
    }

    s_OxUI_Created[playerid] = false;
    s_OxUI_Visible[playerid] = false;
    s_OxUI_CursorActive[playerid] = false;
    s_OxUI_Target[playerid] = INVALID_OX_TARGET;
    s_OxUI_Count[playerid] = 0;
    return 1;
}

stock OxTarget_ShowUI(playerid, OxTarget:target)
{
    if (!IsPlayerConnected(playerid)) return 0;
    if (!s_OxUI_Created[playerid]) OxTarget_UI_Create(playerid);

    new total = OxTarget_GetOptionCount(target);
    new OxTargetEntityType:etype = OxTarget_GetType(target);
    new bool:filtering = OxTarget_IsZoneFiltering(target);
    new OxZone:cur_zone = filtering ? OxTarget_GetPlayerZone(playerid) : OX_ZONE_ALL;
    new count = 0;

    for (new i = 0; i < total && count < OXTARGET_MAX_DISPLAY_OPTIONS; i++)
    {
        new OxTargetOption:opt = OxTarget_GetOption(target, i);
        if (opt == INVALID_OX_OPTION || !OxTarget_IsOptionEnabled(opt)) continue;

        if (filtering)
        {
            new OxZone:opt_zone = OxTarget_GetOptionZone(opt);
            if (opt_zone != OX_ZONE_ALL && opt_zone != cur_zone)
            {
                if (opt_zone == OX_ZONE_SIDES || opt_zone == OX_ZONE_DOORS)
                {
                    if (cur_zone != OX_ZONE_LEFT && cur_zone != OX_ZONE_RIGHT &&
                        cur_zone != OX_ZONE_DOOR_DRIVER && cur_zone != OX_ZONE_DOOR_PASSENGER)
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
        }

        s_OxUI_OptionMap[playerid][count] = opt;
        count++;
    }

    if (count == 0)
    {
        OxTarget_HideUI(playerid);
        return 0;
    }

    s_OxUI_Target[playerid] = target;
    s_OxUI_Visible[playerid] = true;
    s_OxUI_Count[playerid] = count;

    PlayerTextDrawShow(playerid, s_OxUI_Reticle[playerid]);

    for (new i = 0; i < count; i++)
    {
        new OxTargetOption:opt = s_OxUI_OptionMap[playerid][i];
        new opt_name[64], opt_icon[32];
        OxTarget_GetOptionName(opt, opt_name, sizeof(opt_name));
        OxTarget_GetOptionIcon(opt, opt_icon, sizeof(opt_icon));

        if (!opt_icon[0])
        {
            if (etype == OXTARGET_ENTITY_VEHICLE) format(opt_icon, sizeof(opt_icon), "HUD:radar_impound");
            else if (etype == OXTARGET_ENTITY_PLAYER || etype == OXTARGET_ENTITY_ACTOR || etype == OXTARGET_ENTITY_DYNAMIC_ACTOR) format(opt_icon, sizeof(opt_icon), "HUD:radar_centre");
            else format(opt_icon, sizeof(opt_icon), "HUD:radar_light");
        }

        new badge_str[4];
        if (i == 0) format(badge_str, sizeof(badge_str), "Y");
        else format(badge_str, sizeof(badge_str), "%d", i + 1);

        PlayerTextDrawSetString(playerid, s_OxUI_BadgeText[playerid][i], badge_str);
        PlayerTextDrawSetString(playerid, s_OxUI_CardLabel[playerid][i], opt_name);
        PlayerTextDrawSetString(playerid, s_OxUI_CardIcon[playerid][i], opt_icon);

        PlayerTextDrawShow(playerid, s_OxUI_CardBg[playerid][i]);
        PlayerTextDrawShow(playerid, s_OxUI_BadgeBox[playerid][i]);
        PlayerTextDrawShow(playerid, s_OxUI_BadgeText[playerid][i]);
        PlayerTextDrawShow(playerid, s_OxUI_CardLabel[playerid][i]);
        PlayerTextDrawShow(playerid, s_OxUI_CardIcon[playerid][i]);
    }

    for (new i = count; i < OXTARGET_MAX_DISPLAY_OPTIONS; i++)
    {
        s_OxUI_OptionMap[playerid][i] = INVALID_OX_OPTION;
        PlayerTextDrawHide(playerid, s_OxUI_CardBg[playerid][i]);
        PlayerTextDrawHide(playerid, s_OxUI_BadgeBox[playerid][i]);
        PlayerTextDrawHide(playerid, s_OxUI_BadgeText[playerid][i]);
        PlayerTextDrawHide(playerid, s_OxUI_CardLabel[playerid][i]);
        PlayerTextDrawHide(playerid, s_OxUI_CardIcon[playerid][i]);
    }

    // Automatic cursor / touch activation (PC mouse and Android touch mode)
    if (!s_OxUI_CursorActive[playerid])
    {
        s_OxUI_CursorActive[playerid] = true;
        SelectTextDraw(playerid, 0x38E1FFAA);
    }
    return 1;
}

stock OxTarget_HideUI(playerid)
{
    if (!IsPlayerConnected(playerid) || !s_OxUI_Created[playerid]) return 0;
    if (!s_OxUI_Visible[playerid]) return 1;

    s_OxUI_Visible[playerid] = false;
    s_OxUI_Target[playerid] = INVALID_OX_TARGET;
    s_OxUI_Count[playerid] = 0;

    PlayerTextDrawHide(playerid, s_OxUI_Reticle[playerid]);
    for (new i = 0; i < OXTARGET_MAX_DISPLAY_OPTIONS; i++)
    {
        s_OxUI_OptionMap[playerid][i] = INVALID_OX_OPTION;
        PlayerTextDrawHide(playerid, s_OxUI_CardBg[playerid][i]);
        PlayerTextDrawHide(playerid, s_OxUI_BadgeBox[playerid][i]);
        PlayerTextDrawHide(playerid, s_OxUI_BadgeText[playerid][i]);
        PlayerTextDrawHide(playerid, s_OxUI_CardLabel[playerid][i]);
        PlayerTextDrawHide(playerid, s_OxUI_CardIcon[playerid][i]);
    }

    if (s_OxUI_CursorActive[playerid])
    {
        s_OxUI_CursorActive[playerid] = false;
        CancelSelectTextDraw(playerid);
    }
    return 1;
}

stock bool:OxTarget_IsUIOpen(playerid)
{
    return (IsPlayerConnected(playerid) && s_OxUI_Visible[playerid]);
}

stock OxTarget_ToggleCursor(playerid)
{
    if (!IsPlayerConnected(playerid) || !s_OxUI_Visible[playerid]) return 0;
    s_OxUI_CursorActive[playerid] = !s_OxUI_CursorActive[playerid];
    if (s_OxUI_CursorActive[playerid])
    {
        SelectTextDraw(playerid, 0x38E1FFAA);
    }
    else
    {
        CancelSelectTextDraw(playerid);
    }
    return 1;
}

// Internal callbacks invoked from C++ plugin
forward OxT_InternalEnter(playerid, OxTarget:target);
public OxT_InternalEnter(playerid, OxTarget:target)
{
    OxTarget_ShowUI(playerid, target);
    return 1;
}

forward OxT_InternalLeave(playerid, OxTarget:target);
public OxT_InternalLeave(playerid, OxTarget:target)
{
    #pragma unused target
    OxTarget_HideUI(playerid);
    return 1;
}

forward OxT_InternalZoneChange(playerid, OxTarget:target, old_zone, new_zone);
public OxT_InternalZoneChange(playerid, OxTarget:target, old_zone, new_zone)
{
    #pragma unused old_zone, new_zone
    if (s_OxUI_Visible[playerid] && s_OxUI_Target[playerid] == target)
    {
        OxTarget_ShowUI(playerid, target);
    }
    return 1;
}

forward OxT_InternalSelect(playerid, OxTarget:target, OxTargetOption:option, option_id);
public OxT_InternalSelect(playerid, OxTarget:target, OxTargetOption:option, option_id)
{
    #pragma unused target, option, option_id
    OxTarget_HideUI(playerid);
    return 1;
}

// Click dispatcher callable from gamemode hooks
forward OxTarget_OnClickPlayerTD(playerid, PlayerText:playertextid);
public OxTarget_OnClickPlayerTD(playerid, PlayerText:playertextid)
{
    if (s_OxUI_Visible[playerid] && s_OxUI_Target[playerid] != INVALID_OX_TARGET)
    {
        for (new i = 0; i < s_OxUI_Count[playerid]; i++)
        {
            if (playertextid == s_OxUI_CardBg[playerid][i] ||
                playertextid == s_OxUI_CardLabel[playerid][i] ||
                playertextid == s_OxUI_BadgeBox[playerid][i] ||
                playertextid == s_OxUI_BadgeText[playerid][i] ||
                playertextid == s_OxUI_CardIcon[playerid][i])
            {
                new OxTargetOption:opt = s_OxUI_OptionMap[playerid][i];
                OxTarget_HideUI(playerid);
                OxTarget_Select(playerid, opt);
                return 1;
            }
        }
    }
    return 0;
}

#if !defined KEY
    #define KEY: _:
#endif

// ALS Hooking for public multi-gamemode compatibility
public OnPlayerConnect(playerid)
{
    OxTarget_UI_Create(playerid);
    #if defined OxT_OnPlayerConnect
        return OxT_OnPlayerConnect(playerid);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnPlayerConnect
    #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect OxT_OnPlayerConnect
#if defined OxT_OnPlayerConnect
    forward OxT_OnPlayerConnect(playerid);
#endif

public OnPlayerDisconnect(playerid, reason)
{
    OxTarget_UI_Destroy(playerid);
    #if defined OxT_OnPlayerDisconnect
        return OxT_OnPlayerDisconnect(playerid, reason);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnPlayerDisconnect
    #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect OxT_OnPlayerDisconnect
#if defined OxT_OnPlayerDisconnect
    forward OxT_OnPlayerDisconnect(playerid, reason);
#endif

public OnPlayerKeyStateChange(playerid, KEY:newkeys, KEY:oldkeys)
{
    if ((_:(newkeys) & _:KEY_YES) && !(_:(oldkeys) & _:KEY_YES))
    {
        if (s_OxUI_Visible[playerid] && s_OxUI_Count[playerid] > 0)
        {
            new OxTargetOption:opt = s_OxUI_OptionMap[playerid][0];
            OxTarget_HideUI(playerid);
            OxTarget_Select(playerid, opt);
        }
    }
    else if ((_:(newkeys) & _:KEY_CTRL_BACK) && !(_:(oldkeys) & _:KEY_CTRL_BACK))
    {
        if (s_OxUI_Visible[playerid])
        {
            OxTarget_ToggleCursor(playerid);
        }
    }
    #if defined OxT_OnPlayerKeyStateChange
        return OxT_OnPlayerKeyStateChange(playerid, newkeys, oldkeys);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnPlayerKeyStateChange
    #undef OnPlayerKeyStateChange
#else
    #define _ALS_OnPlayerKeyStateChange
#endif
#define OnPlayerKeyStateChange OxT_OnPlayerKeyStateChange
#if defined OxT_OnPlayerKeyStateChange
    forward OxT_OnPlayerKeyStateChange(playerid, KEY:newkeys, KEY:oldkeys);
#endif

public OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
    if (OxTarget_OnClickPlayerTD(playerid, playertextid))
    {
        return 1;
    }
    #if defined OxT_OnPlayerClickPlayerTD
        return OxT_OnPlayerClickPlayerTD(playerid, playertextid);
    #else
        return 0;
    #endif
}
#if defined _ALS_OnPlayerClickPlayerTD
    #undef OnPlayerClickPlayerTextDraw
#else
    #define _ALS_OnPlayerClickPlayerTD
#endif
#define OnPlayerClickPlayerTextDraw OxT_OnPlayerClickPlayerTD
#if defined OxT_OnPlayerClickPlayerTD
    forward OxT_OnPlayerClickPlayerTD(playerid, PlayerText:playertextid);
#endif

public OnPlayerClickTextDraw(playerid, Text:clickedid)
{
    if (clickedid == Text:INVALID_TEXT_DRAW && s_OxUI_Visible[playerid])
    {
        OxTarget_HideUI(playerid);
    }
    #if defined OxT_OnPlayerClickTextDraw
        return OxT_OnPlayerClickTextDraw(playerid, clickedid);
    #else
        return 0;
    #endif
}
#if defined _ALS_OnPlayerClickTextDraw
    #undef OnPlayerClickTextDraw
#else
    #define _ALS_OnPlayerClickTextDraw
#endif
#define OnPlayerClickTextDraw OxT_OnPlayerClickTextDraw
#if defined OxT_OnPlayerClickTextDraw
    forward OxT_OnPlayerClickTextDraw(playerid, Text:clickedid);
#endif

#endif // !defined OXTARGET_CUSTOM_UI

