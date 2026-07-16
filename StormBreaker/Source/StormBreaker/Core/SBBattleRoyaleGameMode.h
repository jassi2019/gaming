// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SBBattleRoyaleGameMode.generated.h"

class ASBBattleRoyaleGameState;

UENUM(BlueprintType)
enum class ESBMatchPhase : uint8
{
    WaitingForPlayers,
    WarmUp,
    PlanePhase,
    InProgress,
    EndGame
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchPhaseChanged, ESBMatchPhase, NewPhase);

/**
 * Authoritative match controller — runs only on the server.
 * Handles player spawning, zone shrink, loot distribution, and win conditions.
 */
UCLASS()
class STORMBREAKER_API ASBBattleRoyaleGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ASBBattleRoyaleGameMode();

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void HandleMatchHasStarted() override;
    virtual void HandleMatchHasEnded() override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    // ----- Match Flow -----

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Match")
    void SetMatchPhase(ESBMatchPhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Match")
    ESBMatchPhase GetMatchPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Match")
    void StartPlanePhase();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Match")
    void TriggerZoneShrink();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Match")
    void SpawnAirDrop(FVector Location);

    UPROPERTY(BlueprintAssignable)
    FOnMatchPhaseChanged OnMatchPhaseChanged;

    // ----- Config -----

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Match")
    int32 MaxPlayersPerMatch = 100;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Match")
    int32 MinPlayersToStart = 2;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Match")
    float WarmUpDuration = 60.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Match")
    int32 TeamSize = 4;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Match")
    int32 NumberOfBots = 0;

protected:
    virtual void Tick(float DeltaSeconds) override;

private:
    void SpawnBots();
    void CheckWinCondition();
    void HandlePlayerEliminated(AController* EliminatedPlayer, AController* Killer);

    ESBMatchPhase CurrentPhase;
    int32 AlivePlayerCount;
    int32 AliveTeamCount;
    FTimerHandle ZoneShrinkTimerHandle;
    FTimerHandle WarmUpTimerHandle;
};
