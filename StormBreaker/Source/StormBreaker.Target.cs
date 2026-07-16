// Copyright StormBreaker Games. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class StormBreakerTarget : TargetRules
{
    public StormBreakerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        ExtraModuleNames.AddRange(new string[] { "StormBreaker" });

        // Android optimizations
        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            bCompileForSize = true;
        }
    }
}
