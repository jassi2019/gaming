// Copyright Island Of Death Games. All Rights Reserved.

#include "Core/SBBattleRoyaleGameMode.h"
#include "Core/SBBattleRoyaleGameState.h"
#include "Core/SBPlayerState.h"
#include "Core/SBPlayerController.h"
#include "Character/SBCharacterBase.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponDataAsset.h"
#include "AI/SBBotSpawner.h"
#include "BattleRoyale/SBMapConfigurator.h"
#include "BattleRoyale/SBWorldGenerator.h"
#include "BattleRoyale/SBEnvironmentManager.h"
#include "BattleRoyale/SBZoneManager.h"
#include "Backend/SBBackendSubsystem.h"
#include "Backend/SBBackendTypes.h"
#include "UI/SBGameFlowHUD.h"
#include "StormBreaker.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ASBBattleRoyaleGameMode::ASBBattleRoyaleGameMode()
    : CurrentPhase(ESBMatchPhase::WaitingForPlayers)
    , AlivePlayerCount(0)
    , AliveTeamCount(0)
    , MatchStartTime(0.0f)
{
    PrimaryActorTick.bCanEverTick = true;

    DefaultPawnClass = ASBCharacterBase::StaticClass();
    GameStateClass = ASBBattleRoyaleGameState::StaticClass();
    PlayerStateClass = ASBPlayerState::StaticClass();
    PlayerControllerClass = ASBPlayerController::StaticClass();
    HUDClass = ASBGameFlowHUD::StaticClass();

    // Match doesn't auto-start — lobby screen handles it
    MinPlayersToStart = 100;
    NumberOfBots = 2;
}

void ASBBattleRoyaleGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // Create starter weapon data
    if (!StarterWeaponData)
    {
        StarterWeaponData = NewObject<USBWeaponDataAsset>(this, TEXT("DA_StarterAK47"));
        StarterWeaponData->WeaponID = FName("AK47");
        StarterWeaponData->DisplayName = FText::FromString(TEXT("AK-47"));
        StarterWeaponData->WeaponType = ESBWeaponType::AssaultRifle;
        StarterWeaponData->DefaultSlot = ESBWeaponSlot::Primary;
        StarterWeaponData->AmmoType = ESBAmmoType::Rifle_762;
        StarterWeaponData->DamageMethod = ESBDamageType::Hitscan;
        StarterWeaponData->AvailableFireModes = { ESBFireMode::Auto, ESBFireMode::Single };
        StarterWeaponData->DefaultFireMode = ESBFireMode::Auto;
        StarterWeaponData->FireRate = 600.0f;
        StarterWeaponData->PelletsPerShot = 1;
        StarterWeaponData->Damage.BaseDamage = 36.0f;
        StarterWeaponData->Damage.HeadshotMultiplier = 2.5f;
        StarterWeaponData->Damage.EffectiveRange = 4000.0f;
        StarterWeaponData->Damage.MaxRange = 15000.0f;
        StarterWeaponData->Damage.PenetrationCount = 1;
        StarterWeaponData->MagazineSize = 30;
        StarterWeaponData->MaxReserveAmmo = 120;
        StarterWeaponData->ReloadTime = 2.3f;
        StarterWeaponData->ReloadTimeEmpty = 2.8f;
        StarterWeaponData->EquipTime = 0.6f;
        StarterWeaponData->UnequipTime = 0.4f;
        StarterWeaponData->Recoil.VerticalMin = 0.3f;
        StarterWeaponData->Recoil.VerticalMax = 0.6f;
        StarterWeaponData->Recoil.HorizontalMin = -0.15f;
        StarterWeaponData->Recoil.HorizontalMax = 0.15f;
        StarterWeaponData->Recoil.ADSMultiplier = 0.5f;
        StarterWeaponData->Recoil.RecoverySpeed = 5.0f;
        StarterWeaponData->Spread.HipFireSpread = 4.0f;
        StarterWeaponData->Spread.ADSSpread = 0.5f;
        StarterWeaponData->Spread.SpreadIncreasePerShot = 0.3f;
        StarterWeaponData->Spread.MaxSpread = 8.0f;
        StarterWeaponData->Spread.SpreadRecoveryRate = 3.0f;
    }

    // Auto-spawn arena if no PlayerStarts exist
    if (bAutoSpawnArena)
    {
        SpawnArena();
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("InitGame — Map: %s, Bots: %d"), *MapName, NumberOfBots);
}

