// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/SBInventoryTypes.h"
#include "SBInventoryComponent.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentChanged, ESBArmorSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoostChanged, float, NewBoostLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsumableUsed, ESBConsumableType, Type, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConsumableCancelled);

/**
 * Manages the player's backpack inventory, equipment (helmet/vest/backpack),
 * consumable usage, boost system, and auto-loot settings.
 * Server-authoritative with client prediction for UI responsiveness.
 */
UCLASS(ClassGroup = "StormBreaker", meta = (BlueprintSpawnableComponent))
class STORMBREAKER_API USBInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USBInventoryComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Item Database ---

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Inventory")
    TObjectPtr<UDataTable> ItemDatabase;

    const FSBItemDefinition* FindItemDefinition(const FName& ItemID) const;

    // --- Inventory Management ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Inventory")
    bool AddItem(const FName& ItemID, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Inventory")
    bool RemoveItem(const FName& ItemID, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Inventory")
    bool HasItem(const FName& ItemID, int32 Count = 1) const;

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Inventory")
    int32 GetItemCount(const FName& ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Inventory")
    void MoveItem(int32 FromSlot, int32 ToSlot);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Inventory")
    void DropItem(const FName& ItemID, int32 Count = 1);

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Inventory")
    const TArray<FSBInventoryItem>& GetItems() const { return Items; }

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Inventory")
    float GetCurrentWeight() const;

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Inventory")
    float GetMaxCapacity() const;

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Inventory")
    bool HasSpaceFor(const FName& ItemID, int32 Count = 1) const;

    // --- Equipment ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Equipment")
    bool EquipArmor(const FName& ItemID);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Equipment")
    void UnequipArmor(ESBArmorSlot Slot);

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Equipment")
    const FSBEquipmentState& GetEquipment() const { return Equipment; }

    float ProcessDamageWithArmor(float IncomingDamage, bool bIsHeadshot);

    // --- Consumables ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Consumable")
    bool UseConsumable(const FName& ItemID);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Consumable")
    void CancelConsumable();

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Consumable")
    bool IsUsingConsumable() const { return bIsUsingConsumable; }

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Consumable")
    float GetConsumableProgress() const;

    // --- Boost ---

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Boost")
    const FSBBoostState& GetBoostState() const { return BoostState; }

    // --- Auto Loot Settings ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|AutoLoot")
    bool bAutoPickupAmmo = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|AutoLoot")
    bool bAutoPickupHealth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|AutoLoot")
    bool bAutoPickupAttachments = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|AutoLoot")
    bool bAutoPickupArmor = true;

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|AutoLoot")
    bool ShouldAutoPickup(const FSBItemDefinition& ItemDef) const;

    // --- Death Crate ---

    TArray<FSBInventoryItem> CollectAllItemsForDeathCrate();

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Inventory")
    float BaseCapacity = 200.0f;

    // --- Delegates ---

    UPROPERTY(BlueprintAssignable)
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(BlueprintAssignable)
    FOnEquipmentChanged OnEquipmentChanged;

    UPROPERTY(BlueprintAssignable)
    FOnBoostChanged OnBoostChanged;

    UPROPERTY(BlueprintAssignable)
    FOnConsumableUsed OnConsumableUsed;

    UPROPERTY(BlueprintAssignable)
    FOnConsumableCancelled OnConsumableCancelled;

protected:
    UFUNCTION(Server, Reliable)
    void Server_AddItem(const FName& ItemID, int32 Count);

    UFUNCTION(Server, Reliable)
    void Server_RemoveItem(const FName& ItemID, int32 Count);

    UFUNCTION(Server, Reliable)
    void Server_UseConsumable(const FName& ItemID);

    UFUNCTION(Server, Reliable)
    void Server_CancelConsumable();

    UFUNCTION(Server, Reliable)
    void Server_EquipArmor(const FName& ItemID);

private:
    void TickBoostAndRegen(float DeltaTime);
    void TickConsumable(float DeltaTime);
    void FinishConsumable();

    UPROPERTY(Replicated)
    TArray<FSBInventoryItem> Items;

    UPROPERTY(Replicated)
    FSBEquipmentState Equipment;

    UPROPERTY(Replicated)
    FSBBoostState BoostState;

    // Consumable use state
    bool bIsUsingConsumable;
    FName ActiveConsumableID;
    float ConsumableElapsed;
    float ConsumableDuration;

    FTimerHandle ConsumableTimerHandle;
};
