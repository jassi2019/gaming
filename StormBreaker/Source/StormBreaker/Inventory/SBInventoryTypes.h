// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SBInventoryTypes.generated.h"

class USBWeaponDataAsset;
class UTexture2D;

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class ESBItemCategory : uint8
{
    Weapon,
    Ammo,
    Attachment,
    Consumable,
    Armor,
    Backpack,
    Throwable
};

UENUM(BlueprintType)
enum class ESBArmorSlot : uint8
{
    Helmet,
    Vest,
    Backpack
};

UENUM(BlueprintType)
enum class ESBArmorLevel : uint8
{
    None = 0,
    Level1 = 1,
    Level2 = 2,
    Level3 = 3
};

UENUM(BlueprintType)
enum class ESBConsumableType : uint8
{
    Bandage,
    FirstAidKit,
    MedKit,
    EnergyDrink,
    Painkiller,
    Adrenaline
};

UENUM(BlueprintType)
enum class ESBAttachmentSlot : uint8
{
    Scope,
    Magazine,
    Grip,
    Stock,
    Muzzle
};

UENUM(BlueprintType)
enum class ESBLootRarity : uint8
{
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary,
    AirDrop
};

UENUM(BlueprintType)
enum class ESBPlayerLifeState : uint8
{
    Alive,
    Knocked,
    Dead,
    Spectating
};

// ============================================================================
// Item Definition
// ============================================================================

USTRUCT(BlueprintType)
struct FSBItemDefinition : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ESBItemCategory Category = ESBItemCategory::Consumable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ESBLootRarity Rarity = ESBLootRarity::Common;

    // If category is Weapon
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Weapon"))
    TObjectPtr<USBWeaponDataAsset> WeaponData;

    // If category is Consumable
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Consumable"))
    ESBConsumableType ConsumableType = ESBConsumableType::Bandage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Consumable"))
    float HealAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Consumable"))
    float BoostAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Consumable"))
    float UseTime = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Consumable"))
    float MaxHealthCap = 75.0f;

    // If category is Armor
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Armor"))
    ESBArmorSlot ArmorSlot = ESBArmorSlot::Vest;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Armor"))
    ESBArmorLevel ArmorLevel = ESBArmorLevel::Level1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Armor"))
    float DamageReduction = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Armor"))
    float Durability = 200.0f;

    // If category is Backpack
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Backpack"))
    float CapacityBonus = 150.0f;

    // If category is Attachment
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Category == ESBItemCategory::Attachment"))
    ESBAttachmentSlot AttachmentSlot = ESBAttachmentSlot::Scope;
};

// ============================================================================
// Inventory Item Instance
// ============================================================================

USTRUCT(BlueprintType)
struct FSBInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StackCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SlotIndex = -1;

    bool IsValid() const { return !ItemID.IsNone() && StackCount > 0; }

    bool operator==(const FSBInventoryItem& Other) const
    {
        return ItemID == Other.ItemID && SlotIndex == Other.SlotIndex;
    }
};

// ============================================================================
// Equipment State
// ============================================================================

USTRUCT(BlueprintType)
struct FSBEquipmentState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FName HelmetItemID;

    UPROPERTY(BlueprintReadOnly)
    float HelmetDurability = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FName VestItemID;

    UPROPERTY(BlueprintReadOnly)
    float VestDurability = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FName BackpackItemID;

    UPROPERTY(BlueprintReadOnly)
    ESBArmorLevel HelmetLevel = ESBArmorLevel::None;

    UPROPERTY(BlueprintReadOnly)
    ESBArmorLevel VestLevel = ESBArmorLevel::None;

    UPROPERTY(BlueprintReadOnly)
    ESBArmorLevel BackpackLevel = ESBArmorLevel::None;

    float GetDamageReduction(ESBArmorSlot Slot) const;
    float AbsorbDamage(float IncomingDamage, ESBArmorSlot Slot);
};

// ============================================================================
// Loot Table Entry
// ============================================================================

USTRUCT(BlueprintType)
struct FSBLootTableEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MinCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ESBLootRarity MinRarity = ESBLootRarity::Common;
};

// ============================================================================
// Boost System
// ============================================================================

USTRUCT(BlueprintType)
struct FSBBoostState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float BoostLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float MaxBoost = 100.0f;

    float GetRegenRate() const
    {
        if (BoostLevel >= 60.0f) return 6.0f;
        if (BoostLevel >= 40.0f) return 4.0f;
        if (BoostLevel >= 20.0f) return 2.0f;
        return 0.0f;
    }

    float GetSpeedBonus() const
    {
        return (BoostLevel >= 60.0f) ? 0.065f : 0.0f;
    }

    void Decay(float DeltaTime)
    {
        BoostLevel = FMath::Max(0.0f, BoostLevel - 3.0f * DeltaTime);
    }
};
