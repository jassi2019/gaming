// Copyright Island Of Death Games. All Rights Reserved.

#include "UI/SBMobileControlsComponent.h"
#include "Character/SBCharacterBase.h"
#include "Inventory/SBInventoryComponent.h"
#include "StormBreaker.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

USBMobileControlsComponent::USBMobileControlsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    bGyroscopeEnabled = false;
    CurrentPeek = ESBPeekDirection::None;
    CurrentPeekOffset = 0.0f;
    bHealingWheelOpen = false;

#if PLATFORM_ANDROID || PLATFORM_IOS
    PrimaryComponentTick.bStartWithTickEnabled = true;
#endif
}

void USBMobileControlsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickGyroscope(DeltaTime);
    TickPeek(DeltaTime);
}

// ============================================================================
// Gyroscope
// ============================================================================

void USBMobileControlsComponent::SetGyroscopeEnabled(bool bEnabled)
{
    bGyroscopeEnabled = bEnabled;
}

void USBMobileControlsComponent::TickGyroscope(float DeltaTime)
{
    if (!bGyroscopeEnabled) return;

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (!Character) return;

    if (bGyroscopeOnlyWhileADS && !Character->IsAiming()) return;

    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    if (!PC) return;

    FVector Tilt, RotRate, GravityVec, AccelVec;
    PC->GetInputMotionState(Tilt, RotRate, GravityVec, AccelVec);

    float PitchInput = -RotRate.X * GyroscopeSensitivity * DeltaTime;
    float YawInput = RotRate.Z * GyroscopeSensitivity * DeltaTime;

    PC->AddPitchInput(PitchInput);
    PC->AddYawInput(YawInput);
}

// ============================================================================
// Peek
// ============================================================================

void USBMobileControlsComponent::StartPeek(ESBPeekDirection Direction)
{
    CurrentPeek = Direction;
}

void USBMobileControlsComponent::StopPeek()
{
    CurrentPeek = ESBPeekDirection::None;
}

void USBMobileControlsComponent::TickPeek(float DeltaTime)
{
    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (!Character || !Character->CameraBoom) return;

    float TargetOffset = 0.0f;
    switch (CurrentPeek)
    {
    case ESBPeekDirection::Left:  TargetOffset = -PeekDistance; break;
    case ESBPeekDirection::Right: TargetOffset = PeekDistance;  break;
    default: break;
    }

    CurrentPeekOffset = FMath::FInterpTo(CurrentPeekOffset, TargetOffset, DeltaTime, PeekSpeed);

    FVector CurrentSocket = Character->CameraBoom->SocketOffset;
    CurrentSocket.Y = Character->CameraSocketOffset.Y + CurrentPeekOffset;
    Character->CameraBoom->SocketOffset = CurrentSocket;
}

// ============================================================================
// Healing Wheel
// ============================================================================

void USBMobileControlsComponent::OpenHealingWheel()
{
    bHealingWheelOpen = true;
}

void USBMobileControlsComponent::CloseHealingWheel()
{
    bHealingWheelOpen = false;
}

void USBMobileControlsComponent::SelectHealingItem(int32 ItemIndex)
{
    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (!Character) return;

    USBInventoryComponent* Inv = Character->InventoryComponent;
    if (!Inv) return;

    static const TArray<FName> HealItems = {
        FName("Bandage"), FName("FirstAidKit"), FName("MedKit"),
        FName("EnergyDrink"), FName("Painkiller"), FName("Adrenaline")
    };

    if (HealItems.IsValidIndex(ItemIndex))
    {
        Inv->UseConsumable(HealItems[ItemIndex]);
    }

    bHealingWheelOpen = false;
}
