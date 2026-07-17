// Copyright Island Of Death Games. All Rights Reserved.

#include "Character/SBCharacterAnimInstance.h"
#include "Character/SBCharacterBase.h"
#include "Character/SBCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "CollisionQueryParams.h"

void USBCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* PawnOwner = TryGetPawnOwner();
    if (!PawnOwner) return;

    OwnerCharacter = Cast<ASBCharacterBase>(PawnOwner);
    if (OwnerCharacter.IsValid())
    {
        MovementComponent = OwnerCharacter->GetSBMovement();
    }

    PreviousRotation = FRotator::ZeroRotator;
    PreviousLean = 0.0f;

    // Initialize all flags to false
    bIsMoving = false;
    bIsInAir = false;
    bIsFalling = false;
    bIsSprinting = false;
    bIsCrouching = false;
    bIsProning = false;
    bIsAiming = false;
    bIsMantling = false;
    bIsVaulting = false;
    bIsSwimming = false;
    bIsOnGround = true;
    bHasWeapon = false;
    bIsFiring = false;
    bIsReloading = false;
    bIsEquipping = false;
    Speed = 0.0f;
    Direction = 0.0f;
    LeanAmount = 0.0f;
    AimPitch = 0.0f;
    AimYaw = 0.0f;
    LeftFootIKOffset = 0.0f;
    RightFootIKOffset = 0.0f;
    HipOffset = 0.0f;
}

void USBCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter.IsValid() || !MovementComponent.IsValid())
    {
        APawn* PawnOwner = TryGetPawnOwner();
        if (!PawnOwner) return;
        OwnerCharacter = Cast<ASBCharacterBase>(PawnOwner);
        if (!OwnerCharacter.IsValid()) return;
        MovementComponent = OwnerCharacter->GetSBMovement();
        if (!MovementComponent.IsValid()) return;
    }

    UpdateLocomotion(DeltaSeconds);
    UpdateMovementState();
    UpdateAimOffset();
    UpdateLean(DeltaSeconds);
    UpdateFootIK();
    UpdateWeaponState();
}

void USBCharacterAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    bIsMoving = Speed > 3.0f;

    Acceleration = MovementComponent->GetCurrentAcceleration();

    if (bIsMoving && OwnerCharacter.IsValid())
    {
        FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Velocity);
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, ActorRotation).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }
}

void USBCharacterAnimInstance::UpdateMovementState()
{
    bIsInAir = MovementComponent->IsFalling() || MovementComponent->IsFlying();
    bIsFalling = MovementComponent->IsFalling();
    bIsOnGround = MovementComponent->IsMovingOnGround();
    bIsSwimming = MovementComponent->IsSwimming();
    bIsCrouching = MovementComponent->IsCrouching();

    bIsSprinting = MovementComponent->IsSprinting();
    bIsProning = MovementComponent->IsProning();
    bIsMantling = MovementComponent->IsMantling();
    bIsVaulting = MovementComponent->IsVaulting();

    if (OwnerCharacter.IsValid())
    {
        bIsAiming = OwnerCharacter->IsAiming();
    }
}

void USBCharacterAnimInstance::UpdateAimOffset()
{
    if (!OwnerCharacter.IsValid()) return;

    APawn* PawnOwner = OwnerCharacter.Get();
    FRotator AimRotation = PawnOwner->GetBaseAimRotation();
    FRotator ActorRotation = PawnOwner->GetActorRotation();

    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, ActorRotation);
    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.0f, 90.0f);
}

void USBCharacterAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter.IsValid() || DeltaSeconds <= 0.0f) return;

    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    float YawDelta = FMath::FindDeltaAngleDegrees(PreviousRotation.Yaw, CurrentRotation.Yaw);
    float TargetLean = FMath::Clamp(YawDelta / DeltaSeconds / 360.0f, -1.0f, 1.0f);

    LeanAmount = FMath::FInterpTo(PreviousLean, TargetLean, DeltaSeconds, 6.0f);
    PreviousLean = LeanAmount;
    PreviousRotation = CurrentRotation;
}

void USBCharacterAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter.IsValid() || !bIsOnGround || bIsProning || bIsSwimming)
    {
        LeftFootIKOffset = 0.0f;
        RightFootIKOffset = 0.0f;
        HipOffset = 0.0f;
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    const FVector ActorLocation = OwnerCharacter->GetActorLocation();
    const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const float TraceDistance = CapsuleHalfHeight + 50.0f;
    const float FootOffset = 5.0f;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter.Get());
    Params.bTraceComplex = false;

    auto TraceFoot = [&](const FName& SocketName) -> float
    {
        USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
        if (!Mesh) return 0.0f;

        FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
        FVector Start = FVector(SocketLocation.X, SocketLocation.Y, ActorLocation.Z);
        FVector End = Start - FVector(0, 0, TraceDistance);

        FHitResult Hit;
        if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
        {
            float IKOffset = (ActorLocation.Z - CapsuleHalfHeight) - Hit.ImpactPoint.Z + FootOffset;
            return FMath::Clamp(IKOffset, -30.0f, 30.0f);
        }
        return 0.0f;
    };

    LeftFootIKOffset = TraceFoot(FName("foot_l"));
    RightFootIKOffset = TraceFoot(FName("foot_r"));

    HipOffset = FMath::Min(LeftFootIKOffset, RightFootIKOffset);
    LeftFootIKOffset -= HipOffset;
    RightFootIKOffset -= HipOffset;
}

void USBCharacterAnimInstance::UpdateWeaponState()
{
    if (!OwnerCharacter.IsValid()) return;

    USBWeaponComponent* WeaponComp = OwnerCharacter->FindComponentByClass<USBWeaponComponent>();
    if (!WeaponComp)
    {
        bHasWeapon = false;
        bIsFiring = false;
        bIsReloading = false;
        bIsEquipping = false;
        return;
    }

    ASBWeaponBase* Weapon = WeaponComp->GetActiveWeapon();
    bHasWeapon = (Weapon != nullptr);

    if (Weapon)
    {
        ESBWeaponState State = Weapon->GetWeaponState();
        bIsFiring = (State == ESBWeaponState::Firing);
        bIsReloading = (State == ESBWeaponState::Reloading);
        bIsEquipping = (State == ESBWeaponState::Equipping);
    }
    else
    {
        bIsFiring = false;
        bIsReloading = false;
        bIsEquipping = false;
    }
}
