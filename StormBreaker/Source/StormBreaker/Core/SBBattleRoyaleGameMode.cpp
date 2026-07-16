// Copyright StormBreaker Games. All Rights Reserved.

#include "Core/SBBattleRoyaleGameMode.h"
#include "Core/SBBattleRoyaleGameState.h"
#include "Core/SBPlayerState.h"
#include "Core/SBPlayerController.h"
#include "Character/SBCharacterBase.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponDataAsset.h"
#include "UI/SBDebugHUD.h"
#include "StormBreaker.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ASBBattleRoyaleGameMode::ASBBattleRoyaleGameMode()
    : CurrentPhase(ESBMatchPhase::WaitingForPlayers)
    , AlivePlayerCount(0)
    , AliveTeamCount(0)
{
    PrimaryActorTick.bCanEverTick = true;

    // Core classes — no Blueprint required
    DefaultPawnClass = ASBCharacterBase::StaticClass();
    GameStateClass = ASBBattleRoyaleGameState::StaticClass();
    PlayerStateClass = ASBPlayerState::StaticClass();
    PlayerControllerClass = ASBPlayerController::StaticClass();
    HUDClass = ASBHUD::StaticClass();

    // PIE-friendly: start match with 1 player
    MinPlayersToStart = 1;
}

void ASBBattleRoyaleGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // Create default weapon data asset for testing
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

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Battle Royale InitGame — Map: %s"), *MapName);
}

void ASBBattleRoyaleGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    SetMatchPhase(ESBMatchPhase::InProgress);

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Match started — InProgress."));
}

void ASBBattleRoyaleGameMode::HandleMatchHasEnded()
{
    Super::HandleMatchHasEnded();
    SetMatchPhase(ESBMatchPhase::EndGame);
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Match ended."));
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

    // Update GameState alive count
    ASBBattleRoyaleGameState* GS = GetGameState<ASBBattleRoyaleGameState>();
    if (GS)
    {
        GS->SetAlivePlayerCount(AlivePlayerCount);
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Player joined. Total alive: %d"), AlivePlayerCount);

    // Auto-start match for PIE testing
    if (CurrentPhase == ESBMatchPhase::WaitingForPlayers && AlivePlayerCount >= MinPlayersToStart)
    {
        StartMatch();
    }

    // Give player a starter weapon after a short delay (pawn needs to be spawned first)
    FTimerHandle WeaponTimer;
    TWeakObjectPtr<APlayerController> WeakPC = NewPlayer;
    GetWorldTimerManager().SetTimer(WeaponTimer, [this, WeakPC]()
    {
        if (!WeakPC.IsValid()) return;
        GiveStarterWeapon(WeakPC.Get());
    }, 0.5f, false);
}

void ASBBattleRoyaleGameMode::GiveStarterWeapon(APlayerController* PC)
{
    if (!PC || !StarterWeaponData) return;

    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return;

    USBWeaponComponent* WeaponComp = Pawn->FindComponentByClass<USBWeaponComponent>();
    if (!WeaponComp) return;

    WeaponComp->AddWeapon(StarterWeaponData, StarterWeaponData->DefaultSlot);

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Gave starter weapon '%s' to %s"),
        *StarterWeaponData->DisplayName.ToString(), *GetNameSafe(PC));
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
}

void ASBBattleRoyaleGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ASBBattleRoyaleGameMode::SpawnBots()
{
    if (NumberOfBots <= 0) return;
    UE_LOG(LogSBAI, Log, TEXT("Spawning %d bots."), NumberOfBots);
}

void ASBBattleRoyaleGameMode::CheckWinCondition()
{
    if (AlivePlayerCount <= 1 && CurrentPhase == ESBMatchPhase::InProgress)
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
