// Copyright StormBreaker Games. All Rights Reserved.

using UnrealBuildTool;

public class StormBreaker : ModuleRules
{
    public StormBreaker(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        IWYUSupport = IWYUSupport.Full;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "UMG",
            "SlateCore",
            "Slate",
            "Niagara",
            "PhysicsCore",
            "ChaosVehicles",
            "NavigationSystem",
            "AIModule",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "NetCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "OnlineSubsystemEOS",
            "Http",
            "Json",
            "JsonUtilities"
        });
    }
}
