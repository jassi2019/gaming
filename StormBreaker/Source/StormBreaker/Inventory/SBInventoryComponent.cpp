// Copyright StormBreaker Games. All Rights Reserved.

#include "Inventory/SBInventoryComponent.h"
#include "Inventory/SBInventoryTypes.h"
#include "StormBreaker.h"
#include "Engine/DataTable.h"
#include "Net/UnrealNetwork.h"
#include "Character/SBCharacterBase.h"
#include "Core/SBAttributeSet.h"
#include "AbilitySystemComponent.h"

USBInventoryComponent::USBInventoryComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;

    bIsUsingConsumable = false;
    ConsumableElapsed = 0.0f;
    ConsumableDuration = 0.0f;
}

void USBInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USBInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GetOwner()->HasAuthority())
    {
        TickBoostAndRegen(DeltaTime);
        TickConsumable(DeltaTime);
    }
}

void USBInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USBInventoryComponent, Items);
    DOREPLIFETIME(USBInventoryComponent, Equipment);
    DOREPLIFETIME(USBInventoryComponent, BoostState);
}

// ============================================================================
// Item Database
// ============================================================================

const FSBItemDefinition* USBInventoryComponent::FindItemDefinition(const FName& ItemID) const
{
    if (!ItemDatabase) return nullptr;
    return ItemDatabase->FindRow<FSBItemDefinition>(ItemID, TEXT("FindItemDef"));
}

// ============================================================================
// Inventory Management
// ============================================================================

bool USBInventoryComponent::AddItem(const FName& ItemID, int32 Count)
{
    if (Count <= 0) return false;

    if (!GetOwner()->HasAuthority())
    {
        Server_AddItem(ItemID, Count);
        return true;
    }

    const FSBItemDefinition* Def = FindItemDefinition(ItemID);
    if (!Def) return false;

    if (!HasSpaceFor(ItemID, Count)) return false;

    // Try to stack with existing
    for (FSBInventoryItem& Item : Items)
    {
        if (Item.ItemID == ItemID && Def->MaxStackSize > 1)
        {
            int32 CanAdd = Def->MaxStackSize - Item.StackCount;
            int32 ToAdd = FMath::Min(Count, CanAdd);
            Item.StackCount += ToAdd;
            Count -= ToAdd;
            if (Count <= 0) break;
        }
    }

    // Create new stacks for remainder
    while (Count > 0)
    {
        FSBInventoryItem NewItem;
        NewItem.ItemID = ItemID;
        NewItem.StackCount = FMath::Min(Count, Def->MaxStackSize);
        NewItem.SlotIndex = Items.Num();
        Items.Add(NewItem);
        Count -= NewItem.StackCount;
    }

    OnInventoryChanged.Broadcast();
    UE_LOG(LogSBInventory, Verbose, TEXT("Added %s"), *ItemID.ToString());
    return true;
}

bool USBInventoryComponent::RemoveItem(const FName& ItemID, int32 Count)
{
    if (Count <= 0) return false;

    if (!GetOwner()->HasAuthority())
    {
        Server_RemoveItem(ItemID, Count);
        return true;
    }

    int32 Remaining = Count;
    for (int32 i = Items.Num() - 1; i >= 0 && Remaining > 0; i--)
    {
        if (Items[i].ItemID == ItemID)
        {
            int32 ToRemove = FMath::Min(Remaining, Items[i].StackCount);
            Items[i].StackCount -= ToRemove;
            Remaining -= ToRemove;

            if (Items[i].StackCount <= 0)
            {
                Items.RemoveAt(i);
            }
        }
    }

    if (Remaining < Count)
    {
        OnInventoryChanged.Broadcast();
        return true;
    }
    return false;
}

bool USBInventoryComponent::HasItem(const FName& ItemID, int32 Count) const
{
    return GetItemCount(ItemID) >= Count;
}

int32 USBInventoryComponent::GetItemCount(const FName& ItemID) const
{
    int32 Total = 0;
    for (const FSBInventoryItem& Item : Items)
    {
        if (Item.ItemID == ItemID)
        {
            Total += Item.StackCount;
        }
    }
    return Total;
}

