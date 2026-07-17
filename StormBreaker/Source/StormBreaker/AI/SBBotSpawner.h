// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AI/SBBotTypes.h"
#include "SBBotSpawner.generated.h"

class ASBBotController;
class ASBCharacterBase;

/**
 * World subsystem that spawns and manages AI bots for battle royale.
 * Called by GameMode to fill a match with bots.
 * Handles spawning, difficulty distribution, and cleanup.
 */
UCLASS()
class STORMBREAKER_API USBBotSpawner : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // --- Spawning ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Bots")
    void SpawnBots(int32 Count, TSubclassOf<ASBCharacterBase> PawnClass);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Bots")
    void DespawnAllBots();

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Bots")
    int32 GetAliveBotCount() const;

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Bots")
    int32 GetTotalBotCount() const { return SpawnedBots.Num(); }

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bots")
    float EasyPercent = 0.40f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bots")
    float NormalPercent = 0.35f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bots")
    float HardPercent = 0.20f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bots")
    float ProPercent = 0.05f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bots")
    float SpawnRadius = 30000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bots")
    float SpawnHeight = 200.0f;

private:
    FVector GetRandomSpawnLocation() const;
    ESBBotDifficulty GetDifficultyForIndex(int32 Index, int32 Total) const;

    UPROPERTY()
    TArray<TObjectPtr<ASBBotController>> SpawnedBots;
};
