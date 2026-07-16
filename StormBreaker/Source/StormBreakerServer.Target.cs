// Copyright StormBreaker Games. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class StormBreakerServerTarget : TargetRules
{
    public StormBreakerServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;

        ExtraModuleNames.AddRange(new string[] { "StormBreaker", "StormBreakerServer" });
    }
}
