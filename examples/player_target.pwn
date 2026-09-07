#include <a_samp>
#include <oxtarget>

// Interaction option IDs
enum {
    PLAYER_ACTION_GREET = 1,
    PLAYER_ACTION_GIVE_CASH,
    PLAYER_ACTION_FRISK,
    PLAYER_ACTION_SHOW_ID
};

new OxTarget:gPlayerTarget[MAX_PLAYERS] = {INVALID_OX_TARGET, ...};

public OnPlayerConnect(playerid)
{
    // Create an interactive target on the connecting player so OTHER players can target them
    gPlayerTarget[playerid] = OxTarget_AddPlayer(playerid);
    OxTarget_SetDistance(gPlayerTarget[playerid], 2.5); // 2.5 meters range

    // Add interaction options
    OxTarget_AddOption(gPlayerTarget[playerid], "Handshake", "HUD:radar_centre", PLAYER_ACTION_GREET);
    OxTarget_AddOption(gPlayerTarget[playerid], "Give Cash ($50)", "HUD:radar_cash", PLAYER_ACTION_GIVE_CASH);
    OxTarget_AddOption(gPlayerTarget[playerid], "Frisk / Search", "HUD:radar_impound", PLAYER_ACTION_FRISK);
    OxTarget_AddOption(gPlayerTarget[playerid], "Show ID Card", "HUD:radar_light", PLAYER_ACTION_SHOW_ID);

    // Ensure player has OxTarget raycasting enabled
    OxTarget_EnablePlayer(playerid, true);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    #pragma unused reason
    if (OxTarget_IsValid(gPlayerTarget[playerid]))
    {
        OxTarget_Destroy(gPlayerTarget[playerid]);
    }
    gPlayerTarget[playerid] = INVALID_OX_TARGET;
    return 1;
}

// Permission / validation check:
// Return 1 to allow the option, or 0 to hide/prevent it
public OnPlayerOxTargetOptionCheck(playerid, OxTarget:target, OxTargetOption:option)
{
    new option_id = OxTarget_GetOptionData(option);

    // Example: Only players in Police skin (ID 280) can use the Frisk option
    if (option_id == PLAYER_ACTION_FRISK)
    {
        if (GetPlayerSkin(playerid) != 280)
        {
            return 0; // Regular civilians cannot search other players
        }
    }
    return 1;
}

public OnPlayerOxTargetSelect(playerid, OxTarget:target, OxTargetOption:option, option_id)
{
    #pragma unused option
    new target_playerid = OxTarget_GetEntity(target);
    if (!IsPlayerConnected(target_playerid)) return 1;

    new pName[MAX_PLAYER_NAME], tName[MAX_PLAYER_NAME], msg[128];
    GetPlayerName(playerid, pName, sizeof(pName));
    GetPlayerName(target_playerid, tName, sizeof(tName));

    switch (option_id)
    {
        case PLAYER_ACTION_GREET:
        {
            format(msg, sizeof(msg), "* %s shakes hands with %s.", pName, tName);
            SendClientMessageToAll(0xC2A2DAAA, msg);
            ApplyAnimation(playerid, "GANGS", "hndshkfa", 4.0, false, false, false, false, 0);
            ApplyAnimation(target_playerid, "GANGS", "hndshkfa", 4.0, false, false, false, false, 0);
        }
        case PLAYER_ACTION_GIVE_CASH:
        {
            if (GetPlayerMoney(playerid) < 50)
            {
                SendClientMessage(playerid, 0xFF0000FF, "You do not have enough cash ($50)!");
                return 1;
            }
            GivePlayerMoney(playerid, -50);
            GivePlayerMoney(target_playerid, 50);

            format(msg, sizeof(msg), "You gave $50 to %s.", tName);
            SendClientMessage(playerid, 0x00FF00FF, msg);
            format(msg, sizeof(msg), "%s gave you $50.", pName);
            SendClientMessage(target_playerid, 0x00FF00FF, msg);
        }
        case PLAYER_ACTION_FRISK:
        {
            format(msg, sizeof(msg), "Officer %s searches %s.", pName, tName);
            SendClientMessageToAll(0xC2A2DAAA, msg);
            format(msg, sizeof(msg), "Cash: $%d | Weapon ID: %d", GetPlayerMoney(target_playerid), GetPlayerWeapon(target_playerid));
            SendClientMessage(playerid, 0xFFFF00FF, msg);
        }
        case PLAYER_ACTION_SHOW_ID:
        {
            format(msg, sizeof(msg), "%s shows you their ID card.", pName);
            SendClientMessage(target_playerid, 0x00FFFFFF, msg);
            SendClientMessage(playerid, 0x00FFFFFF, "You showed your ID card.");
        }
    }
    return 1;
}

