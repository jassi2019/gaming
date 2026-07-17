// Copyright Island Of Death Games. All Rights Reserved.

#include "Character/SBCharacterMovementComponent.h"
#include "StormBreaker.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

// ============================================================================
// Movement Component
// ============================================================================

USBCharacterMovementComponent::USBCharacterMovementComponent()
{
    bWantsToSprint = false;
    bIsProning = false;
    bIsMantling = false;
    bIsVaulting = false;
    bIsAiming = false;
    MantleElapsed = 0.0f;
    MantleTotalDuration = 0.0f;
    StoredHalfHeight = 0.0f;

    // Defaults
    MaxWalkSpeed = RunSpeed;
    MaxWalkSpeedCrouched = CrouchSpeed;
    MaxSwimSpeed = SwimSpeed;
    NavAgentProps.bCanCrouch = true;
    bCanWalkOffLedgesWhenCrouching = true;
    SetCrouchedHalfHeight(50.0f);
    AirControl = 0.35f;
    JumpZVelocity = 500.0f;
    BrakingDecelerationWalking = 1400.0f;
    BrakingDecelerationFalling = 200.0f;

    SetIsReplicatedByDefault(true);
}

void USBCharacterMovementComponent::InitializeComponent()
{
    Super::InitializeComponent();
    MaxWalkSpeed = RunSpeed;
}

// --- Speed ---

float USBCharacterMovementComponent::GetMaxSpeed() const
{
    if (bIsMantling || bIsVaulting)
    {
        return 0.0f;
    }

    switch (MovementMode)
    {
    case MOVE_Walking:
    case MOVE_NavWalking:
        if (bIsProning)
        {
            return ProneSpeed;
        }
        if (IsCrouching())
        {
            return CrouchSpeed;
        }
        if (bIsAiming)
        {
            return ADSSpeed;
        }
        if (bWantsToSprint && !bIsAiming)
        {
            return SprintSpeed;
        }
        return RunSpeed;

    case MOVE_Swimming:
        return SwimSpeed;

    case MOVE_Custom:
        if (CustomMovementMode == static_cast<uint8>(ESBCustomMovement::Prone))
        {
            return ProneSpeed;
        }
        return 0.0f;

    default:
        return Super::GetMaxSpeed();
    }
}

float USBCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
    if (bIsProning)
    {
        return BrakingDecelerationWalking * 2.0f;
    }
    return Super::GetMaxBrakingDeceleration();
}

bool USBCharacterMovementComponent::CanCrouchInCurrentState() const
{
    if (bIsProning || bIsMantling || bIsVaulting)
    {
        return false;
    }
    return Super::CanCrouchInCurrentState();
}

bool USBCharacterMovementComponent::CanAttemptJump() const
{
    if (bIsProning || bIsMantling || bIsVaulting)
    {
        return false;
    }
    return Super::CanAttemptJump();
}

bool USBCharacterMovementComponent::DoJump(bool bReplayingMoves, float DeltaTime)
{
    if (bIsProning)
    {
        StopProne();
        return false;
    }
    return Super::DoJump(bReplayingMoves, DeltaTime);
}

void USBCharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations)
{
    Super::ProcessLanded(Hit, RemainingTime, Iterations);

    if (bWantsToSprint)
    {
        MaxWalkSpeed = SprintSpeed;
    }
}

void USBCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

    if (MovementMode == MOVE_Swimming)
    {
        if (bIsProning)
        {
            StopProne();
        }
        if (IsCrouching())
        {
            bWantsToCrouch = false;
            bWantsToCrouch = false;
        }
    }
}

// --- Sprint ---

void USBCharacterMovementComponent::StartSprinting()
{
    if (bIsProning || bIsMantling || bIsVaulting)
    {
        return;
    }
    bWantsToSprint = true;
    if (IsCrouching())
    {
        bWantsToCrouch = false;
        bWantsToCrouch = false;
    }
}

void USBCharacterMovementComponent::StopSprinting()
{
    bWantsToSprint = false;
}

// --- Prone ---

void USBCharacterMovementComponent::StartProne()
{
    if (!IsMovingOnGround() || bIsMantling || bIsVaulting || MovementMode == MOVE_Swimming)
    {
        return;
    }

    if (bIsProning)
    {
        StopProne();
        return;
    }

    bWantsToSprint = false;
    if (IsCrouching())
    {
        bWantsToCrouch = false;
        bWantsToCrouch = false;
    }

    bIsProning = true;
    UpdateProneCollision(true);
    OnCustomMovementChanged.Broadcast(ESBCustomMovement::Prone);

    UE_LOG(LogSBCharacter, Verbose, TEXT("Entered prone."));
}

