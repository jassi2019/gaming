// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/SBWeaponTypes.h"
#include "SBWeaponBase.generated.h"

class USBWeaponDataAsset;
class USkeletalMeshComponent;
class UNiagaraComponent;
class ASBCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, CurrentMag, int32, CurrentReserve);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponStateChanged, ESBWeaponState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireModeChanged, ESBFireMode, NewMode);

/**
 * Base weapon actor — data-driven via USBWeaponDataAsset.
 * Handles: fire (hitscan + projectile), reload, recoil, spread, ADS,
 * fire modes, ammo, penetration, damage falloff, headshots.
 * All combat logic is server-authoritative.
 */
UCLASS()
class STORMBREAKER_API ASBWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    ASBWeaponBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Initialization ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Weapon")
    void InitializeWeapon(USBWeaponDataAsset* InData);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Weapon")
    TObjectPtr<USBWeaponDataAsset> WeaponData;

    // --- State ---

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Weapon")
    ESBWeaponState GetWeaponState() const { return WeaponState; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Weapon")
    ESBFireMode GetCurrentFireMode() const { return CurrentFireMode; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Weapon")
    int32 GetCurrentMagazine() const { return CurrentMagazineAmmo; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Weapon")
    int32 GetCurrentReserve() const { return CurrentReserveAmmo; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Weapon")
    bool CanFire() const;

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Weapon")
    bool CanReload() const;

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Weapon")
    float GetCurrentSpread() const;

    // --- Actions ---

    void StartFire();
    void StopFire();
    void Reload();
    void CycleFireMode();
    void Equip(ASBCharacterBase* NewOwner);
    void Unequip();
    void Inspect();

    // Ammo management
    void AddAmmo(int32 Amount);
    void SetAmmo(int32 MagAmmo, int32 ReserveAmmo);

    // Attachments
    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Weapon")
    void ApplyAttachment(const FSBAttachmentModifier& Attachment);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Weapon")
    void RemoveAttachment(const FName& AttachmentName);

    // Owner
    UFUNCTION(BlueprintPure, Category = "StormBreaker|Weapon")
    ASBCharacterBase* GetOwningCharacter() const { return OwningCharacter; }

    // --- Components ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Weapon")
    TObjectPtr<USkeletalMeshComponent> WeaponMeshComp;

    // --- Delegates ---

    UPROPERTY(BlueprintAssignable)
    FOnAmmoChanged OnAmmoChanged;

    UPROPERTY(BlueprintAssignable)
    FOnWeaponStateChanged OnWeaponStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnFireModeChanged OnFireModeChanged;

protected:
    // --- Server RPCs ---

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Fire(FVector Origin, FVector Direction, float ClientTimestamp);

    UFUNCTION(Server, Reliable)
    void Server_Reload();

    UFUNCTION(Server, Reliable)
    void Server_CycleFireMode();

    // --- Multicast RPCs ---

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayFireEffects(FVector MuzzleLocation, FVector Direction);

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayImpactEffects(const TArray<FSBHitResult>& Hits);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayReloadEffects();

    // --- Core Logic ---

    void FireShot();
    void FireHitscan(const FVector& MuzzleLocation, const FVector& AimDirection);
    void FireProjectile(const FVector& MuzzleLocation, const FVector& AimDirection);
    void FireMelee();
    void FireGrenade();
    FVector CalculateSpreadDirection(const FVector& BaseDirection) const;
    float CalculateDamageAtDistance(float Distance) const;
    void ApplyRecoil();
    void RecoverRecoil(float DeltaTime);
    void RecoverSpread(float DeltaTime);
    void ProcessHitscanHit(const FHitResult& Hit, const FVector& ShotDirection, float DamageMultiplier);
    FVector GetMuzzleLocation() const;
    FVector GetAimDirection() const;

    // --- State Management ---

    void SetWeaponState(ESBWeaponState NewState);
    void FinishReload();
    void FinishEquip();
    void FinishUnequip();

    // --- Aim Assist (Mobile) ---

    FVector ApplyAimAssist(const FVector& AimDirection) const;

    // --- Replicated ---

    UPROPERTY(ReplicatedUsing = OnRep_WeaponState)
    ESBWeaponState WeaponState;

    UPROPERTY(Replicated)
    ESBFireMode CurrentFireMode;

    UPROPERTY(ReplicatedUsing = OnRep_Ammo)
    int32 CurrentMagazineAmmo;

    UPROPERTY(Replicated)
    int32 CurrentReserveAmmo;

    UFUNCTION()
    void OnRep_WeaponState();

    UFUNCTION()
    void OnRep_Ammo();

private:
    UPROPERTY()
    TObjectPtr<ASBCharacterBase> OwningCharacter;

    // Active attachments
    TArray<FSBAttachmentModifier> ActiveAttachments;

    // Computed modifiers from attachments
    float AttachDamageMul;
    float AttachRecoilMul;
    float AttachSpreadMul;
    float AttachRangeMul;
    float AttachReloadMul;
    int32 AttachMagBonus;
    void RecalculateAttachmentModifiers();

    // Fire state
    bool bWantsToFire;
    float LastFireTime;
    float CurrentSpreadAccumulator;
    int32 BurstShotsRemaining;
    int32 ShotsFiredInBurst;
    uint8 ShotIndex;

    // Recoil state
    float AccumulatedRecoilPitch;
    float AccumulatedRecoilYaw;
    int32 RecoilPatternIndex;

    // Timers
    FTimerHandle ReloadTimerHandle;
    FTimerHandle EquipTimerHandle;
    FTimerHandle UnequipTimerHandle;
    FTimerHandle BurstTimerHandle;

    // Auto fire timer
    FTimerHandle AutoFireTimerHandle;
    void AutoFireTick();
};
