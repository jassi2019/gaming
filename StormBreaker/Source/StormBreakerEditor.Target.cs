// Copyright StormBreaker Games. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class StormBreakerEditorTarget : TargetRules
{
    public StormBreakerEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        ExtraModuleNames.AddRange(new string[] { "StormBreaker" });
    }
}
