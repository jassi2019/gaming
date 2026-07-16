// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Core/SBBattleRoyaleGameMode.h"
#include "SBBattleRoyaleGameState.generated.h"

USTRUCT(BlueprintType)
struct FSBZoneData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector Center = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float CurrentRadius = 10000.0f;

    UPROPERTY(BlueprintReadOnly)
    float TargetRadius = 5000.0f;

    UPROPERTY(BlueprintReadOnly)
    float ShrinkDuration = 120.0f;

    UPROPERTY(BlueprintReadOnly)
    float DamagePerSecond = 1.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentPhaseIndex = 0;
};

/**
 * Replicated game state — visible to all clients.
 * Holds zone data, alive count, match phase, and kill feed.
 */
UCLASS()
class STORMBREAKER_API ASBBattleRoyaleGameState : public AGameState
{
    GENERATED_BODY()

public:
    ASBBattleRoyaleGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ----- Zone -----

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Zone")
    FSBZoneData GetZoneData() const { return ZoneData; }

    void BeginZoneShrink();
    void AdvanceZonePhase();

    // ----- Match State -----

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Match")
    ESBMatchPhase GetMatchPhase() const { return MatchPhase; }

    void SetMatchPhase(ESBMatchPhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Match")
    int32 GetAlivePlayerCount() const { return AlivePlayerCount; }

    void SetAlivePlayerCount(int32 Count);

    // ----- Match Timer -----

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Match")
    float GetMatchElapsedTime() const;

protected:
    virtual void Tick(float DeltaSeconds) override;

private:
    void UpdateZoneShrink(float DeltaSeconds);

    UPROPERTY(Replicated)
    FSBZoneData ZoneData;

    UPROPERTY(Replicated)
    ESBMatchPhase MatchPhase;

    UPROPERTY(Replicated)
    int32 AlivePlayerCount;

    UPROPERTY(Replicated)
    float MatchStartServerTime;

    bool bIsZoneShrinking;
    float ZoneShrinkElapsed;
};