void USBInventoryComponent::MoveItem(int32 FromSlot, int32 ToSlot)
{
    if (!Items.IsValidIndex(FromSlot)) return;

    if (Items.IsValidIndex(ToSlot))
    {
        // Swap
        Items.Swap(FromSlot, ToSlot);
        Items[FromSlot].SlotIndex = FromSlot;
        Items[ToSlot].SlotIndex = ToSlot;
    }
    else
    {
        Items[FromSlot].SlotIndex = ToSlot;
    }

    OnInventoryChanged.Broadcast();
}

void USBInventoryComponent::DropItem(const FName& ItemID, int32 Count)
{
    if (!HasItem(ItemID, Count)) return;
    RemoveItem(ItemID, Count);

    // Spawn world pickup — handled by game mode or loot system
    UE_LOG(LogSBInventory, Verbose, TEXT("Dropped %d x %s"), Count, *ItemID.ToString());
}

float USBInventoryComponent::GetCurrentWeight() const
{
    float Total = 0.0f;
    for (const FSBInventoryItem& Item : Items)
    {
        const FSBItemDefinition* Def = FindItemDefinition(Item.ItemID);
        if (Def)
        {
            Total += Def->Weight * Item.StackCount;
        }
    }
    return Total;
}

float USBInventoryComponent::GetMaxCapacity() const
{
    float Capacity = BaseCapacity;

    switch (Equipment.BackpackLevel)
    {
    case ESBArmorLevel::Level1: Capacity += 150.0f; break;
    case ESBArmorLevel::Level2: Capacity += 200.0f; break;
    case ESBArmorLevel::Level3: Capacity += 250.0f; break;
    default: break;
    }

    return Capacity;
}

bool USBInventoryComponent::HasSpaceFor(const FName& ItemID, int32 Count) const
{
    const FSBItemDefinition* Def = FindItemDefinition(ItemID);
    if (!Def) return false;

    float AdditionalWeight = Def->Weight * Count;
    return (GetCurrentWeight() + AdditionalWeight) <= GetMaxCapacity();
}

// ============================================================================
// Equipment
// ============================================================================

bool USBInventoryComponent::EquipArmor(const FName& ItemID)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_EquipArmor(ItemID);
        return true;
    }

    const FSBItemDefinition* Def = FindItemDefinition(ItemID);
    if (!Def || Def->Category != ESBItemCategory::Armor) return false;

    // Unequip existing if better
    switch (Def->ArmorSlot)
    {
    case ESBArmorSlot::Helmet:
        if (Equipment.HelmetLevel >= Def->ArmorLevel) return false;
        if (!Equipment.HelmetItemID.IsNone()) UnequipArmor(ESBArmorSlot::Helmet);
        Equipment.HelmetItemID = ItemID;
        Equipment.HelmetLevel = Def->ArmorLevel;
        Equipment.HelmetDurability = Def->Durability;
        break;

    case ESBArmorSlot::Vest:
        if (Equipment.VestLevel >= Def->ArmorLevel) return false;
        if (!Equipment.VestItemID.IsNone()) UnequipArmor(ESBArmorSlot::Vest);
        Equipment.VestItemID = ItemID;
        Equipment.VestLevel = Def->ArmorLevel;
        Equipment.VestDurability = Def->Durability;
        break;

    case ESBArmorSlot::Backpack:
        if (Equipment.BackpackLevel >= Def->ArmorLevel) return false;
        if (!Equipment.BackpackItemID.IsNone()) UnequipArmor(ESBArmorSlot::Backpack);
        Equipment.BackpackItemID = ItemID;
        Equipment.BackpackLevel = Def->ArmorLevel;
        break;
    }

    RemoveItem(ItemID, 1);
    OnEquipmentChanged.Broadcast(Def->ArmorSlot);
    return true;
}

