// Copyright StormBreaker Games. All Rights Reserved.

using UnrealBuildTool;

public class StormBreaker : ModuleRules
{
    public StormBreaker(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Strict includes — no monolithic headers
        bEnforceIWYU = true;

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
            "OnlineSubsystemEOS",
            "OnlineSubsystemUtils",
            "NetCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Http",
            "Json",
            "JsonUtilities",
            "AudioModulation",
            "MetasoundEngine"
        });

        // GAS setup
        PublicIncludePaths.AddRange(new string[]
        {
            "StormBreaker/Core",
            "StormBreaker/Character",
            "StormBreaker/Weapon",
            "StormBreaker/Inventory",
            "StormBreaker/BattleRoyale",
            "StormBreaker/Multiplayer",
            "StormBreaker/AI",
            "StormBreaker/UI",
            "StormBreaker/Vehicle",
            "StormBreaker/Backend",
            "StormBreaker/Subsystems"
        });
    }
}
