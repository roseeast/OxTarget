// NOTE: OxTarget ALREADY supports SA-MP Android out of the box!
// When using the default built-in TextDraw UI, Android players can simply TAP
// directly on the screen to select option cards.
//
// This example demonstrates how to build a CUSTOM mobile-friendly touch button
// (e.g. a dedicated large "INTERACT" button on the bottom right of the screen)
// using the `OXTARGET_CUSTOM_UI` mode:

#define OXTARGET_CUSTOM_UI

#include <a_samp>
#include <oxtarget>

new PlayerText:gInteractBtn[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...};

public OnPlayerConnect(playerid)
{
    // Create a large, easily-tappable mobile button for touch screens
    gInteractBtn[playerid] = CreatePlayerTextDraw(playerid, 500.0, 320.0, "INTERACT");
    PlayerTextDrawLetterSize(playerid, gInteractBtn[playerid], 0.45, 1.8);
    PlayerTextDrawAlignment(playerid, gInteractBtn[playerid], 2);
    PlayerTextDrawColour(playerid, gInteractBtn[playerid], 0xFFFFFFFF);
    PlayerTextDrawUseBox(playerid, gInteractBtn[playerid], true);
    PlayerTextDrawBoxColour(playerid, gInteractBtn[playerid], 0x1E1E28DD);
    PlayerTextDrawSetShadow(playerid, gInteractBtn[playerid], 0);
    PlayerTextDrawSetOutline(playerid, gInteractBtn[playerid], 1);
    PlayerTextDrawBackgroundColour(playerid, gInteractBtn[playerid], 0x000000FF);
    PlayerTextDrawFont(playerid, gInteractBtn[playerid], 2);
    PlayerTextDrawSetProportional(playerid, gInteractBtn[playerid], true);
    PlayerTextDrawTextSize(playerid, gInteractBtn[playerid], 22.0, 100.0);
    PlayerTextDrawSetSelectable(playerid, gInteractBtn[playerid], true);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    #pragma unused reason
    if (gInteractBtn[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, gInteractBtn[playerid]);
        gInteractBtn[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }
    return 1;
}

// Show the mobile button when the player aims at any valid target
public OnPlayerOxTargetEnter(playerid, OxTarget:target)
{
    #pragma unused target
    PlayerTextDrawShow(playerid, gInteractBtn[playerid]);
    // Enable touch tap selection on mobile
    SelectTextDraw(playerid, 0x38E1FFAA);
    return 1;
}

// Hide the button when player looks away
public OnPlayerOxTargetLeave(playerid, OxTarget:target)
{
    #pragma unused target
    PlayerTextDrawHide(playerid, gInteractBtn[playerid]);
    CancelSelectTextDraw(playerid);
    return 1;
}

public OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
    if (playertextid == gInteractBtn[playerid])
    {
        // Execute the first option of the currently targeted entity
        OxTarget_SelectIndex(playerid, 0);
        return 1;
    }
    return 0;
}

public OnPlayerOxTargetSelect(playerid, OxTarget:target, OxTargetOption:option, option_id)
{
    #pragma unused target, option
    new message[80];
    format(message, sizeof(message), "Mobile Touch: You selected option ID %d!", option_id);
    SendClientMessage(playerid, 0x00FF00FF, message);
    return 1;
}

