// Copyright StormBreaker Games. All Rights Reserved.

#include "UI/SBMobileTouchWidget.h"
#include "Character/SBCharacterBase.h"
#include "Character/SBCharacterMovementComponent.h"
#include "StormBreaker.h"
#include "GameFramework/PlayerController.h"

void USBMobileTouchWidget::NativeConstruct()
{
    Super::NativeConstruct();

    MoveInput = FVector2D::ZeroVector;
    LookInput = FVector2D::ZeroVector;
    PreviousLookInput = FVector2D::ZeroVector;

    UE_LOG(LogStormBreaker, Log, TEXT("Mobile touch widget constructed."));
}

void USBMobileTouchWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    ASBCharacterBase* Character = GetOwningCharacter();
    if (!Character) return;

    // Apply movement input
    if (!MoveInput.IsNearlyZero(JoystickDeadzone))
    {
        const FRotator YawRotation(0.0f, Character->GetControlRotation().Yaw, 0.0f);
        const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        Character->AddMovementInput(ForwardDir, MoveInput.Y);
        Character->AddMovementInput(RightDir, MoveInput.X);
    }

    // Apply look input
    if (!LookInput.IsNearlyZero())
    {
        FVector2D LookDelta = LookInput - PreviousLookInput;
        Character->AddControllerYawInput(LookDelta.X * LookSensitivity);
        Character->AddControllerPitchInput(LookDelta.Y * LookSensitivity);
    }

    PreviousLookInput = LookInput;

    // Reset look input each frame (it's a delta)
    LookInput = FVector2D::ZeroVector;
}

ASBCharacterBase* USBMobileTouchWidget::GetOwningCharacter() const
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return nullptr;
    return Cast<ASBCharacterBase>(PC->GetPawn());
}

// --- Input Setters ---

void USBMobileTouchWidget::SetMoveInput(FVector2D Input)
{
    MoveInput = Input;
}

void USBMobileTouchWidget::SetLookInput(FVector2D Input)
{
    LookInput = Input;
}

// --- Action Handlers ---

void USBMobileTouchWidget::OnJumpPressed()
{
    ASBCharacterBase* Character = GetOwningCharacter();
    if (!Character) return;

    USBCharacterMovementComponent* CMC = Character->GetSBMovement();
    if (CMC && CMC->TryMantleOrVault())
    {
        return;
    }

    Character->Jump();
}

void USBMobileTouchWidget::OnCrouchPressed()
{
    ASBCharacterBase* Character = GetOwningCharacter();
    if (!Character) return;

    USBCharacterMovementComponent* CMC = Character->GetSBMovement();
    if (!CMC) return;

    if (CMC->IsProning())
    {
        CMC->StopProne();
        return;
    }

    if (Character->bIsCrouched)
    {
        Character->UnCrouch();
    }
    else
    {
        Character->Crouch();
    }
}

void USBMobileTouchWidget::OnPronePressed()
{
    ASBCharacterBase* Character = GetOwningCharacter();
    if (!Character) return;

    USBCharacterMovementComponent* CMC = Character->GetSBMovement();
    if (CMC)
    {
        if (CMC->IsProning())
        {
            CMC->StopProne();
        }
        else
        {
            CMC->StartProne();
        }
    }
}

void USBMobileTouchWidget::OnSprintPressed()
{
    ASBCharacterBase* Character = GetOwningCharacter();
    if (!Character) return;

    USBCharacterMovementComponent* CMC = Character->GetSBMovement();
    if (CMC)
    {
        CMC->StartSprinting();
    }
}

void USBMobileTouchWidget::OnSprintReleased()
{
    ASBCharacterBase* Character = GetOwningCharacter();
    if (!Character) return;

    USBCharacterMovementComponent* CMC = Character->GetSBMovement();
    if (CMC)
    {
        CMC->StopSprinting();
    }
}

void USBMobileTouchWidget::OnFireButtonPressed()
{
    OnFirePressed.Broadcast();
}

void USBMobileTouchWidget::OnFireButtonReleased()
{
    OnFireReleased.Broadcast();
}

void USBMobileTouchWidget::OnADSButtonPressed()
{
    OnADSPressed.Broadcast();

    ASBCharacterBase* Character = GetOwningCharacter();
    if (Character)
    {
        // Directly set ADS via the character's input handler equivalent
        USBCharacterMovementComponent* CMC = Character->GetSBMovement();
        if (CMC)
        {
            CMC->SetAiming(true);
        }
    }
}

void USBMobileTouchWidget::OnADSButtonReleased()
{
    OnADSReleased.Broadcast();

    ASBCharacterBase* Character = GetOwningCharacter();
    if (Character)
    {
        USBCharacterMovementComponent* CMC = Character->GetSBMovement();
        if (CMC)
        {
            CMC->SetAiming(false);
        }
    }
}

void USBMobileTouchWidget::OnReloadButtonPressed()
{
    OnReloadPressed.Broadcast();
}

void USBMobileTouchWidget::OnInteractButtonPressed()
{
    OnInteractPressed.Broadcast();

    ASBCharacterBase* Character = GetOwningCharacter();
    if (!Character) return;

    USBCharacterMovementComponent* CMC = Character->GetSBMovement();
    if (CMC)
    {
        CMC->TryMantleOrVault();
    }
}
