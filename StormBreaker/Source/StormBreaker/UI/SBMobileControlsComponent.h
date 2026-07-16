// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SBMobileControlsComponent.generated.h"

UENUM(BlueprintType)
enum class ESBPeekDirection : uint8
{
    None,
    Left,
    Right
};

/**
 * Mobile-specific controls component.
 * Handles gyroscope aiming, peek left/right, auto-pickup toggle,
 * custom HUD layout positions, and healing wheel selection.
 * Attached to player character on mobile platforms.
 */
UCLASS(ClassGroup = "StormBreaker", meta = (BlueprintSpawnableComponent))
class STORMBREAKER_API USBMobileControlsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USBMobileControlsComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Gyroscope ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void SetGyroscopeEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Mobile")
    bool IsGyroscopeEnabled() const { return bGyroscopeEnabled; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StormBreaker|Mobile")
    float GyroscopeSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StormBreaker|Mobile")
    bool bGyroscopeOnlyWhileADS = true;

    // --- Peek ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void StartPeek(ESBPeekDirection Direction);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void StopPeek();

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Mobile")
    ESBPeekDirection GetPeekDirection() const { return CurrentPeek; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StormBreaker|Mobile")
    float PeekDistance = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StormBreaker|Mobile")
    float PeekSpeed = 10.0f;

    // --- Auto Pickup ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StormBreaker|Mobile")
    bool bAutoPickupEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StormBreaker|Mobile")
    float AutoPickupRadius = 300.0f;

    // --- Healing Wheel ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void OpenHealingWheel();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void CloseHealingWheel();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Mobile")
    void SelectHealingItem(int32 ItemIndex);

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Mobile")
    bool IsHealingWheelOpen() const { return bHealingWheelOpen; }

private:
    void TickGyroscope(float DeltaTime);
    void TickPeek(float DeltaTime);

    bool bGyroscopeEnabled;
    ESBPeekDirection CurrentPeek;
    float CurrentPeekOffset;
    bool bHealingWheelOpen;
};
