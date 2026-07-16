// Copyright StormBreaker Games. All Rights Reserved.

#include "Core/SBPlayerController.h"
#include "StormBreaker.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

ASBPlayerController::ASBPlayerController()
{
}

void ASBPlayerController::BeginPlay()
{
    Super::BeginPlay();
    BindInputMappingContext();
}

void ASBPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    UE_LOG(LogSBCharacter, Log, TEXT("PlayerController possessed pawn: %s"), *GetNameSafe(InPawn));
}

void ASBPlayerController::OnUnPossess()
{
    UE_LOG(LogSBCharacter, Log, TEXT("PlayerController unpossessed pawn."));
    Super::OnUnPossess();
}

void ASBPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    // Input actions will be bound in Phase 2 (Character module)
}

void ASBPlayerController::BindInputMappingContext()
{
    if (!IsLocalController()) return;

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

    if (InputSubsystem && DefaultMappingContext)
    {
        InputSubsystem->AddMappingContext(DefaultMappingContext, DefaultMappingPriority);
        UE_LOG(LogSBCharacter, Log, TEXT("Input mapping context bound."));
    }
}

// ----- HUD -----

void ASBPlayerController::ShowMatchHUD()
{
    // Widget creation handled by Blueprint subclass (BP_SBPlayerController)
}

void ASBPlayerController::ShowInventoryUI()
{
    SetInputMode(FInputModeGameAndUI());
    SetShowMouseCursor(true);
}

void ASBPlayerController::HideInventoryUI()
{
    SetInputMode(FInputModeGameOnly());
    SetShowMouseCursor(false);
}

void ASBPlayerController::ShowMapUI()
{
    // Map widget toggle handled by Blueprint
}

// ----- Spectating -----

void ASBPlayerController::StartSpectating()
{
    ChangeState(NAME_Spectating);
    UE_LOG(LogSBCharacter, Log, TEXT("Player started spectating."));
}

void ASBPlayerController::Server_RequestRespawn_Implementation()
{
    // Only valid during warm-up or team revive
    UE_LOG(LogSBCharacter, Log, TEXT("Respawn requested by: %s"), *GetNameSafe(this));
}

// ----- Client RPCs -----

void ASBPlayerController::Client_OnMatchEnd_Implementation(bool bIsWinner)
{
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Match ended. Winner: %s"), bIsWinner ? TEXT("YES") : TEXT("NO"));
    // Trigger end-game UI via Blueprint
}

void ASBPlayerController::Client_ShowKillFeed_Implementation(
    const FString& KillerName, const FString& VictimName, const FString& WeaponName)
{
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Kill Feed: %s eliminated %s with %s"),
        *KillerName, *VictimName, *WeaponName);
    // Push to HUD widget via Blueprint
}
