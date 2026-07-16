// Copyright StormBreaker Games. All Rights Reserved.

#include "Core/SBBattleRoyaleGameMode.h"
#include "Core/SBBattleRoyaleGameState.h"
#include "Core/SBPlayerState.h"
#include "Core/SBPlayerController.h"
#include "StormBreaker.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ASBBattleRoyaleGameMode::ASBBattleRoyaleGameMode()
    : CurrentPhase(ESBMatchPhase::WaitingForPlayers)
    , AlivePlayerCount(0)
    , AliveTeamCount(0)
{
    bUseSeamlessTravel = true;
    PrimaryActorTick.bCanEverTick = true;

    GameStateClass = ASBBattleRoyaleGameState::StaticClass();
    PlayerStateClass = ASBPlayerState::StaticClass();
    PlayerControllerClass = ASBPlayerController::StaticClass();

    // Default pawn will be set via Blueprint to BP_SBCharacter
}

void ASBBattleRoyaleGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Battle Royale InitGame — Map: %s"), *MapName);
}

void ASBBattleRoyaleGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    SpawnBots();
    SetMatchPhase(ESBMatchPhase::WarmUp);

    GetWorldTimerManager().SetTimer(WarmUpTimerHandle, [this]()
    {
        StartPlanePhase();
    }, WarmUpDuration, false);

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Match started. Warm-up: %.0fs"), WarmUpDuration);
}

void ASBBattleRoyaleGameMode::HandleMatchHasEnded()
{
    Super::HandleMatchHasEnded();
    SetMatchPhase(ESBMatchPhase::EndGame);
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Match ended."));
}

AActor* ASBBattleRoyaleGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    // During warm-up: use lobby spawn points
    // During match: players spawn from plane (handled separately)
    TArray<AActor*> Starts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Starts);

    if (Starts.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, Starts.Num() - 1);
        return Starts[Index];
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}

void ASBBattleRoyaleGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    AlivePlayerCount++;

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Player joined. Total alive: %d"), AlivePlayerCount);

    if (CurrentPhase == ESBMatchPhase::WaitingForPlayers && AlivePlayerCount >= MinPlayersToStart)
    {
        StartMatch();
    }
}

void ASBBattleRoyaleGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    if (CurrentPhase == ESBMatchPhase::InProgress)
    {
        AlivePlayerCount = FMath::Max(0, AlivePlayerCount - 1);
        CheckWinCondition();
    }
}

void ASBBattleRoyaleGameMode::SetMatchPhase(ESBMatchPhase NewPhase)
{
    if (CurrentPhase == NewPhase) return;

    CurrentPhase = NewPhase;
    OnMatchPhaseChanged.Broadcast(NewPhase);

    ASBBattleRoyaleGameState* GS = GetGameState<ASBBattleRoyaleGameState>();
    if (GS)
    {
        GS->SetMatchPhase(NewPhase);
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Match phase -> %d"), (int32)NewPhase);
}

void ASBBattleRoyaleGameMode::StartPlanePhase()
{
    SetMatchPhase(ESBMatchPhase::PlanePhase);
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Plane phase started."));

    // After plane phase, transition to InProgress (duration driven by Blueprint)
}

void ASBBattleRoyaleGameMode::TriggerZoneShrink()
{
    ASBBattleRoyaleGameState* GS = GetGameState<ASBBattleRoyaleGameState>();
    if (GS)
    {
        GS->BeginZoneShrink();
    }
}

void ASBBattleRoyaleGameMode::SpawnAirDrop(FVector Location)
{
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Air drop spawned at: %s"), *Location.ToString());
    // Actual spawning handled by Blueprint subclass
}

void ASBBattleRoyaleGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (CurrentPhase == ESBMatchPhase::InProgress)
    {
        CheckWinCondition();
    }
}

void ASBBattleRoyaleGameMode::SpawnBots()
{
    if (NumberOfBots <= 0) return;

    UE_LOG(LogSBAI, Log, TEXT("Spawning %d bots."), NumberOfBots);
    // AI bot spawning handled in Phase 7 — AI module
}

void ASBBattleRoyaleGameMode::CheckWinCondition()
{
    if (AlivePlayerCount <= 1)
    {
        UE_LOG(LogSBBattleRoyale, Log, TEXT("Win condition met. Ending match."));
        EndMatch();
    }
}

void ASBBattleRoyaleGameMode::HandlePlayerEliminated(AController* EliminatedPlayer, AController* Killer)
{
    AlivePlayerCount = FMath::Max(0, AlivePlayerCount - 1);

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Player eliminated. Alive: %d"), AlivePlayerCount);

    ASBPlayerState* KillerPS = Killer ? Killer->GetPlayerState<ASBPlayerState>() : nullptr;
    if (KillerPS)
    {
        KillerPS->AddKill();
    }

    CheckWinCondition();
}
