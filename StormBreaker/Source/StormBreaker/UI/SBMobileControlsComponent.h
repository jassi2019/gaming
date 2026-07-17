// Copyright Island Of Death Games. All Rights Reserved.

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

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void SetGyroscopeEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Mobile")
    bool IsGyroscopeEnabled() const { return bGyroscopeEnabled; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Mobile")
    float GyroscopeSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Mobile")
    bool bGyroscopeOnlyWhileADS = true;

    // --- Peek ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void StartPeek(ESBPeekDirection Direction);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void StopPeek();

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Mobile")
    ESBPeekDirection GetPeekDirection() const { return CurrentPeek; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Mobile")
    float PeekDistance = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Mobile")
    float PeekSpeed = 10.0f;

    // --- Auto Pickup ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Mobile")
    bool bAutoPickupEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Mobile")
    float AutoPickupRadius = 300.0f;

    // --- Healing Wheel ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void OpenHealingWheel();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void CloseHealingWheel();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Mobile")
    void SelectHealingItem(int32 ItemIndex);

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Mobile")
    bool IsHealingWheelOpen() const { return bHealingWheelOpen; }

private:
    void TickGyroscope(float DeltaTime);
    void TickPeek(float DeltaTime);

    bool bGyroscopeEnabled;
    ESBPeekDirection CurrentPeek;
    float CurrentPeekOffset;
    bool bHealingWheelOpen;
};
