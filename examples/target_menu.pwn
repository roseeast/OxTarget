// Define OXTARGET_CUSTOM_UI before including oxtarget to disable the built-in TextDraw UI
// This is ideal if you prefer SA-MP Dialogs, custom TextDraws, or CEF / browser radial menus:
#define OXTARGET_CUSTOM_UI

#include <a_samp>
#include <oxtarget>

#define OXTARGET_DIALOG_ID (24190)

// Store the active target handle when the dialog is opened
new OxTarget:gPlayerMenuTarget[MAX_PLAYERS] = {INVALID_OX_TARGET, ...};

// When player looks at a valid target, prompt them to press a key (e.g. KEY_NO / N)
public OnPlayerOxTargetEnter(playerid, OxTarget:target)
{
    #pragma unused target
    GameTextForPlayer(playerid, "~y~Press ~w~N ~y~to Interact", 2000, 4);
    return 1;
}

public OnPlayerOxTargetLeave(playerid, OxTarget:target)
{
    #pragma unused target
    // Hide game text or close dialog if target is lost
    GameTextForPlayer(playerid, " ", 10, 4);
    return 1;
}

// Show a dialog listing all options on the active target
stock ShowTargetDialog(playerid)
{
    new OxTarget:target = OxTarget_GetPlayerTarget(playerid);
    if (target == INVALID_OX_TARGET)
    {
        SendClientMessage(playerid, 0xFF0000FF, "No active target in your camera view!");
        return 0;
    }

    new total = OxTarget_GetOptionCount(target);
    if (total <= 0) return 0;

    new body[1024], name[64];
    body[0] = '\0';

    for (new i = 0; i < total; i++)
    {
        new OxTargetOption:option = OxTarget_GetOption(target, i);
        if (!OxTarget_IsOptionEnabled(option)) continue;

        OxTarget_GetOptionName(option, name, sizeof(name));
        format(body, sizeof(body), "%s%d. %s\n", body, i + 1, name);
    }

    gPlayerMenuTarget[playerid] = target;
    ShowPlayerDialog(playerid, OXTARGET_DIALOG_ID, DIALOG_STYLE_LIST, "Interaction Menu", body, "Select", "Cancel");
    return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused inputtext
    if (dialogid == OXTARGET_DIALOG_ID)
    {
        if (response && gPlayerMenuTarget[playerid] != INVALID_OX_TARGET)
        {
            // Execute the selected option deterministically:
            OxTarget_SelectTargetOption(playerid, gPlayerMenuTarget[playerid], listitem);
        }
        gPlayerMenuTarget[playerid] = INVALID_OX_TARGET;
        return 1;
    }
    return 0;
}

#if defined _INC_open_mp
public OnPlayerKeyStateChange(playerid, KEY:newkeys, KEY:oldkeys)
#else
public OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
#endif
{
    #pragma unused oldkeys
    // Press 'N' key (KEY_NO) to open the interaction dialog
    if (newkeys & KEY_NO)
    {
        ShowTargetDialog(playerid);
    }
    return 1;
}

public OnPlayerOxTargetSelect(playerid, OxTarget:target, OxTargetOption:option, option_id)
{
    #pragma unused target, option
    new msg[80];
    format(msg, sizeof(msg), "Successfully selected option ID: %d", option_id);
    SendClientMessage(playerid, 0x00FF00FF, msg);
    return 1;
}
