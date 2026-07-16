// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapon/SBWeaponTypes.h"
#include "SBWeaponDataAsset.generated.h"

class UAnimMontage;
class USoundBase;
class UNiagaraSystem;
class UStaticMesh;
class USkeletalMesh;

/**
 * Data asset defining all properties of a weapon.
 * Each weapon type has one of these assigned — no subclassing needed.
 * Create via Content Browser → Miscellaneous → Data Asset → SBWeaponDataAsset.
 */
UCLASS(BlueprintType)
class STORMBREAKER_API USBWeaponDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // --- Identity ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FName WeaponID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    ESBWeaponType WeaponType = ESBWeaponType::AssaultRifle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    ESBWeaponSlot DefaultSlot = ESBWeaponSlot::Primary;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    ESBAmmoType AmmoType = ESBAmmoType::Rifle_556;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    UTexture2D* Icon = nullptr;

    // --- Combat ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    ESBDamageType DamageMethod = ESBDamageType::Hitscan;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    TArray<ESBFireMode> AvailableFireModes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    ESBFireMode DefaultFireMode = ESBFireMode::Auto;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    int32 BurstCount = 3;

    // Rounds per minute
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float FireRate = 600.0f;

    // Pellets per shot (shotgun = 8, others = 1)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    int32 PelletsPerShot = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    FSBDamageData Damage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    FSBRecoilData Recoil;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    FSBSpreadData Spread;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    FSBProjectileData Projectile;

    // --- Magazine ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magazine")
    int32 MagazineSize = 30;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magazine")
    int32 MaxReserveAmmo = 120;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magazine")
    float ReloadTime = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magazine")
    float ReloadTimeEmpty = 2.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magazine")
    bool bCanReloadWhileADS = false;

    // --- ADS ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ADS")
    float ADSSpeed = 0.2f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ADS")
    float ADSFOVMultiplier = 0.75f;

    // --- Timing ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
    float EquipTime = 0.6f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
    float UnequipTime = 0.4f;

    // --- Melee ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee", meta = (EditCondition = "WeaponType == ESBWeaponType::Melee"))
    float MeleeRange = 200.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee", meta = (EditCondition = "WeaponType == ESBWeaponType::Melee"))
    float MeleeAngle = 60.0f;

    // --- Grenade ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grenade", meta = (EditCondition = "WeaponType == ESBWeaponType::Grenade"))
    float ThrowForce = 2000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grenade", meta = (EditCondition = "WeaponType == ESBWeaponType::Grenade"))
    float FuseTime = 4.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grenade", meta = (EditCondition = "WeaponType == ESBWeaponType::Grenade"))
    float ExplosionRadius = 500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grenade", meta = (EditCondition = "WeaponType == ESBWeaponType::Grenade"))
    float ExplosionDamage = 150.0f;

    // --- Visuals ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    TObjectPtr<USkeletalMesh> WeaponMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    FName MuzzleSocketName = FName("Muzzle");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    FName EjectionSocketName = FName("ShellEject");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    TObjectPtr<UNiagaraSystem> MuzzleFlashVFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    TObjectPtr<UNiagaraSystem> BulletTrailVFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    TObjectPtr<UNiagaraSystem> ShellEjectVFX;

    // --- Audio ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> FireSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> FireSoundSilenced;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> DryFireSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> ReloadSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> EquipSound;

    // --- Animations ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    TObjectPtr<UAnimMontage> FireMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    TObjectPtr<UAnimMontage> ReloadMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    TObjectPtr<UAnimMontage> ReloadEmptyMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    TObjectPtr<UAnimMontage> EquipMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    TObjectPtr<UAnimMontage> UnequipMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    TObjectPtr<UAnimMontage> InspectMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    TObjectPtr<UAnimMontage> MeleeMontage;

    // --- Attachments ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attachments")
    TArray<FName> AllowedAttachmentSlots;

    // --- Aim Assist (Mobile) ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mobile")
    float AimAssistStrength = 0.3f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mobile")
    float AimAssistRange = 3000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mobile")
    float AimAssistAngle = 15.0f;

    // --- Helpers ---

    float GetTimeBetweenShots() const
    {
        return (FireRate > 0.0f) ? (60.0f / FireRate) : 0.1f;
    }

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("Weapon", WeaponID);
    }
};
