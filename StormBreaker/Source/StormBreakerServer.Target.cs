// Copyright StormBreaker Games. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class StormBreakerServerTarget : TargetRules
{
    public StormBreakerServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        ExtraModuleNames.AddRange(new string[] { "StormBreaker", "StormBreakerServer" });
    }
}
