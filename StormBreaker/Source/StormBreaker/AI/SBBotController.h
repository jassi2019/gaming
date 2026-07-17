// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/SBBotTypes.h"
#include "SBBotController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class ASBCharacterBase;
class ASBZoneManager;

/**
 * AI controller for battle royale bots.
 * Runs a state machine driven by perception, health, zone, and loot needs.
 * Optimized for 100 concurrent bots via tick throttling and distance-based LOD.
 */
UCLASS()
class STORMBREAKER_API ASBBotController : public AAIController
{
    GENERATED_BODY()

public:
    ASBBotController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bot")
    ESBBotDifficulty Difficulty = ESBBotDifficulty::Normal;

    UPROPERTY(BlueprintReadOnly, Category = "IslandOfDeath|Bot")
    FSBBotDifficultySettings DifficultySettings;

    UPROPERTY(BlueprintReadOnly, Category = "IslandOfDeath|Bot")
    ESBBotState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "IslandOfDeath|Bot")
    FSBBotMemory Memory;

    // --- Perception ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IslandOfDeath|Bot")
    TObjectPtr<UAIPerceptionComponent> PerceptionComp;

    // --- AI LOD ---

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bot|Performance")
    float FullTickDistance = 5000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bot|Performance")
    float ReducedTickDistance = 15000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bot|Performance")
    float MinTickInterval = 0.05f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Bot|Performance")
    float MaxTickInterval = 0.5f;

private:
    void SetupPerception();
    void UpdateAILOD();
    void DecideState(float DeltaTime);

    // --- State Handlers ---
    void TickIdle(float DeltaTime);
    void TickLooting(float DeltaTime);
    void TickRotating(float DeltaTime);
    void TickEngaging(float DeltaTime);
    void TickHealing(float DeltaTime);
    void TickFleeing(float DeltaTime);
    void TickTakingCover(float DeltaTime);

    // --- Combat ---
    void AimAtTarget(float DeltaTime);
    void FireAtTarget();
    void StopFiring();
    FVector CalculateAimPoint(AActor* Target) const;

    // --- Navigation ---
    void MoveToLocation(const FVector& Location);
    void MoveToSafeZone();
    FVector FindCoverLocation(const FVector& ThreatDirection) const;
    FVector FindLootLocation() const;

    // --- Perception Callbacks ---
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // --- Helpers ---
    ASBCharacterBase* GetBotCharacter() const;
    float GetDistanceToNearestPlayer() const;
    bool IsInSafeZone() const;
    bool NeedsHealing() const;
    bool NeedsLoot() const;
    bool HasWeapon() const;
    float GetHealthPercent() const;

    // --- State ---
    float StateTimer;
    float FireTimer;
    float AimJitterTimer;
    FVector AimJitterOffset;
    bool bIsFiring;

    UPROPERTY()
    TWeakObjectPtr<AActor> CurrentTarget;

    UPROPERTY()
    TWeakObjectPtr<ASBZoneManager> CachedZoneManager;
};
