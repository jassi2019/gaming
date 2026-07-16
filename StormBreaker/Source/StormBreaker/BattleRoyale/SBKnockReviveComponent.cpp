// Copyright StormBreaker Games. All Rights Reserved.

#include "BattleRoyale/SBKnockReviveComponent.h"
#include "Inventory/SBDeathCrate.h"
#include "Inventory/SBInventoryComponent.h"
#include "Character/SBCharacterBase.h"
#include "Character/SBCharacterMovementComponent.h"
#include "Weapon/SBWeaponComponent.h"
#include "Core/SBPlayerState.h"
#include "Core/SBPlayerController.h"
#include "StormBreaker.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"

USBKnockReviveComponent::USBKnockReviveComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;

    LifeState = ESBPlayerLifeState::Alive;
    BleedOutElapsed = 0.0f;
    ReviveElapsed = 0.0f;
}

void USBKnockReviveComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USBKnockReviveComponent, LifeState);
    DOREPLIFETIME(USBKnockReviveComponent, BleedOutElapsed);
    DOREPLIFETIME(USBKnockReviveComponent, ReviveElapsed);
}

void USBKnockReviveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner()->HasAuthority()) return;

    if (LifeState == ESBPlayerLifeState::Knocked)
    {
        TickBleedOut(DeltaTime);
        TickRevive(DeltaTime);
    }
}

// ============================================================================
// State
// ============================================================================

float USBKnockReviveComponent::GetBleedOutTimeRemaining() const
{
    if (LifeState != ESBPlayerLifeState::Knocked) return 0.0f;
    return FMath::Max(0.0f, BleedOutDuration - BleedOutElapsed);
}

float USBKnockReviveComponent::GetReviveProgress() const
{
    if (!ActiveReviver || ReviveDuration <= 0.0f) return 0.0f;
    return FMath::Clamp(ReviveElapsed / ReviveDuration, 0.0f, 1.0f);
}

void USBKnockReviveComponent::SetLifeState(ESBPlayerLifeState NewState)
{
    if (LifeState == NewState) return;
    LifeState = NewState;
    OnLifeStateChanged.Broadcast(NewState);

    ASBPlayerState* PS = Cast<ASBPlayerState>(Cast<APawn>(GetOwner())->GetPlayerState());
    if (PS)
    {
        ESBPlayerStatus Status;
        switch (NewState)
        {
        case ESBPlayerLifeState::Alive: Status = ESBPlayerStatus::Alive; break;
        case ESBPlayerLifeState::Knocked: Status = ESBPlayerStatus::Downed; break;
        case ESBPlayerLifeState::Dead: Status = ESBPlayerStatus::Dead; break;
        case ESBPlayerLifeState::Spectating: Status = ESBPlayerStatus::Spectating; break;
        default: Status = ESBPlayerStatus::Alive; break;
        }
        PS->SetPlayerStatus(Status);
    }
}

void USBKnockReviveComponent::OnRep_LifeState()
{
    OnLifeStateChanged.Broadcast(LifeState);
}

// ============================================================================
// Damage → Knock/Death
// ============================================================================

void USBKnockReviveComponent::HandleHealthReachedZero(AController* Killer)
{
    if (!GetOwner()->HasAuthority()) return;
    LastKiller = Killer;

    if (bEnableKnockSystem && LifeState == ESBPlayerLifeState::Alive)
    {
        EnterKnockedState();
    }
    else
    {
        Die(Killer);
    }
}

void USBKnockReviveComponent::EnterKnockedState()
{
    SetLifeState(ESBPlayerLifeState::Knocked);
    BleedOutElapsed = 0.0f;
    ReviveElapsed = 0.0f;

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (Character)
    {
        // Disable most abilities, reduce speed
        USBCharacterMovementComponent* CMC = Character->GetSBMovement();
        if (CMC)
        {
            CMC->StopSprinting();
            CMC->StartProne();
        }

        // Drop current weapon
        USBWeaponComponent* WeaponComp = Character->FindComponentByClass<USBWeaponComponent>();
        if (WeaponComp)
        {
            WeaponComp->StopFire();
        }
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("%s knocked."), *GetNameSafe(GetOwner()));
}

void USBKnockReviveComponent::ForceKill(AController* Killer)
{
    if (!GetOwner()->HasAuthority()) return;
    Die(Killer);
}