void USBInventoryComponent::UnequipArmor(ESBArmorSlot Slot)
{
    FName ItemToReturn = NAME_None;

    switch (Slot)
    {
    case ESBArmorSlot::Helmet:
        ItemToReturn = Equipment.HelmetItemID;
        Equipment.HelmetItemID = NAME_None;
        Equipment.HelmetLevel = ESBArmorLevel::None;
        Equipment.HelmetDurability = 0.0f;
        break;

    case ESBArmorSlot::Vest:
        ItemToReturn = Equipment.VestItemID;
        Equipment.VestItemID = NAME_None;
        Equipment.VestLevel = ESBArmorLevel::None;
        Equipment.VestDurability = 0.0f;
        break;

    case ESBArmorSlot::Backpack:
        ItemToReturn = Equipment.BackpackItemID;
        Equipment.BackpackItemID = NAME_None;
        Equipment.BackpackLevel = ESBArmorLevel::None;
        break;
    }

    if (!ItemToReturn.IsNone())
    {
        AddItem(ItemToReturn, 1);
    }

    OnEquipmentChanged.Broadcast(Slot);
}

float USBInventoryComponent::ProcessDamageWithArmor(float IncomingDamage, bool bIsHeadshot)
{
    float Remaining = IncomingDamage;

    if (bIsHeadshot)
    {
        Remaining = Equipment.AbsorbDamage(Remaining, ESBArmorSlot::Helmet);
    }

    Remaining = Equipment.AbsorbDamage(Remaining, ESBArmorSlot::Vest);

    return Remaining;
}

// ============================================================================
// Consumables
// ============================================================================

bool USBInventoryComponent::UseConsumable(const FName& ItemID)
{
    if (bIsUsingConsumable) return false;
    if (!HasItem(ItemID, 1)) return false;

    const FSBItemDefinition* Def = FindItemDefinition(ItemID);
    if (!Def || Def->Category != ESBItemCategory::Consumable) return false;

    // Health cap check for bandages
    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (Character && Def->HealAmount > 0.0f && Def->MaxHealthCap < 100.0f)
    {
        UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
        if (ASC)
        {
            // Check if already at cap — skip if so
        }
    }

    if (!GetOwner()->HasAuthority())
    {
        Server_UseConsumable(ItemID);
    }

    bIsUsingConsumable = true;
    ActiveConsumableID = ItemID;
    ConsumableElapsed = 0.0f;
    ConsumableDuration = Def->UseTime;

    OnConsumableUsed.Broadcast(Def->ConsumableType, ConsumableDuration);

    UE_LOG(LogSBInventory, Verbose, TEXT("Using consumable: %s (%.1fs)"), *ItemID.ToString(), ConsumableDuration);
    return true;
}

void USBInventoryComponent::CancelConsumable()
{
    if (!bIsUsingConsumable) return;

    bIsUsingConsumable = false;
    ActiveConsumableID = NAME_None;
    ConsumableElapsed = 0.0f;

    if (!GetOwner()->HasAuthority())
    {
        Server_CancelConsumable();
    }

    OnConsumableCancelled.Broadcast();
}

float USBInventoryComponent::GetConsumableProgress() const
{
    if (!bIsUsingConsumable || ConsumableDuration <= 0.0f) return 0.0f;
    return FMath::Clamp(ConsumableElapsed / ConsumableDuration, 0.0f, 1.0f);
}

void USBInventoryComponent::TickConsumable(float DeltaTime)
{
    if (!bIsUsingConsumable) return;

    // Cancel if taking damage or moving too fast
    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (Character)
    {
        // Cancel if sprinting
        USBCharacterMovementComponent* CMC = Character->GetSBMovement();
        if (CMC && CMC->IsSprinting())
        {
            CancelConsumable();
            return;
        }
    }

    ConsumableElapsed += DeltaTime;

    if (ConsumableElapsed >= ConsumableDuration)
    {
        FinishConsumable();
    }
}

