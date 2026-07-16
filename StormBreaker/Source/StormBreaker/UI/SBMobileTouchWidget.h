// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SBMobileTouchWidget.generated.h"

class ASBCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMobileFirePressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMobileFireReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMobileADSPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMobileADSReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMobileReloadPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMobileInteractPressed);

/**
 * Mobile touch input widget — provides virtual joysticks and action buttons.
 * Left side: movement joystick.
 * Right side: look touch zone.
 * Action buttons: Fire, ADS, Jump, Crouch, Prone, Sprint, Reload, Interact.
 *
 * Bind to this widget's delegates to receive touch events.
 * Movement and look values are polled each frame by the character.
 */
UCLASS()
class STORMBREAKER_API USBMobileTouchWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // --- Joystick Output (polled by character each frame) ---

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Mobile")
    FVector2D GetMoveInput() const { return MoveInput; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Mobile")
    FVector2D GetLookInput() const { return LookInput; }

    // --- Action Delegates ---

    UPROPERTY(BlueprintAssignable, Category = "StormBreaker|Mobile")
    FOnMobileFirePressed OnFirePressed;

    UPROPERTY(BlueprintAssignable, Category = "StormBreaker|Mobile")
    FOnMobileFireReleased OnFireReleased;

    UPROPERTY(BlueprintAssignable, Category = "StormBreaker|Mobile")
    FOnMobileADSPressed OnADSPressed;

    UPROPERTY(BlueprintAssignable, Category = "StormBreaker|Mobile")
    FOnMobileADSReleased OnADSReleased;

    UPROPERTY(BlueprintAssignable, Category = "StormBreaker|Mobile")
    FOnMobileReloadPressed OnReloadPressed;

    UPROPERTY(BlueprintAssignable, Category = "StormBreaker|Mobile")
    FOnMobileInteractPressed OnInteractPressed;

    // --- Joystick Config ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StormBreaker|Mobile")
    float JoystickDeadzone = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StormBreaker|Mobile")
    float JoystickRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StormBreaker|Mobile")
    float LookSensitivity = 0.5f;

    // --- Called by Blueprint widget buttons ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void SetMoveInput(FVector2D Input);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void SetLookInput(FVector2D Input);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnJumpPressed();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnCrouchPressed();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnPronePressed();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnSprintPressed();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnSprintReleased();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnFireButtonPressed();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnFireButtonReleased();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnADSButtonPressed();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnADSButtonReleased();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnReloadButtonPressed();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OnInteractButtonPressed();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    ASBCharacterBase* GetOwningCharacter() const;

    FVector2D MoveInput;
    FVector2D LookInput;
    FVector2D PreviousLookInput;
};