void USBCharacterMovementComponent::StopProne()
{
    if (!bIsProning)
    {
        return;
    }

    if (!CanStandFromProne())
    {
        UE_LOG(LogSBCharacter, Verbose, TEXT("Cannot stand from prone — blocked."));
        return;
    }

    bIsProning = false;
    UpdateProneCollision(false);
    OnCustomMovementChanged.Broadcast(ESBCustomMovement::None);

    UE_LOG(LogSBCharacter, Verbose, TEXT("Exited prone."));
}

void USBCharacterMovementComponent::UpdateProneCollision(bool bEnteringProne)
{
    ACharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    UCapsuleComponent* Capsule = Owner->GetCapsuleComponent();
    if (!Capsule) return;

    if (bEnteringProne)
    {
        StoredHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
        Capsule->SetCapsuleHalfHeight(ProneHalfHeight);

        // Offset character down so they stay on ground
        float HeightDiff = StoredHalfHeight - ProneHalfHeight;
        FVector NewLocation = Owner->GetActorLocation();
        NewLocation.Z -= HeightDiff;
        Owner->SetActorLocation(NewLocation);
    }
    else
    {
        float HeightDiff = StoredHalfHeight - ProneHalfHeight;
        Capsule->SetCapsuleHalfHeight(StoredHalfHeight);

        FVector NewLocation = Owner->GetActorLocation();
        NewLocation.Z += HeightDiff;
        Owner->SetActorLocation(NewLocation);
    }
}

bool USBCharacterMovementComponent::CanStandFromProne() const
{
    ACharacter* Owner = GetCharacterOwner();
    if (!Owner) return true;

    UCapsuleComponent* Capsule = Owner->GetCapsuleComponent();
    if (!Capsule) return true;

    float StandRadius = Capsule->GetUnscaledCapsuleRadius();
    float StandHalfHeight = StoredHalfHeight;

    FVector TraceStart = Owner->GetActorLocation();
    TraceStart.Z += (StandHalfHeight - ProneHalfHeight);

    FCollisionShape StandShape = FCollisionShape::MakeCapsule(StandRadius, StandHalfHeight);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    FHitResult Hit;
    bool bBlocked = GetWorld()->SweepSingleByChannel(
        Hit, TraceStart, TraceStart, FQuat::Identity,
        ECC_Pawn, StandShape, Params);

    return !bBlocked;
}

void USBCharacterMovementComponent::PhysProne(float DeltaTime, int32 Iterations)
{
    // Prone uses walking physics with reduced speed
    PhysWalking(DeltaTime, Iterations);
}

// --- Mantle / Vault ---

bool USBCharacterMovementComponent::TryMantleOrVault()
{
    if (bIsMantling || bIsVaulting || !IsMovingOnGround())
    {
        return false;
    }

    FVector LedgeLocation;
    FVector LedgeNormal;
    float LedgeHeight;

    if (!TraceForMantleOrVault(LedgeLocation, LedgeNormal, LedgeHeight))
    {
        return false;
    }

    if (LedgeHeight <= MaxVaultHeight)
    {
        ExecuteVault(LedgeLocation, LedgeHeight);
        return true;
    }
    else if (LedgeHeight <= MaxMantleHeight)
    {
        ExecuteMantle(LedgeLocation, LedgeHeight);
        return true;
    }

    return false;
}

bool USBCharacterMovementComponent::TraceForMantleOrVault(
    FVector& OutLedgeLocation, FVector& OutLedgeNormal, float& OutLedgeHeight) const
{
    ACharacter* Owner = GetCharacterOwner();
    if (!Owner) return false;

    const FVector ActorLocation = Owner->GetActorLocation();
    const FVector Forward = Owner->GetActorForwardVector();

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    // Forward trace — detect wall/obstacle
    const float ForwardDist = MantleReachDistance + Owner->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const FVector ForwardStart = ActorLocation + FVector(0, 0, 50.0f);
    const FVector ForwardEnd = ForwardStart + Forward * ForwardDist;

    FHitResult ForwardHit;
    bool bHitForward = GetWorld()->LineTraceSingleByChannel(
        ForwardHit, ForwardStart, ForwardEnd, ECC_WorldStatic, Params);

    if (!bHitForward || !ForwardHit.bBlockingHit)
    {
        return false;
    }

    // Don't mantle very steep or overhanging surfaces
    if (ForwardHit.ImpactNormal.Z < -0.1f)
    {
        return false;
    }

    OutLedgeNormal = ForwardHit.ImpactNormal;
    const FVector WallPoint = ForwardHit.ImpactPoint;

    // Downward trace from above the wall — find top surface
    const FVector DownTraceStart = FVector(WallPoint.X, WallPoint.Y, ActorLocation.Z + MaxMantleHeight + 50.0f)
                                   + Forward * 30.0f;
    const FVector DownTraceEnd = FVector(WallPoint.X, WallPoint.Y, ActorLocation.Z)
                                 + Forward * 30.0f;

    FHitResult TopHit;
    bool bHitTop = GetWorld()->LineTraceSingleByChannel(
        TopHit, DownTraceStart, DownTraceEnd, ECC_WorldStatic, Params);

    if (!bHitTop || !TopHit.bBlockingHit)
    {
        return false;
    }

    // Must be a walkable surface
    if (TopHit.ImpactNormal.Z < GetWalkableFloorZ())
    {
        return false;
    }

    OutLedgeLocation = TopHit.ImpactPoint;
    OutLedgeHeight = TopHit.ImpactPoint.Z - ActorLocation.Z +
                     Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

    // Minimum height check — don't vault tiny steps
    if (OutLedgeHeight < MaxStepHeight)
    {
        return false;
    }

    // Space check — can we fit on top?
    const float CapsuleRadius = Owner->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float CapsuleHalfHeight = Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    FVector FitCheckLocation = OutLedgeLocation + FVector(0, 0, CapsuleHalfHeight + 5.0f);
    FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

    FHitResult FitHit;
    bool bBlocked = GetWorld()->SweepSingleByChannel(
        FitHit, FitCheckLocation, FitCheckLocation, FQuat::Identity,
        ECC_Pawn, CapsuleShape, Params);

    return !bBlocked;
}

