// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SBAntiCheatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheatDetected, AActor*, Cheater, FString, Reason);

/**
 * Server-side anti-cheat validation component.
 * Attached to player characters on the server.
 * Validates movement speed, teleportation, fire rate, and position consistency.
 */
UCLASS(ClassGroup = "StormBreaker", meta = (BlueprintSpawnableComponent))
class STORMBREAKER_API USBAntiCheatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USBAntiCheatComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Validation ---

    UFUNCTION(BlueprintPure, Category = "StormBreaker|AntiCheat")
    int32 GetViolationCount() const { return ViolationCount; }

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|AntiCheat")
    float MaxAllowedSpeed = 1200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|AntiCheat")
    float TeleportThreshold = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|AntiCheat")
    float SpeedCheckInterval = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|AntiCheat")
    int32 MaxViolationsBeforeKick = 10;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|AntiCheat")
    float ViolationDecayRate = 1.0f;

    // --- Delegate ---

    UPROPERTY(BlueprintAssignable)
    FOnCheatDetected OnCheatDetected;

private:
    void ValidateMovement(float DeltaTime);
    void ValidatePosition();
    void AddViolation(const FString& Reason);

    FVector LastValidatedPosition;
    float TimeSinceLastSpeedCheck;
    float ViolationDecayTimer;
    int32 ViolationCount;
    bool bHasLastPosition;
};
