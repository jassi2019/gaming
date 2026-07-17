// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon/SBWeaponTypes.h"
#include "SBWeaponComponent.generated.h"

class ASBWeaponBase;
class USBWeaponDataAsset;
class ASBCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponEquipped, ASBWeaponBase*, Weapon, ESBWeaponSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponUnequipped, ASBWeaponBase*, Weapon, ESBWeaponSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveWeaponChanged, ASBWeaponBase*, NewWeapon);

/**
 * Component managing the weapon inventory on a character.
 * Handles weapon slots, switching, pickup, drop, and input routing.
 * Attach to ASBCharacterBase.
 */
UCLASS(ClassGroup = "StormBreaker", meta = (BlueprintSpawnableComponent))
class STORMBREAKER_API USBWeaponComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USBWeaponComponent();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Weapon Management ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    bool AddWeapon(USBWeaponDataAsset* WeaponData, ESBWeaponSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    bool AddWeaponToSlot(USBWeaponDataAsset* WeaponData, int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void RemoveWeapon(ESBWeaponSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void DropCurrentWeapon();

    // --- Switching ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void SwitchToSlot(ESBWeaponSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void SwitchToNextWeapon();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void SwitchToPreviousWeapon();

    // --- Actions (forwarded to active weapon) ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void StartFire();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void StopFire();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void Reload();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void CycleFireMode();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void Inspect();

    // --- Queries ---

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Weapon")
    ASBWeaponBase* GetActiveWeapon() const { return ActiveWeapon; }

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Weapon")
    ASBWeaponBase* GetWeaponInSlot(ESBWeaponSlot Slot) const;

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Weapon")
    ESBWeaponSlot GetActiveSlot() const { return ActiveSlot; }

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Weapon")
    bool HasWeaponInSlot(ESBWeaponSlot Slot) const;

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Weapon")
    int32 GetAmmoForType(ESBAmmoType AmmoType) const;

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Weapon")
    void AddAmmoForType(ESBAmmoType AmmoType, int32 Amount);

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Weapon")
    FName WeaponAttachSocket = FName("weapon_r");

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Weapon")
    FName BackpackSocket = FName("spine_03");

    // --- Delegates ---

    UPROPERTY(BlueprintAssignable)
    FOnWeaponEquipped OnWeaponEquipped;

    UPROPERTY(BlueprintAssignable)
    FOnWeaponUnequipped OnWeaponUnequipped;

    UPROPERTY(BlueprintAssignable)
    FOnActiveWeaponChanged OnActiveWeaponChanged;

protected:
    UFUNCTION(Server, Reliable)
    void Server_AddWeapon(USBWeaponDataAsset* WeaponData, ESBWeaponSlot Slot);

    UFUNCTION(Server, Reliable)
    void Server_SwitchToSlot(ESBWeaponSlot Slot);

    UFUNCTION(Server, Reliable)
    void Server_DropWeapon(ESBWeaponSlot Slot);

private:
    ASBWeaponBase* SpawnWeapon(USBWeaponDataAsset* Data);
    void AttachWeaponToHand(ASBWeaponBase* Weapon);
    void AttachWeaponToBack(ASBWeaponBase* Weapon);
    void DetachWeapon(ASBWeaponBase* Weapon);
    ASBCharacterBase* GetOwnerCharacter() const;

    UPROPERTY(Replicated)
    TArray<TObjectPtr<ASBWeaponBase>> WeaponSlots;

    UPROPERTY(ReplicatedUsing = OnRep_ActiveWeapon)
    TObjectPtr<ASBWeaponBase> ActiveWeapon;

    UPROPERTY(Replicated)
    ESBWeaponSlot ActiveSlot;

    UFUNCTION()
    void OnRep_ActiveWeapon();

    // Ammo pool (shared reserve ammo per type)
    TMap<ESBAmmoType, int32> AmmoPool;
};
