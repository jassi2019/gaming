// Copyright StormBreaker Games. All Rights Reserved.

using UnrealBuildTool;

public class StormBreakerServer : ModuleRules
{
    public StormBreakerServer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "StormBreaker",
            "OnlineSubsystem",
            "OnlineSubsystemEOS"
        });
    }
}