void USBCharacterMovementComponent::ExecuteVault(const FVector& LedgeLocation, float LedgeHeight)
{
    ACharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    bIsVaulting = true;
    MantleStartLocation = Owner->GetActorLocation();

    // Vault target: over the obstacle and slightly forward
    const FVector Forward = Owner->GetActorForwardVector();
    const float CapsuleHalfHeight = Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    MantleTargetLocation = LedgeLocation + FVector(0, 0, CapsuleHalfHeight) + Forward * 100.0f;

    MantleElapsed = 0.0f;
    MantleTotalDuration = VaultDuration;

    SetMovementMode(MOVE_Flying);
    OnCustomMovementChanged.Broadcast(ESBCustomMovement::Vault);

    UE_LOG(LogSBCharacter, Verbose, TEXT("Vault started. Height: %.1f"), LedgeHeight);
}

void USBCharacterMovementComponent::ExecuteMantle(const FVector& LedgeLocation, float LedgeHeight)
{
    ACharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    bIsMantling = true;
    MantleStartLocation = Owner->GetActorLocation();

    const float CapsuleHalfHeight = Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    MantleTargetLocation = LedgeLocation + FVector(0, 0, CapsuleHalfHeight + 5.0f);

    MantleElapsed = 0.0f;
    MantleTotalDuration = MantleDuration;

    SetMovementMode(MOVE_Flying);
    OnCustomMovementChanged.Broadcast(ESBCustomMovement::Mantle);

    UE_LOG(LogSBCharacter, Verbose, TEXT("Mantle started. Height: %.1f"), LedgeHeight);
}

void USBCharacterMovementComponent::UpdateMantleVault(float DeltaTime)
{
    if (!bIsMantling && !bIsVaulting)
    {
        return;
    }

    ACharacter* Owner = GetCharacterOwner();
    if (!Owner)
    {
        FinishMantleVault();
        return;
    }

    MantleElapsed += DeltaTime;
    float Alpha = FMath::Clamp(MantleElapsed / MantleTotalDuration, 0.0f, 1.0f);

    // Smooth ease-in-out curve
    float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

    FVector NewLocation;
    if (bIsVaulting)
    {
        // Vault uses a parabolic arc
        FVector LinearPos = FMath::Lerp(MantleStartLocation, MantleTargetLocation, EasedAlpha);
        float ArcHeight = 40.0f;
        float ArcOffset = ArcHeight * FMath::Sin(Alpha * PI);
        NewLocation = LinearPos + FVector(0, 0, ArcOffset);
    }
    else
    {
        // Mantle: move up first, then forward
        if (Alpha < 0.6f)
        {
            float VertAlpha = Alpha / 0.6f;
            float EasedVert = FMath::InterpEaseOut(0.0f, 1.0f, VertAlpha, 2.0f);
            FVector VertTarget = FVector(MantleStartLocation.X, MantleStartLocation.Y, MantleTargetLocation.Z);
            NewLocation = FMath::Lerp(MantleStartLocation, VertTarget, EasedVert);
        }
        else
        {
            float HorizAlpha = (Alpha - 0.6f) / 0.4f;
            float EasedHoriz = FMath::InterpEaseIn(0.0f, 1.0f, HorizAlpha, 2.0f);
            FVector VertTarget = FVector(MantleStartLocation.X, MantleStartLocation.Y, MantleTargetLocation.Z);
            NewLocation = FMath::Lerp(VertTarget, MantleTargetLocation, EasedHoriz);
        }
    }

    Owner->SetActorLocation(NewLocation);
    Velocity = FVector::ZeroVector;

    if (Alpha >= 1.0f)
    {
        FinishMantleVault();
    }
}

