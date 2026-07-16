// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SBCharacterAnimInstance.generated.h"

class ASBCharacterBase;
class USBCharacterMovementComponent;

/**
 * Animation instance driving the character's animation blueprint.
 * Updates every frame from the character's movement and state data.
 * All values are safe to read in AnimGraph — updated in NativeUpdateAnimation.
 */
UCLASS()
class STORMBREAKER_API USBCharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // --- Locomotion ---

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    float AimYaw;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    FVector Acceleration;

    // --- Movement State ---

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsMoving : 1;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsInAir : 1;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsFalling : 1;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsSprinting : 1;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsCrouching : 1;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsProning : 1;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsAiming : 1;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsMantling : 1;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsVaulting : 1;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsSwimming : 1;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    uint8 bIsOnGround : 1;

    // --- Lean ---

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    float LeanAmount;

    // --- Foot IK ---

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    float LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    float RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "StormBreaker|Animation")
    float HipOffset;

private:
    UPROPERTY()
    TWeakObjectPtr<ASBCharacterBase> OwnerCharacter;

    UPROPERTY()
    TWeakObjectPtr<USBCharacterMovementComponent> MovementComponent;

    void UpdateLocomotion(float DeltaSeconds);
    void UpdateMovementState();
    void UpdateAimOffset();
    void UpdateLean(float DeltaSeconds);
    void UpdateFootIK();

    FRotator PreviousRotation;
    float PreviousLean;
};