void ASBBattleRoyaleGameMode::SpawnArena()
{
    TArray<AActor*> ExistingStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ExistingStarts);

    if (ExistingStarts.Num() == 0)
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // Spawn full world generator (4x4km island with POIs, buildings, terrain, loot)
        GetWorld()->SpawnActor<ASBWorldGenerator>(ASBWorldGenerator::StaticClass(),
            FVector::ZeroVector, FRotator::ZeroRotator, Params);

        // Spawn environment manager (day/night + weather)
        GetWorld()->SpawnActor<ASBEnvironmentManager>(ASBEnvironmentManager::StaticClass(),
            FVector::ZeroVector, FRotator::ZeroRotator, Params);

        UE_LOG(LogSBBattleRoyale, Log, TEXT("Auto-spawned WorldGenerator + EnvironmentManager."));
    }
}

// ============================================================================
// Match Flow
// ============================================================================

void ASBBattleRoyaleGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    MatchStartTime = GetWorld()->GetTimeSeconds();
    SetMatchPhase(ESBMatchPhase::InProgress);

    // Spawn bots
    SpawnBots();

    // Start zone phases after delay
    if (bAutoStartZone)
    {
        GetWorldTimerManager().SetTimer(ZoneShrinkTimerHandle, this,
            &ASBBattleRoyaleGameMode::StartZonePhases, ZoneStartDelay, false);
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Match started. Alive: %d"), AlivePlayerCount);
}

void ASBBattleRoyaleGameMode::HandleMatchHasEnded()
{
    Super::HandleMatchHasEnded();
    SetMatchPhase(ESBMatchPhase::EndGame);
    ShowMatchResults();
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Match ended."));
}

void ASBBattleRoyaleGameMode::StartZonePhases()
{
    for (TActorIterator<ASBZoneManager> It(GetWorld()); It; ++It)
    {
        (*It)->StartNextPhase();
        UE_LOG(LogSBBattleRoyale, Log, TEXT("Zone phases started."));
        break;
    }
}

AActor* ASBBattleRoyaleGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
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

    ASBBattleRoyaleGameState* GS = GetGameState<ASBBattleRoyaleGameState>();
    if (GS) GS->SetAlivePlayerCount(AlivePlayerCount);

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Player joined. Alive: %d"), AlivePlayerCount);

    // Match start is controlled by lobby screen (SBGameFlowHUD)

    // Give starter weapon
    FTimerHandle WeaponTimer;
    TWeakObjectPtr<APlayerController> WeakPC = NewPlayer;
    GetWorldTimerManager().SetTimer(WeaponTimer, [this, WeakPC]()
    {
        if (WeakPC.IsValid()) GiveStarterWeapon(WeakPC.Get());
    }, 0.5f, false);
}

void ASBBattleRoyaleGameMode::GiveStarterWeapon(APlayerController* PC)
{
    if (!PC || !StarterWeaponData) return;
    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return;

    USBWeaponComponent* WeaponComp = Pawn->FindComponentByClass<USBWeaponComponent>();
    if (WeaponComp)
    {
        WeaponComp->AddWeapon(StarterWeaponData, StarterWeaponData->DefaultSlot);
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
    if (GS) GS->SetMatchPhase(NewPhase);

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Phase -> %d"), (int32)NewPhase);
}

void ASBBattleRoyaleGameMode::StartPlanePhase()
{
    SetMatchPhase(ESBMatchPhase::PlanePhase);
}

void ASBBattleRoyaleGameMode::TriggerZoneShrink()
{
    ASBBattleRoyaleGameState* GS = GetGameState<ASBBattleRoyaleGameState>();
    if (GS) GS->BeginZoneShrink();
}

void ASBBattleRoyaleGameMode::SpawnAirDrop(FVector Location)
{
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Air drop at: %s"), *Location.ToString());
}

void ASBBattleRoyaleGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

// ============================================================================
// Bots
// ============================================================================

void ASBBattleRoyaleGameMode::SpawnBots()
{
    if (NumberOfBots <= 0) return;

    USBBotSpawner* BotSpawner = GetWorld()->GetSubsystem<USBBotSpawner>();
    if (BotSpawner)
    {
        BotSpawner->SpawnBots(NumberOfBots, TSubclassOf<ASBCharacterBase>(DefaultPawnClass));
        AlivePlayerCount += BotSpawner->GetAliveBotCount();

        ASBBattleRoyaleGameState* GS = GetGameState<ASBBattleRoyaleGameState>();
        if (GS) GS->SetAlivePlayerCount(AlivePlayerCount);
    }

    UE_LOG(LogSBAI, Log, TEXT("Spawned %d bots. Total alive: %d"), NumberOfBots, AlivePlayerCount);
}

// ============================================================================
// Win Condition
// ============================================================================

void ASBBattleRoyaleGameMode::CheckWinCondition()
{
    // Count alive human players
    int32 AliveHumans = 0;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->GetPawn())
        {
            AliveHumans++;
        }
    }

    // Count alive bots
    int32 AliveBots = 0;
    USBBotSpawner* BotSpawner = GetWorld()->GetSubsystem<USBBotSpawner>();
    if (BotSpawner) AliveBots = BotSpawner->GetAliveBotCount();

    int32 TotalAlive = AliveHumans + AliveBots;
    AlivePlayerCount = TotalAlive;

    ASBBattleRoyaleGameState* GS = GetGameState<ASBBattleRoyaleGameState>();
    if (GS) GS->SetAlivePlayerCount(TotalAlive);

    if (TotalAlive <= 1 && CurrentPhase == ESBMatchPhase::InProgress)
    {
        UE_LOG(LogSBBattleRoyale, Log, TEXT("WINNER WINNER CHICKEN DINNER! Alive: %d"), TotalAlive);
        EndMatch();
    }
}

