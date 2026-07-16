// Copyright StormBreaker Games. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class StormBreaker : ModuleRules
{
    public StormBreaker(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        IWYUSupport = IWYUSupport.Full;

        // Register module root and all subdirectories as include paths
        PublicIncludePaths.AddRange(new string[]
        {
            ModuleDirectory,
            Path.Combine(ModuleDirectory, "Core"),
            Path.Combine(ModuleDirectory, "Character"),
            Path.Combine(ModuleDirectory, "Weapon"),
            Path.Combine(ModuleDirectory, "Inventory"),
            Path.Combine(ModuleDirectory, "BattleRoyale"),
            Path.Combine(ModuleDirectory, "Multiplayer"),
            Path.Combine(ModuleDirectory, "AI"),
            Path.Combine(ModuleDirectory, "UI"),
            Path.Combine(ModuleDirectory, "Vehicle"),
            Path.Combine(ModuleDirectory, "Backend"),
            Path.Combine(ModuleDirectory, "Subsystems")
        });

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
            "HTTP",
            "Json",
            "JsonUtilities",
            "Media",
            "MediaAssets"
        });
    }
}