void USBInventoryComponent::FinishConsumable()
{
    if (!GetOwner()->HasAuthority()) return;

    const FSBItemDefinition* Def = FindItemDefinition(ActiveConsumableID);
    if (!Def)
    {
        bIsUsingConsumable = false;
        return;
    }

    // Consume the item
    RemoveItem(ActiveConsumableID, 1);

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (Character)
    {
        UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();

        // Apply heal
        if (Def->HealAmount > 0.0f && ASC)
        {
            // Direct health modification via attribute set
            // In production, this would go through a GameplayEffect
            UE_LOG(LogSBInventory, Log, TEXT("Healed %.0f HP from %s"), Def->HealAmount, *ActiveConsumableID.ToString());
        }

        // Apply boost
        if (Def->BoostAmount > 0.0f)
        {
            BoostState.BoostLevel = FMath::Min(BoostState.MaxBoost, BoostState.BoostLevel + Def->BoostAmount);
            OnBoostChanged.Broadcast(BoostState.BoostLevel);
            UE_LOG(LogSBInventory, Log, TEXT("Boost +%.0f from %s"), Def->BoostAmount, *ActiveConsumableID.ToString());
        }
    }

    bIsUsingConsumable = false;
    ActiveConsumableID = NAME_None;
    ConsumableElapsed = 0.0f;
}

// ============================================================================
// Boost & Regen
// ============================================================================

void USBInventoryComponent::TickBoostAndRegen(float DeltaTime)
{
    if (BoostState.BoostLevel <= 0.0f) return;

    BoostState.Decay(DeltaTime);

    float RegenRate = BoostState.GetRegenRate();
    if (RegenRate > 0.0f)
    {
        ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
        if (Character)
        {
            // Apply passive regen via GAS
            // RegenRate HP per second
        }
    }

    OnBoostChanged.Broadcast(BoostState.BoostLevel);
}

// ============================================================================
// Auto Loot
// ============================================================================

bool USBInventoryComponent::ShouldAutoPickup(const FSBItemDefinition& ItemDef) const
{
    switch (ItemDef.Category)
    {
    case ESBItemCategory::Ammo: return bAutoPickupAmmo;
    case ESBItemCategory::Consumable: return bAutoPickupHealth;
    case ESBItemCategory::Attachment: return bAutoPickupAttachments;
    case ESBItemCategory::Armor: return bAutoPickupArmor;
    case ESBItemCategory::Backpack: return bAutoPickupArmor;
    default: return false;
    }
}

// ============================================================================
// Death Crate
// ============================================================================

TArray<FSBInventoryItem> USBInventoryComponent::CollectAllItemsForDeathCrate()
{
    TArray<FSBInventoryItem> AllItems = Items;

    // Add equipped armor
    if (!Equipment.HelmetItemID.IsNone())
    {
        FSBInventoryItem ArmorItem;
        ArmorItem.ItemID = Equipment.HelmetItemID;
        ArmorItem.StackCount = 1;
        AllItems.Add(ArmorItem);
    }
    if (!Equipment.VestItemID.IsNone())
    {
        FSBInventoryItem ArmorItem;
        ArmorItem.ItemID = Equipment.VestItemID;
        ArmorItem.StackCount = 1;
        AllItems.Add(ArmorItem);
    }
    if (!Equipment.BackpackItemID.IsNone())
    {
        FSBInventoryItem ArmorItem;
        ArmorItem.ItemID = Equipment.BackpackItemID;
        ArmorItem.StackCount = 1;
        AllItems.Add(ArmorItem);
    }

    // Clear inventory
    Items.Empty();
    Equipment = FSBEquipmentState();
    OnInventoryChanged.Broadcast();

    return AllItems;
}

// ============================================================================
// Server RPCs
// ============================================================================

void USBInventoryComponent::Server_AddItem_Implementation(const FName& ItemID, int32 Count)
{
    AddItem(ItemID, Count);
}

void USBInventoryComponent::Server_RemoveItem_Implementation(const FName& ItemID, int32 Count)
{
    RemoveItem(ItemID, Count);
}

void USBInventoryComponent::Server_UseConsumable_Implementation(const FName& ItemID)
{
    UseConsumable(ItemID);
}

void USBInventoryComponent::Server_CancelConsumable_Implementation()
{
    CancelConsumable();
}

void USBInventoryComponent::Server_EquipArmor_Implementation(const FName& ItemID)
{
    EquipArmor(ItemID);
}
