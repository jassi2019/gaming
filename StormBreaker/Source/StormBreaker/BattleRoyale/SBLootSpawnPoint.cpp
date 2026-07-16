// Copyright StormBreaker Games. All Rights Reserved.

#include "BattleRoyale/SBLootSpawnPoint.h"
#include "Inventory/SBLootManager.h"
#include "Weapon/SBWeaponPickup.h"
#include "Weapon/SBWeaponDataAsset.h"
#include "StormBreaker.h"

ASBLootSpawnPoint::ASBLootSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

#if WITH_EDITORONLY_DATA
    bIsSpatiallyLoaded = false;
#endif
}

void ASBLootSpawnPoint::SpawnLoot()
{
    if (bHasSpawned || !HasAuthority()) return;
    bHasSpawned = true;

    USBLootManager* LootMgr = GetWorld()->GetSubsystem<USBLootManager>();
    if (!LootMgr) return;

    int32 Count = FMath::RandRange(MinItems, MaxItems);
    UDataTable* Table = LootTable ? LootTable : LootMgr->DefaultLootTable;

    TArray<FSBLootDrop> Drops = LootMgr->GenerateLootFromTable(Table, Count);

    FVector SpawnBase = GetActorLocation();
    for (int32 i = 0; i < Drops.Num(); i++)
    {
        FVector Offset(FMath::FRandRange(-50.0f, 50.0f), FMath::FRandRange(-50.0f, 50.0f), 20.0f);
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ASBWeaponPickup* Pickup = GetWorld()->SpawnActor<ASBWeaponPickup>(
            ASBWeaponPickup::StaticClass(), SpawnBase + Offset, FRotator::ZeroRotator, Params);

        if (Pickup)
        {
            // Pickup initialization handled by data table lookup in full implementation
            UE_LOG(LogSBInventory, Verbose, TEXT("Loot spawned: %s at %s"), *Drops[i].ItemID.ToString(), *SpawnBase.ToString());
        }
    }
}
