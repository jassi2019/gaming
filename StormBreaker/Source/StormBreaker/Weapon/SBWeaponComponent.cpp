// Copyright StormBreaker Games. All Rights Reserved.

#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponBase.h"
#include "Weapon/SBWeaponDataAsset.h"
#include "Weapon/SBWeaponPickup.h"
#include "Character/SBCharacterBase.h"
#include "StormBreaker.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

USBWeaponComponent::USBWeaponComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = false;

    ActiveSlot = ESBWeaponSlot::Primary;
    WeaponSlots.SetNum(static_cast<int32>(ESBWeaponSlot::MAX));
}

void USBWeaponComponent::BeginPlay()
{
    Super::BeginPlay();
    WeaponSlots.SetNum(static_cast<int32>(ESBWeaponSlot::MAX));
}

void USBWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USBWeaponComponent, WeaponSlots);
    DOREPLIFETIME(USBWeaponComponent, ActiveWeapon);
    DOREPLIFETIME(USBWeaponComponent, ActiveSlot);
}

// ============================================================================
// Weapon Management
// ============================================================================

bool USBWeaponComponent::AddWeapon(USBWeaponDataAsset* WeaponData, ESBWeaponSlot Slot)
{
    return AddWeaponToSlot(WeaponData, static_cast<int32>(Slot));
}

bool USBWeaponComponent::AddWeaponToSlot(USBWeaponDataAsset* WeaponData, int32 SlotIndex)
{
    if (!WeaponData || SlotIndex < 0 || SlotIndex >= static_cast<int32>(ESBWeaponSlot::MAX))
    {
        return false;
    }

    ESBWeaponSlot Slot = static_cast<ESBWeaponSlot>(SlotIndex);

    if (!GetOwner()->HasAuthority())
    {
        Server_AddWeapon(WeaponData, Slot);
        return true;
    }

    // Drop existing weapon in slot if any
    if (WeaponSlots[SlotIndex] != nullptr)
    {
        RemoveWeapon(Slot);
    }

    ASBWeaponBase* NewWeapon = SpawnWeapon(WeaponData);
    if (!NewWeapon) return false;

    WeaponSlots[SlotIndex] = NewWeapon;
    OnWeaponEquipped.Broadcast(NewWeapon, Slot);

    // If no active weapon, equip this one
    if (!ActiveWeapon)
    {
        SwitchToSlot(Slot);
    }
    else
    {
        AttachWeaponToBack(NewWeapon);
    }

    UE_LOG(LogSBWeapon, Log, TEXT("Added weapon '%s' to slot %d"),
        *WeaponData->DisplayName.ToString(), SlotIndex);

    return true;
}

void USBWeaponComponent::RemoveWeapon(ESBWeaponSlot Slot)
{
    int32 Index = static_cast<int32>(Slot);
    if (Index < 0 || Index >= WeaponSlots.Num() || !WeaponSlots[Index]) return;

    ASBWeaponBase* Weapon = WeaponSlots[Index];
    Weapon->Unequip();
    OnWeaponUnequipped.Broadcast(Weapon, Slot);

    if (ActiveWeapon == Weapon)
    {
        ActiveWeapon = nullptr;
        OnActiveWeaponChanged.Broadcast(nullptr);
    }

    WeaponSlots[Index] = nullptr;
    DetachWeapon(Weapon);
    Weapon->Destroy();
}

void USBWeaponComponent::DropCurrentWeapon()
{
    if (!ActiveWeapon) return;

    if (!GetOwner()->HasAuthority())
    {
        Server_DropWeapon(ActiveSlot);
        return;
    }

    ASBCharacterBase* Character = GetOwnerCharacter();
    if (!Character) return;

    // Spawn pickup at character's location
    FVector DropLocation = Character->GetActorLocation() + Character->GetActorForwardVector() * 100.0f;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ASBWeaponPickup* Pickup = GetWorld()->SpawnActor<ASBWeaponPickup>(
        ASBWeaponPickup::StaticClass(), DropLocation, FRotator::ZeroRotator, SpawnParams);

    if (Pickup && ActiveWeapon->WeaponData)
    {
        Pickup->InitPickup(ActiveWeapon->WeaponData,
            ActiveWeapon->GetCurrentMagazine(), ActiveWeapon->GetCurrentReserve());
    }

    RemoveWeapon(ActiveSlot);

    // Switch to next available weapon
    SwitchToNextWeapon();
}

// ============================================================================
// Switching
// ============================================================================

void USBWeaponComponent::SwitchToSlot(ESBWeaponSlot Slot)
{
    int32 Index = static_cast<int32>(Slot);
    if (Index < 0 || Index >= WeaponSlots.Num()) return;
    if (!WeaponSlots[Index]) return;
    if (ActiveWeapon == WeaponSlots[Index]) return;

    if (!GetOwner()->HasAuthority())
    {
        Server_SwitchToSlot(Slot);
    }

    // Unequip current weapon
    if (ActiveWeapon)
    {
        ActiveWeapon->Unequip();
        AttachWeaponToBack(ActiveWeapon);
    }

    // Equip new weapon
    ActiveWeapon = WeaponSlots[Index];
    ActiveSlot = Slot;
    AttachWeaponToHand(ActiveWeapon);
    ActiveWeapon->Equip(GetOwnerCharacter());

    OnActiveWeaponChanged.Broadcast(ActiveWeapon);
}

