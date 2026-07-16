// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SBCharacterMovementComponent.generated.h"

// Custom movement modes stored in CustomMovementMode (uint8)
UENUM(BlueprintType)
enum class ESBCustomMovement : uint8
{
    None    = 0,
    Prone   = 1,
    Mantle  = 2,
    Vault   = 3
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementModeChangedSB, ESBCustomMovement, NewMode);

/**
 * Extended character movement component.
 * Adds: Prone, Mantle, Vault, enhanced Swimming, Sprint speed management.
 * All modes are network-predicted via saved move extension.
 */
UCLASS()
class STORMBREAKER_API USBCharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    USBCharacterMovementComponent();

    // --- Engine Overrides ---
    virtual void InitializeComponent() override;
    virtual float GetMaxSpeed() const override;
    virtual float GetMaxBrakingDeceleration() const override;
    virtual bool CanCrouchInCurrentState() const override;
    virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
    virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
    virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
    virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
    virtual bool CanAttemptJump() const override;
    virtual bool DoJump(bool bReplayingMoves, float DeltaTime) override;
    virtual void ProcessLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations) override;

    // --- Network Prediction ---
    virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;

    // --- Sprint ---
    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Movement")
    void StartSprinting();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Movement")
    void StopSprinting();

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Movement")
    bool IsSprinting() const { return bWantsToSprint && IsMovingOnGround(); }

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Sprint")
    float SprintSpeed = 800.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Sprint")
    float SprintStaminaDrainPerSec = 15.0f;

    // --- Prone ---
    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Movement")
    void StartProne();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Movement")
    void StopProne();

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Movement")
    bool IsProning() const { return bIsProning; }

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Prone")
    float ProneSpeed = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Prone")
    float ProneHalfHeight = 30.0f;

    // --- Mantle / Vault ---
    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Movement")
    bool TryMantleOrVault();

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Movement")
    bool IsMantling() const { return bIsMantling; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Movement")
    bool IsVaulting() const { return bIsVaulting; }

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Mantle")
    float MaxVaultHeight = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Mantle")
    float MaxMantleHeight = 250.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Mantle")
    float MantleReachDistance = 75.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Mantle")
    float MantleDuration = 0.8f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Mantle")
    float VaultDuration = 0.4f;

    // --- Speed Config ---
    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Speed")
    float WalkSpeed = 250.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Speed")
    float RunSpeed = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Speed")
    float CrouchSpeed = 200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Speed")
    float SwimSpeed = 300.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Movement|Speed")
    float ADSSpeed = 200.0f;

    // --- ADS ---
    void SetAiming(bool bNewAiming);
    UFUNCTION(BlueprintPure, Category = "StormBreaker|Movement")
    bool IsAiming() const { return bIsAiming; }

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "StormBreaker|Movement")
    FOnMovementModeChangedSB OnCustomMovementChanged;

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // --- Mantle/Vault internals ---
    bool TraceForMantleOrVault(FVector& OutLedgeLocation, FVector& OutLedgeNormal, float& OutLedgeHeight) const;
    void ExecuteMantle(const FVector& LedgeLocation, float LedgeHeight);
    void ExecuteVault(const FVector& LedgeLocation, float LedgeHeight);
    void UpdateMantleVault(float DeltaTime);
    void FinishMantleVault();

    // --- Prone internals ---
    void PhysProne(float DeltaTime, int32 Iterations);
    bool CanStandFromProne() const;
    void UpdateProneCollision(bool bEnteringProne);

    friend class FSBSavedMove;

    // --- Flags (replicated via saved move) ---
    uint8 bWantsToSprint : 1;
    uint8 bIsProning : 1;
    uint8 bIsMantling : 1;
    uint8 bIsVaulting : 1;
    uint8 bIsAiming : 1;

    // --- Mantle/Vault animation state ---
    FVector MantleStartLocation;
    FVector MantleTargetLocation;
    float MantleElapsed;
    float MantleTotalDuration;
    float StoredHalfHeight;
};

// --- Network Prediction Extensions ---

class FSBSavedMove : public FSavedMove_Character
{
public:
    typedef FSavedMove_Character Super;

    uint8 bSavedWantsToSprint : 1;
    uint8 bSavedIsProning : 1;
    uint8 bSavedIsAiming : 1;

    FSBSavedMove();
    virtual void Clear() override;
    virtual uint8 GetCompressedFlags() const override;
    virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
    virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
    virtual void PrepMoveFor(ACharacter* C) override;
};

class FSBNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
{
public:
    typedef FNetworkPredictionData_Client_Character Super;

    FSBNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement);
    virtual FSavedMovePtr AllocateNewMove() override;
};
