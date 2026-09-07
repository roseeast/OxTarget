#include <a_samp>
#include <oxtarget>

// Command: /ray - Casts a ray directly from the player's camera forward up to 30 meters
public OnPlayerCommandText(playerid, cmdtext[])
{
    if (!strcmp(cmdtext, "/ray", true))
    {
        new OxRay:ray;
        // Cast 30 meters from camera across all targetable masks
        if (OxTarget_CastCamera(playerid, 30.0, OXTARGET_MASK_ALL, ray))
        {
            new OxTargetHitType:hitType = OxTarget_GetHitType(ray);
            new OxTargetEntityType:entityType = OxTarget_GetHitEntityType(ray);
            new entityid = OxTarget_GetHitEntity(ray);
            new Float:hitDist = OxTarget_GetHitDistance(ray);
            new Float:hx, Float:hy, Float:hz;
            new Float:nx, Float:ny, Float:nz;
            new OxZone:zone = OxTarget_GetHitZone(ray);

            OxTarget_GetHitPosition(ray, hx, hy, hz);
            OxTarget_GetHitNormal(ray, nx, ny, nz);

            new msg[144];
            format(msg, sizeof(msg), "[Raycast Hit] Type: %d | EntityType: %d | ID: %d | Dist: %.2fm | Zone: %d",
                _:hitType, _:entityType, entityid, hitDist, _:zone);
            SendClientMessage(playerid, 0x00FF00FF, msg);

            format(msg, sizeof(msg), "[Hit Pos] (%.2f, %.2f, %.2f) | Normal: (%.2f, %.2f, %.2f)",
                hx, hy, hz, nx, ny, nz);
            SendClientMessage(playerid, 0xFFFF00FF, msg);

            // Always destroy temporary rays when done to free internal handles
            OxTarget_DestroyRay(ray);
        }
        else
        {
            SendClientMessage(playerid, 0xFF0000FF, "[Raycast] No object or entity hit within 30 meters.");
        }
        return 1;
    }
    return 0;
}
