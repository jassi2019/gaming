// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBZoneManager.generated.h"

USTRUCT(BlueprintType)
struct FSBZonePhase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float RadiusMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float WaitDuration = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ShrinkDuration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float DamagePerSecond = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float CenterShiftFactor = 0.25f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZonePhaseChanged, int32, PhaseIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnZoneShrinkStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnZoneShrinkCompleted);

/**
 * Manages the shrinking safe zone / blue zone for battle royale.
 * Configurable phase table, random center shifting, damage application.
 * Replicated to all clients for HUD/minimap visualization.
 */
UCLASS()
class STORMBREAKER_API ASBZoneManager : public AActor
{
    GENERATED_BODY()

public:
    ASBZoneManager();

    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Control ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Zone")
    void InitZone(const FVector& MapCenter, float InitialRadius);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Zone")
    void StartNextPhase();

    // --- Queries ---

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    FVector GetCurrentCenter() const { return CurrentCenter; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    float GetCurrentRadius() const { return CurrentRadius; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    FVector GetTargetCenter() const { return TargetCenter; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    float GetTargetRadius() const { return TargetRadius; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    int32 GetCurrentPhase() const { return CurrentPhaseIndex; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    bool IsShrinking() const { return bIsShrinking; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    float GetTimeUntilShrink() const;

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    float GetShrinkProgress() const;

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    bool IsLocationInSafeZone(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    float GetCurrentDPS() const;

    // --- Phase Table ---

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Zone")
    TArray<FSBZonePhase> Phases;

    // --- Delegates ---

    UPROPERTY(BlueprintAssignable)
    FOnZonePhaseChanged OnZonePhaseChanged;

    UPROPERTY(BlueprintAssignable)
    FOnZoneShrinkStarted OnZoneShrinkStarted;

    UPROPERTY(BlueprintAssignable)
    FOnZoneShrinkCompleted OnZoneShrinkCompleted;

protected:
    void ApplyZoneDamage(float DeltaTime);
    void TickWaiting(float DeltaTime);
    void TickShrinking(float DeltaTime);
    void CompleteShrink();
    void GenerateNextTarget();

    UPROPERTY(Replicated)
    FVector CurrentCenter;

    UPROPERTY(Replicated)
    float CurrentRadius;

    UPROPERTY(Replicated)
    FVector TargetCenter;

    UPROPERTY(Replicated)
    float TargetRadius;

    UPROPERTY(Replicated)
    int32 CurrentPhaseIndex;

    UPROPERTY(Replicated)
    uint8 bIsShrinking : 1;

    UPROPERTY(Replicated)
    uint8 bIsWaiting : 1;

private:
    float WaitElapsed;
    float ShrinkElapsed;
    FVector ShrinkStartCenter;
    float ShrinkStartRadius;
    float InitialMapRadius;
};
