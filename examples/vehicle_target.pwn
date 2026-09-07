#include <a_samp>
#include <oxtarget>

enum { VEHICLE_HOOD = 1, VEHICLE_TRUNK, VEHICLE_INSPECT };
new OxTarget:gVehicleTarget[MAX_VEHICLES];

public OnGameModeInit()
{
    new vehicleid = CreateVehicle(411, 1480.0, -1720.0, 13.5, 90.0, -1, -1, -1);
    gVehicleTarget[vehicleid] = OxTarget_AddVehicle(vehicleid);
    OxTarget_SetDistance(gVehicleTarget[vehicleid], 4.0);

    // Enable zone filtering so options appear dynamically based on where the player aims their camera!
    // (By default, zone filtering is false and all options appear at once)
    OxTarget_SetZoneFiltering(gVehicleTarget[vehicleid], true);

    // Aiming at the front of the vehicle shows "Open Hood"
    OxTarget_AddZoneOption(gVehicleTarget[vehicleid], OX_ZONE_HOOD, "Open Hood", "HUD:radar_impound", VEHICLE_HOOD);

    // Aiming at the back of the vehicle shows "Open Trunk"
    OxTarget_AddZoneOption(gVehicleTarget[vehicleid], OX_ZONE_TRUNK, "Open Trunk", "HUD:radar_impound", VEHICLE_TRUNK);

    // General options (OX_ZONE_ALL) appear on all parts of the vehicle
    OxTarget_AddZoneOption(gVehicleTarget[vehicleid], OX_ZONE_ALL, "Inspect", "HUD:radar_light", VEHICLE_INSPECT);
    return 1;
}

public OnPlayerOxTargetEnter(playerid, OxTarget:target)
{
    // The built-in style UI automatically opens with cursor on PC and touch on Android!
    SendClientMessage(playerid, 0x80FF80FF, "Target ready. Click an option, tap on screen, or press Y for option 1.");
    return 1;
}

public OnPlayerOxTargetSelect(playerid, OxTarget:target, OxTargetOption:option, option_id)
{
    #pragma unused option
    new vehicleid = OxTarget_GetEntity(target);
    new message[80];
    format(message, sizeof message, "Selected option %d on vehicle %d", option_id, vehicleid);
    SendClientMessage(playerid, -1, message);
    return 1;
}

#if defined _INC_open_mp
public OnPlayerKeyStateChange(playerid, KEY:newkeys, KEY:oldkeys)
#else
public OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
#endif
{
    #pragma unused oldkeys
    if (newkeys & KEY_YES) OxTarget_SelectIndex(playerid, 0);
    return 1;
}
