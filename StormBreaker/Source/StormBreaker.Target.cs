// Copyright StormBreaker Games. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class StormBreakerTarget : TargetRules
{
    public StormBreakerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        ExtraModuleNames.AddRange(new string[] { "StormBreaker" });
    }
}