void USBKnockReviveComponent::Die(AController* Killer)
{
    SetLifeState(ESBPlayerLifeState::Dead);

    SpawnDeathCrate();

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (Character)
    {
        Character->SetActorEnableCollision(false);
        Character->SetActorHiddenInGame(true);
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("%s died. Killer: %s"),
        *GetNameSafe(GetOwner()), *GetNameSafe(Killer));

    // Delay then enter spectator
    FTimerHandle SpectateTimer;
    GetWorld()->GetTimerManager().SetTimer(SpectateTimer, [this]()
    {
        EnterSpectatorMode();
    }, 3.0f, false);
}

void USBKnockReviveComponent::EnterSpectatorMode()
{
    SetLifeState(ESBPlayerLifeState::Spectating);

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (Character)
    {
        ASBPlayerController* PC = Cast<ASBPlayerController>(Character->GetController());
        if (PC)
        {
            PC->StartSpectating();
        }
    }
}

// ============================================================================
// Revive
// ============================================================================

void USBKnockReviveComponent::StartRevive(ASBCharacterBase* Reviver)
{
    if (LifeState != ESBPlayerLifeState::Knocked || !Reviver) return;

    if (!GetOwner()->HasAuthority())
    {
        Server_StartRevive(Reviver);
        return;
    }

    ActiveReviver = Reviver;
    ReviveElapsed = 0.0f;

    UE_LOG(LogSBBattleRoyale, Log, TEXT("%s started reviving %s"),
        *GetNameSafe(Reviver), *GetNameSafe(GetOwner()));
}

void USBKnockReviveComponent::CancelRevive()
{
    if (!ActiveReviver) return;

    if (!GetOwner()->HasAuthority())
    {
        Server_CancelRevive();
        return;
    }

    ActiveReviver = nullptr;
    ReviveElapsed = 0.0f;

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Revive cancelled for %s"), *GetNameSafe(GetOwner()));
}

void USBKnockReviveComponent::FinishRevive()
{
    SetLifeState(ESBPlayerLifeState::Alive);
    ActiveReviver = nullptr;
    ReviveElapsed = 0.0f;
    BleedOutElapsed = 0.0f;

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (Character)
    {
        USBCharacterMovementComponent* CMC = Character->GetSBMovement();
        if (CMC)
        {
            CMC->StopProne();
        }

        // Restore health to revive amount via GAS
        UE_LOG(LogSBBattleRoyale, Log, TEXT("%s revived with %.0f HP"),
            *GetNameSafe(GetOwner()), HealthAfterRevive);
    }
}

// ============================================================================
// Tick
// ============================================================================

void USBKnockReviveComponent::TickBleedOut(float DeltaTime)
{
    BleedOutElapsed += DeltaTime;

    if (BleedOutElapsed >= BleedOutDuration)
    {
        Die(LastKiller);
    }
}

void USBKnockReviveComponent::TickRevive(float DeltaTime)
{
    if (!ActiveReviver) return;

    // Check distance — cancel if reviver moved away
    float Distance = FVector::Dist(
        GetOwner()->GetActorLocation(),
        ActiveReviver->GetActorLocation());

    if (Distance > 300.0f)
    {
        CancelRevive();
        return;
    }

    ReviveElapsed += DeltaTime;
    OnReviveProgress.Broadcast(GetReviveProgress(), ActiveReviver);

    if (ReviveElapsed >= ReviveDuration)
    {
        FinishRevive();
    }
}

// ============================================================================
// Death Crate
// ============================================================================

void USBKnockReviveComponent::SpawnDeathCrate()
{
    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (!Character) return;

    USBInventoryComponent* Inv = Character->FindComponentByClass<USBInventoryComponent>();
    if (!Inv) return;

    TArray<FSBInventoryItem> AllItems = Inv->CollectAllItemsForDeathCrate();
    if (AllItems.Num() == 0) return;

    FVector SpawnLoc = Character->GetActorLocation();
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    TSubclassOf<ASBDeathCrate> CrateClass = DeathCrateClass.Get() ? DeathCrateClass : TSubclassOf<ASBDeathCrate>(ASBDeathCrate::StaticClass());

    ASBDeathCrate* Crate = GetWorld()->SpawnActor<ASBDeathCrate>(
        CrateClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);

    if (Crate)
    {
        FString PlayerName = TEXT("Unknown");
        if (APlayerState* PS = Character->GetPlayerState())
        {
            PlayerName = PS->GetPlayerName();
        }
        Crate->InitFromPlayerItems(AllItems, PlayerName);
        OnDeathCrateSpawned.Broadcast(Crate);
    }
}

// ============================================================================
// Server RPCs
// ============================================================================

void USBKnockReviveComponent::Server_StartRevive_Implementation(ASBCharacterBase* Reviver)
{
    StartRevive(Reviver);
}

void USBKnockReviveComponent::Server_CancelRevive_Implementation()
{
    CancelRevive();
}
