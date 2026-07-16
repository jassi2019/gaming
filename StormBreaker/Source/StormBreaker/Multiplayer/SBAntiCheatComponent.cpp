// Copyright StormBreaker Games. All Rights Reserved.

#include "Multiplayer/SBAntiCheatComponent.h"
#include "StormBreaker.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

USBAntiCheatComponent::USBAntiCheatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    ViolationCount = 0;
    TimeSinceLastSpeedCheck = 0.0f;
    ViolationDecayTimer = 0.0f;
    bHasLastPosition = false;
}

void USBAntiCheatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Only run on server
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;

    // Only validate player-controlled characters
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn || !Pawn->IsPlayerControlled()) return;

    ValidateMovement(DeltaTime);
    ValidatePosition();

    // Decay violations over time
    ViolationDecayTimer += DeltaTime;
    if (ViolationDecayTimer >= 1.0f && ViolationCount > 0)
    {
        ViolationDecayTimer = 0.0f;
        ViolationCount = FMath::Max(0, ViolationCount - (int32)ViolationDecayRate);
    }
}

void USBAntiCheatComponent::ValidateMovement(float DeltaTime)
{
    TimeSinceLastSpeedCheck += DeltaTime;
    if (TimeSinceLastSpeedCheck < SpeedCheckInterval) return;
    TimeSinceLastSpeedCheck = 0.0f;

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;

    float CurrentSpeed = Character->GetVelocity().Size();

    // Allow some tolerance for physics interactions and lag
    float Tolerance = MaxAllowedSpeed * 1.2f;

    if (CurrentSpeed > Tolerance)
    {
        AddViolation(FString::Printf(TEXT("Speed hack: %.0f cm/s (max: %.0f)"),
            CurrentSpeed, MaxAllowedSpeed));
    }
}

void USBAntiCheatComponent::ValidatePosition()
{
    FVector CurrentPosition = GetOwner()->GetActorLocation();

    if (bHasLastPosition)
    {
        float Distance = FVector::Dist(LastValidatedPosition, CurrentPosition);

        if (Distance > TeleportThreshold)
        {
            // Check if this is a legitimate teleport (respawn, vehicle exit, etc.)
            ACharacter* Character = Cast<ACharacter>(GetOwner());
            bool bLegitimate = false;

            if (Character)
            {
                UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
                if (CMC && CMC->MovementMode == MOVE_None)
                {
                    bLegitimate = true;
                }
            }

            if (!bLegitimate)
            {
                AddViolation(FString::Printf(TEXT("Teleport: %.0f units in one tick (threshold: %.0f)"),
                    Distance, TeleportThreshold));
            }
        }
    }

    LastValidatedPosition = CurrentPosition;
    bHasLastPosition = true;
}

void USBAntiCheatComponent::AddViolation(const FString& Reason)
{
    ViolationCount++;

    UE_LOG(LogSBMultiplayer, Warning, TEXT("Anti-cheat violation [%d/%d] on %s: %s"),
        ViolationCount, MaxViolationsBeforeKick, *GetNameSafe(GetOwner()), *Reason);

    OnCheatDetected.Broadcast(GetOwner(), Reason);

    if (ViolationCount >= MaxViolationsBeforeKick)
    {
        APawn* Pawn = Cast<APawn>(GetOwner());
        if (Pawn)
        {
            APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
            if (PC)
            {
                UE_LOG(LogSBMultiplayer, Error, TEXT("KICKING player %s — %d violations reached."),
                    *GetNameSafe(PC), ViolationCount);

                // In production, call GameMode->GameSession->KickPlayer()
                // For now, log the event
            }
        }
    }
}