void USBCharacterMovementComponent::FinishMantleVault()
{
    bool bWasVault = bIsVaulting;
    bIsMantling = false;
    bIsVaulting = false;
    MantleElapsed = 0.0f;

    SetMovementMode(MOVE_Walking);
    OnCustomMovementChanged.Broadcast(ESBCustomMovement::None);

    UE_LOG(LogSBCharacter, Verbose, TEXT("%s finished."), bWasVault ? TEXT("Vault") : TEXT("Mantle"));
}

// --- Custom Physics ---

void USBCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    Super::PhysCustom(DeltaTime, Iterations);

    switch (static_cast<ESBCustomMovement>(CustomMovementMode))
    {
    case ESBCustomMovement::Prone:
        PhysProne(DeltaTime, Iterations);
        break;
    default:
        break;
    }
}

void USBCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
    Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

    // Auto-stop sprint when not moving forward
    if (bWantsToSprint && Velocity.Size2D() < 10.0f)
    {
        // Don't auto-stop, just don't apply sprint speed — GetMaxSpeed handles this
    }
}

void USBCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
    Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
}

void USBCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateMantleVault(DeltaTime);
}

// --- ADS ---

void USBCharacterMovementComponent::SetAiming(bool bNewAiming)
{
    bIsAiming = bNewAiming;
    if (bNewAiming && bWantsToSprint)
    {
        StopSprinting();
    }
}

// --- Network Prediction ---

FNetworkPredictionData_Client* USBCharacterMovementComponent::GetPredictionData_Client() const
{
    if (!ClientPredictionData)
    {
        USBCharacterMovementComponent* MutableThis = const_cast<USBCharacterMovementComponent*>(this);
        MutableThis->ClientPredictionData = new FSBNetworkPredictionData_Client(*this);
    }
    return ClientPredictionData;
}

void USBCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);

    bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
    bIsProning = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
    bIsAiming = (Flags & FSavedMove_Character::FLAG_Custom_2) != 0;
}

// ============================================================================
// Saved Move
// ============================================================================

FSBSavedMove::FSBSavedMove()
    : bSavedWantsToSprint(false)
    , bSavedIsProning(false)
    , bSavedIsAiming(false)
{
}

void FSBSavedMove::Clear()
{
    Super::Clear();
    bSavedWantsToSprint = false;
    bSavedIsProning = false;
    bSavedIsAiming = false;
}

uint8 FSBSavedMove::GetCompressedFlags() const
{
    uint8 Result = Super::GetCompressedFlags();

    if (bSavedWantsToSprint)
    {
        Result |= FLAG_Custom_0;
    }
    if (bSavedIsProning)
    {
        Result |= FLAG_Custom_1;
    }
    if (bSavedIsAiming)
    {
        Result |= FLAG_Custom_2;
    }

    return Result;
}

bool FSBSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
    const FSBSavedMove* SBNewMove = static_cast<const FSBSavedMove*>(NewMove.Get());
    if (!SBNewMove)
    {
        return false;
    }

    if (bSavedWantsToSprint != SBNewMove->bSavedWantsToSprint ||
        bSavedIsProning != SBNewMove->bSavedIsProning ||
        bSavedIsAiming != SBNewMove->bSavedIsAiming)
    {
        return false;
    }

    return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSBSavedMove::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
    FNetworkPredictionData_Client_Character& ClientData)
{
    Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

    USBCharacterMovementComponent* CMC = Cast<USBCharacterMovementComponent>(C->GetCharacterMovement());
    if (CMC)
    {
        bSavedWantsToSprint = CMC->bWantsToSprint;
        bSavedIsProning = CMC->bIsProning;
        bSavedIsAiming = CMC->bIsAiming;
    }
}

void FSBSavedMove::PrepMoveFor(ACharacter* C)
{
    Super::PrepMoveFor(C);

    USBCharacterMovementComponent* CMC = Cast<USBCharacterMovementComponent>(C->GetCharacterMovement());
    if (CMC)
    {
        CMC->bWantsToSprint = bSavedWantsToSprint;
        CMC->bIsProning = bSavedIsProning;
        CMC->bIsAiming = bSavedIsAiming;
    }
}

// ============================================================================
// Network Prediction Data
// ============================================================================

FSBNetworkPredictionData_Client::FSBNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement)
    : Super(ClientMovement)
{
}

FSavedMovePtr FSBNetworkPredictionData_Client::AllocateNewMove()
{
    return FSavedMovePtr(new FSBSavedMove());
}
