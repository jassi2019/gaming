// Copyright Island Of Death Games. All Rights Reserved.

#include "Core/SBPlayerController.h"
#include "StormBreaker.h"
#include "UI/SBMobileTouchWidget.h"
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
    CreateMobileTouchWidget();
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

void ASBPlayerController::CreateMobileTouchWidget()
{
    if (!IsLocalController()) return;

    // Only create on mobile platforms
    bool bIsMobile = false;
#if PLATFORM_ANDROID || PLATFORM_IOS
    bIsMobile = true;
#endif

    if (!bIsMobile || !MobileTouchWidgetClass)
    {
        return;
    }

    MobileTouchWidget = CreateWidget<USBMobileTouchWidget>(this, MobileTouchWidgetClass);
    if (MobileTouchWidget)
    {
        MobileTouchWidget->AddToViewport(100);
        UE_LOG(LogStormBreaker, Log, TEXT("Mobile touch widget created."));
    }
}

// ----- HUD -----

void ASBPlayerController::ShowMatchHUD()
{
    // Widget creation handled by Blueprint subclass
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
    UE_LOG(LogSBCharacter, Log, TEXT("Respawn requested by: %s"), *GetNameSafe(this));
}

// ----- Client RPCs -----

void ASBPlayerController::Client_OnMatchEnd_Implementation(bool bIsWinner)
{
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Match ended. Winner: %s"), bIsWinner ? TEXT("YES") : TEXT("NO"));
}

void ASBPlayerController::Client_ShowKillFeed_Implementation(
    const FString& KillerName, const FString& VictimName, const FString& WeaponName)
{
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Kill Feed: %s eliminated %s with %s"),
        *KillerName, *VictimName, *WeaponName);
}