// ============================================================================
// Match Results
// ============================================================================

void ASBBattleRoyaleGameMode::ShowMatchResults()
{
    float MatchDuration = GetWorld()->GetTimeSeconds() - MatchStartTime;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        ASBPlayerState* PS = PC->GetPlayerState<ASBPlayerState>();
        if (!PS) continue;

        // Build match result
        FSBMatchResult Result;
        Result.MatchID = FGuid::NewGuid().ToString();
        Result.Placement = (AlivePlayerCount <= 1) ? 1 : AlivePlayerCount;
        Result.Kills = PS->GetKills();
        Result.Assists = PS->GetAssists();
        Result.DamageDealt = PS->GetDamageDealt();
        Result.SurvivalSeconds = MatchDuration;

        // Calculate rewards
        Result.XPEarned = Result.Kills * 20 + FMath::FloorToInt(MatchDuration / 60.0f) * 10;
        if (Result.Placement == 1) Result.XPEarned += 100;
        if (Result.Placement <= 10) Result.XPEarned += 50;

        Result.RankPointsDelta = CalculateRankPointsDelta(Result.Placement, Result.Kills);
        Result.CoinsEarned = Result.Kills * 10 + (Result.Placement == 1 ? 50 : 0);
        Result.Timestamp = FDateTime::UtcNow();

        // Submit to backend
        UGameInstance* GI = GetGameInstance();
        if (GI)
        {
            USBBackendSubsystem* Backend = GI->GetSubsystem<USBBackendSubsystem>();
            if (Backend)
            {
                Backend->SubmitMatchResult(Result);
            }
        }

        // Notify client
        ASBPlayerController* SBPC = Cast<ASBPlayerController>(PC);
        if (SBPC)
        {
            SBPC->Client_OnMatchEnd(Result.Placement == 1);
        }

        UE_LOG(LogSBBattleRoyale, Log, TEXT("Player %s: #%d, %d kills, +%d XP, +%d RP"),
            *PS->GetPlayerName(), Result.Placement, Result.Kills, Result.XPEarned, Result.RankPointsDelta);
    }
}

void ASBBattleRoyaleGameMode::HandlePlayerEliminated(AController* EliminatedPlayer, AController* Killer)
{
    AlivePlayerCount = FMath::Max(0, AlivePlayerCount - 1);

    ASBPlayerState* KillerPS = Killer ? Killer->GetPlayerState<ASBPlayerState>() : nullptr;
    if (KillerPS) KillerPS->AddKill();

    // Kill feed
    FString KillerName = Killer ? GetNameSafe(Killer) : TEXT("Zone");
    FString VictimName = GetNameSafe(EliminatedPlayer);

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ASBPlayerController* PC = Cast<ASBPlayerController>(It->Get());
        if (PC)
        {
            PC->Client_ShowKillFeed(KillerName, VictimName, TEXT("AK-47"));
        }
    }

    CheckWinCondition();
}

int32 ASBBattleRoyaleGameMode::CalculateRankPointsDelta(int32 Placement, int32 Kills) const
{
    // Placement rewards
    int32 Points = 0;
    if (Placement == 1)       Points += 25;
    else if (Placement <= 5)  Points += 15;
    else if (Placement <= 10) Points += 10;
    else if (Placement <= 25) Points += 5;
    else if (Placement <= 50) Points += 0;
    else                      Points -= 10;

    // Kill rewards
    Points += Kills * 3;

    return Points;
}