void USBWeaponComponent::SwitchToNextWeapon()
{
    int32 Current = static_cast<int32>(ActiveSlot);
    int32 Max = static_cast<int32>(ESBWeaponSlot::MAX);

    for (int32 i = 1; i < Max; i++)
    {
        int32 NextIndex = (Current + i) % Max;
        if (WeaponSlots.IsValidIndex(NextIndex) && WeaponSlots[NextIndex])
        {
            SwitchToSlot(static_cast<ESBWeaponSlot>(NextIndex));
            return;
        }
    }
}

void USBWeaponComponent::SwitchToPreviousWeapon()
{
    int32 Current = static_cast<int32>(ActiveSlot);
    int32 Max = static_cast<int32>(ESBWeaponSlot::MAX);

    for (int32 i = 1; i < Max; i++)
    {
        int32 PrevIndex = (Current - i + Max) % Max;
        if (WeaponSlots.IsValidIndex(PrevIndex) && WeaponSlots[PrevIndex])
        {
            SwitchToSlot(static_cast<ESBWeaponSlot>(PrevIndex));
            return;
        }
    }
}

// ============================================================================
// Action Forwarding
// ============================================================================

void USBWeaponComponent::StartFire()
{
    if (ActiveWeapon) ActiveWeapon->StartFire();
}

void USBWeaponComponent::StopFire()
{
    if (ActiveWeapon) ActiveWeapon->StopFire();
}

void USBWeaponComponent::Reload()
{
    if (ActiveWeapon) ActiveWeapon->Reload();
}

void USBWeaponComponent::CycleFireMode()
{
    if (ActiveWeapon) ActiveWeapon->CycleFireMode();
}

void USBWeaponComponent::Inspect()
{
    if (ActiveWeapon) ActiveWeapon->Inspect();
}

// ============================================================================
// Queries
// ============================================================================

ASBWeaponBase* USBWeaponComponent::GetWeaponInSlot(ESBWeaponSlot Slot) const
{
    int32 Index = static_cast<int32>(Slot);
    if (Index >= 0 && Index < WeaponSlots.Num())
    {
        return WeaponSlots[Index];
    }
    return nullptr;
}

bool USBWeaponComponent::HasWeaponInSlot(ESBWeaponSlot Slot) const
{
    return GetWeaponInSlot(Slot) != nullptr;
}

int32 USBWeaponComponent::GetAmmoForType(ESBAmmoType AmmoType) const
{
    const int32* Found = AmmoPool.Find(AmmoType);
    return Found ? *Found : 0;
}

void USBWeaponComponent::AddAmmoForType(ESBAmmoType AmmoType, int32 Amount)
{
    int32& Current = AmmoPool.FindOrAdd(AmmoType);
    Current += Amount;
}

// ============================================================================
// Server RPCs
// ============================================================================

void USBWeaponComponent::Server_AddWeapon_Implementation(USBWeaponDataAsset* WeaponData, ESBWeaponSlot Slot)
{
    AddWeaponToSlot(WeaponData, static_cast<int32>(Slot));
}

void USBWeaponComponent::Server_SwitchToSlot_Implementation(ESBWeaponSlot Slot)
{
    SwitchToSlot(Slot);
}

void USBWeaponComponent::Server_DropWeapon_Implementation(ESBWeaponSlot Slot)
{
    if (ActiveSlot == Slot)
    {
        DropCurrentWeapon();
    }
}

// ============================================================================
// Internal
// ============================================================================

ASBWeaponBase* USBWeaponComponent::SpawnWeapon(USBWeaponDataAsset* Data)
{
    UWorld* World = GetWorld();
    if (!World || !Data) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.Instigator = Cast<APawn>(GetOwner());
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ASBWeaponBase* Weapon = World->SpawnActor<ASBWeaponBase>(
        ASBWeaponBase::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

    if (Weapon)
    {
        Weapon->InitializeWeapon(Data);
    }

    return Weapon;
}

void USBWeaponComponent::AttachWeaponToHand(ASBWeaponBase* Weapon)
{
    if (!Weapon) return;

    ASBCharacterBase* Character = GetOwnerCharacter();
    if (!Character || !Character->GetMesh()) return;

    Weapon->AttachToComponent(Character->GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);
    Weapon->SetActorHiddenInGame(false);
}

void USBWeaponComponent::AttachWeaponToBack(ASBWeaponBase* Weapon)
{
    if (!Weapon) return;

    ASBCharacterBase* Character = GetOwnerCharacter();
    if (!Character || !Character->GetMesh()) return;

    Weapon->AttachToComponent(Character->GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale, BackpackSocket);
}

void USBWeaponComponent::DetachWeapon(ASBWeaponBase* Weapon)
{
    if (!Weapon) return;
    Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

ASBCharacterBase* USBWeaponComponent::GetOwnerCharacter() const
{
    return Cast<ASBCharacterBase>(GetOwner());
}

void USBWeaponComponent::OnRep_ActiveWeapon()
{
    OnActiveWeaponChanged.Broadcast(ActiveWeapon);
}
