#include <a_samp>
#include <oxtarget>

// Option identifiers
enum {
    ATM_ACTION_WITHDRAW = 1,
    ATM_ACTION_BALANCE,
    ATM_ACTION_REPAIR,
    ATM_ACTION_INFO,
    DOOR_ACTION_ENTER
};

new OxTarget:gAtmTarget;
new OxTarget:gDoorTarget;

public OnGameModeInit()
{
    // 1. Create a static ATM object (model 2942)
    new objectid = CreateObject(2942, 1481.0, -1720.0, 13.5, 0.0, 0.0, 0.0);
    gAtmTarget = OxTarget_AddObject(objectid);
    OxTarget_SetDistance(gAtmTarget, 3.0); // 3.0 meters interaction range

    // Enable Universal Zone Filtering on this object
    // Options will appear dynamically based on which side the player looks at:
    OxTarget_SetZoneFiltering(gAtmTarget, true);

    // Front of ATM (+Y face): Screen & keypad options
    OxTarget_AddZoneOption(gAtmTarget, OX_ZONE_FRONT, "Withdraw Cash", "HUD:radar_cash", ATM_ACTION_WITHDRAW);
    OxTarget_AddZoneOption(gAtmTarget, OX_ZONE_FRONT, "Check Balance", "HUD:radar_light", ATM_ACTION_BALANCE);

    // Rear of ATM (-Y face): Maintenance wiring
    OxTarget_AddZoneOption(gAtmTarget, OX_ZONE_REAR, "Power / Service Panel", "HUD:radar_impound", ATM_ACTION_REPAIR);

    // General option (OX_ZONE_ALL): Appears regardless of which face is targeted
    OxTarget_AddZoneOption(gAtmTarget, OX_ZONE_ALL, "ATM Info", "HUD:radar_centre", ATM_ACTION_INFO);

    // 2. Create a Point Target (sphere volume without any object model)
    // Useful for door entrances, checkpoints, elevator call buttons, etc.
    gDoorTarget = OxTarget_AddPoint(1483.0, -1720.0, 14.0, 1.5); // 1.5m radius
    OxTarget_SetDistance(gDoorTarget, 2.5);
    OxTarget_AddOption(gDoorTarget, "Enter Building", "HUD:radar_light", DOOR_ACTION_ENTER);

    print("[OxTarget] Object & Point targets initialized successfully.");
    return 1;
}

public OnGameModeExit()
{
    if (OxTarget_IsValid(gAtmTarget)) OxTarget_Destroy(gAtmTarget);
    if (OxTarget_IsValid(gDoorTarget)) OxTarget_Destroy(gDoorTarget);
    return 1;
}

public OnPlayerOxTargetSelect(playerid, OxTarget:target, OxTargetOption:option, option_id)
{
    #pragma unused option
    if (target == gAtmTarget)
    {
        switch (option_id)
        {
            case ATM_ACTION_WITHDRAW: SendClientMessage(playerid, 0x00FF00FF, "ATM: You selected Withdraw Cash.");
            case ATM_ACTION_BALANCE:  SendClientMessage(playerid, 0x00FF00FF, "ATM: Your account balance is $25,000.");
            case ATM_ACTION_REPAIR:   SendClientMessage(playerid, 0xFFFF00FF, "ATM: Maintenance technician panel opened.");
            case ATM_ACTION_INFO:     SendClientMessage(playerid, 0x00FFFFFF, "ATM: Bank of San Andreas - Terminal #104.");
        }
    }
    else if (target == gDoorTarget)
    {
        if (option_id == DOOR_ACTION_ENTER)
        {
            SendClientMessage(playerid, 0x00FF00FF, "Door: You entered the building.");
            SetPlayerPos(playerid, 1483.0, -1715.0, 14.0);
        }
    }
    return 1;
}

