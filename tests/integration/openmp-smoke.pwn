#include <open.mp>
#include <oxtarget>

main() {}

public OnGameModeInit()
{
    new OxTarget:target = OxTarget_AddPoint(10.0, 20.0, 30.0, 0.5);
    if (target == INVALID_OX_TARGET || !OxTarget_IsValid(target))
    {
        print("[OxTarget Smoke] FAILED: point target was not created");
        SendRconCommand("exit");
        return 0;
    }
    OxTarget_AddOption(target, "Inspect", 7);
    if (OxTarget_GetActiveTargetCount() != 1 || OxTarget_GetOptionCount(target) != 1)
    {
        print("[OxTarget Smoke] FAILED: target or option count mismatch");
        SendRconCommand("exit");
        return 0;
    }
    print("[OxTarget Smoke] PASSED");
    SendRconCommand("exit");
    return 1;
}

