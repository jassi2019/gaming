// Copyright Island Of Death Games. All Rights Reserved.

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

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Mobile")
    FVector2D GetMoveInput() const { return MoveInput; }

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Mobile")
    FVector2D GetLookInput() const { return LookInput; }

    // --- Action Delegates ---

    UPROPERTY(BlueprintAssignable, Category = "IslandOfDeath|Mobile")
    FOnMobileFirePressed OnFirePressed;

    UPROPERTY(BlueprintAssignable, Category = "IslandOfDeath|Mobile")
    FOnMobileFireReleased OnFireReleased;

    UPROPERTY(BlueprintAssignable, Category = "IslandOfDeath|Mobile")
    FOnMobileADSPressed OnADSPressed;

    UPROPERTY(BlueprintAssignable, Category = "IslandOfDeath|Mobile")
    FOnMobileADSReleased OnADSReleased;

    UPROPERTY(BlueprintAssignable, Category = "IslandOfDeath|Mobile")
    FOnMobileReloadPressed OnReloadPressed;

    UPROPERTY(BlueprintAssignable, Category = "IslandOfDeath|Mobile")
    FOnMobileInteractPressed OnInteractPressed;

    // --- Joystick Config ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Mobile")
    float JoystickDeadzone = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Mobile")
    float JoystickRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Mobile")
    float LookSensitivity = 0.5f;

    // --- Called by Blueprint widget buttons ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void SetMoveInput(FVector2D Input);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void SetLookInput(FVector2D Input);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OnJumpPressed();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OnCrouchPressed();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OnPronePressed();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OnSprintPressed();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OnSprintReleased();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OnFireButtonPressed();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OnFireButtonReleased();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OnADSButtonPressed();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OnADSButtonReleased();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OnReloadButtonPressed();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
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
