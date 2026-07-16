// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "SBCharacterBase.generated.h"

class USBCharacterMovementComponent;
class USBWeaponComponent;
class USBInventoryComponent;
class USBKnockReviveComponent;
class USBParachuteComponent;
class USBMinimapDataComponent;
class USBAntiCheatComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAbilitySystemComponent;
class USBAttributeSet;
class USBCharacterAnimInstance;

UENUM(BlueprintType)
enum class ESBStance : uint8
{
    Standing,
    Crouching,
    Prone
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStanceChanged, ESBStance, OldStance, ESBStance, NewStance);

/**
 * Base playable character.
 *
 * Movement: Walk, Run, Sprint, Jump, Crouch, Prone, Vault, Mantle, Swimming.
 * Camera: Third-person with smooth ADS transition.
 * Input: Enhanced Input System with keyboard/mouse + mobile touch.
 * Networking: All movement states replicated via custom CMC saved move.
 */
UCLASS()
class STORMBREAKER_API ASBCharacterBase : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ASBCharacterBase(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // --- Components ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Components")
    TObjectPtr<UCameraComponent> FollowCamera;

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Movement")
    USBCharacterMovementComponent* GetSBMovement() const { return SBMovementComponent; }

    // --- Input Actions (set in Blueprint or C++ defaults) ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_Move;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_Look;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_Jump;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_Sprint;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_Crouch;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_Prone;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_ADS;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_Interact;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_Fire;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_Reload;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_WeaponSlot1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_WeaponSlot2;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputAction> IA_WeaponSlot3;

    // --- Weapon ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Components")
    TObjectPtr<USBWeaponComponent> WeaponComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Components")
    TObjectPtr<USBInventoryComponent> InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Components")
    TObjectPtr<USBKnockReviveComponent> KnockReviveComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Components")
    TObjectPtr<USBParachuteComponent> ParachuteComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Components")
    TObjectPtr<USBMinimapDataComponent> MinimapDataComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Components")
    TObjectPtr<USBAntiCheatComponent> AntiCheatComponent;

    // --- Stance ---

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Movement")
    ESBStance GetCurrentStance() const;

    UPROPERTY(BlueprintAssignable, Category = "StormBreaker|Movement")
    FOnStanceChanged OnStanceChanged;

    // --- ADS ---

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Combat")
    bool IsAiming() const { return bIsAiming; }

    // --- Camera Config ---

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Camera")
    float DefaultBoomLength = 300.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Camera")
    float ADSBoomLength = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Camera")
    float DefaultFOV = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Camera")
    float ADSFOV = 65.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Camera")
    float ADSTransitionSpeed = 12.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Camera")
    FVector CameraSocketOffset = FVector(0.0f, 60.0f, 70.0f);

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Camera")
    FVector ADSCameraOffset = FVector(0.0f, 80.0f, 60.0f);

protected:
    // --- Input Handlers ---
    void Input_Move(const FInputActionValue& Value);
    void Input_Look(const FInputActionValue& Value);
    void Input_JumpStart(const FInputActionValue& Value);
    void Input_JumpStop(const FInputActionValue& Value);
    void Input_SprintStart(const FInputActionValue& Value);
    void Input_SprintStop(const FInputActionValue& Value);
    void Input_CrouchToggle(const FInputActionValue& Value);
    void Input_ProneToggle(const FInputActionValue& Value);
    void Input_ADSStart(const FInputActionValue& Value);
    void Input_ADSStop(const FInputActionValue& Value);
    void Input_Interact(const FInputActionValue& Value);
    void Input_FireStart(const FInputActionValue& Value);
    void Input_FireStop(const FInputActionValue& Value);
    void Input_Reload(const FInputActionValue& Value);
    void Input_WeaponSlot1(const FInputActionValue& Value);
    void Input_WeaponSlot2(const FInputActionValue& Value);
    void Input_WeaponSlot3(const FInputActionValue& Value);

    // --- Camera ---
    void UpdateCameraADS(float DeltaTime);

    // --- GAS ---
    void InitAbilitySystem();

    // --- Replicated State ---
    UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
    uint8 bIsAiming : 1;

    UFUNCTION()
    void OnRep_IsAiming();

    UFUNCTION(Server, Reliable)
    void Server_SetAiming(bool bNewAiming);

private:
    UPROPERTY()
    TObjectPtr<USBCharacterMovementComponent> SBMovementComponent;

    float CurrentBoomLength;
    float CurrentFOV;
};
