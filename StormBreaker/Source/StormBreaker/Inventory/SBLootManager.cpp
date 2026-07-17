// Copyright Island Of Death Games. All Rights Reserved.

#include "Inventory/SBLootManager.h"
#include "StormBreaker.h"
#include "Engine/DataTable.h"

void USBLootManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogSBInventory, Log, TEXT("Loot Manager initialized."));
}

TArray<FSBLootDrop> USBLootManager::GenerateLootFromTable(UDataTable* LootTable, int32 NumItems) const
{
    TArray<FSBLootDrop> Result;
    if (!LootTable) return Result;

    TArray<FSBLootTableEntry*> Entries;
    LootTable->GetAllRows<FSBLootTableEntry>(TEXT("GenerateLoot"), Entries);

    if (Entries.Num() == 0) return Result;

    for (int32 i = 0; i < NumItems; i++)
    {
        FSBLootDrop Drop = RollFromEntries(Entries);
        if (!Drop.ItemID.IsNone())
        {
            // Merge with existing
            bool bMerged = false;
            for (FSBLootDrop& Existing : Result)
            {
                if (Existing.ItemID == Drop.ItemID)
                {
                    Existing.Count += Drop.Count;
                    bMerged = true;
                    break;
                }
            }
            if (!bMerged)
            {
                Result.Add(Drop);
            }
        }
    }

    return Result;
}

TArray<FSBLootDrop> USBLootManager::GenerateAirDropLoot() const
{
    UDataTable* Table = AirDropLootTable ? AirDropLootTable : DefaultLootTable;
    return GenerateLootFromTable(Table, AirDropItemCount);
}

TArray<FSBLootDrop> USBLootManager::GenerateBuildingLoot() const
{
    int32 NumItems = FMath::RandRange(MinBuildingLoot, MaxBuildingLoot);
    return GenerateLootFromTable(DefaultLootTable, NumItems);
}

FSBLootDrop USBLootManager::RollSingleItem(UDataTable* LootTable) const
{
    if (!LootTable) return FSBLootDrop();

    TArray<FSBLootTableEntry*> Entries;
    LootTable->GetAllRows<FSBLootTableEntry>(TEXT("RollSingle"), Entries);
    return RollFromEntries(Entries);
}

FSBLootDrop USBLootManager::RollFromEntries(const TArray<FSBLootTableEntry*>& Entries) const
{
    FSBLootDrop Result;
    if (Entries.Num() == 0) return Result;

    // Weighted random selection
    float TotalWeight = 0.0f;
    for (const FSBLootTableEntry* Entry : Entries)
    {
        TotalWeight += Entry->SpawnWeight;
    }

    float Roll = FMath::FRandRange(0.0f, TotalWeight);
    float Accumulated = 0.0f;

    for (const FSBLootTableEntry* Entry : Entries)
    {
        Accumulated += Entry->SpawnWeight;
        if (Roll <= Accumulated)
        {
            Result.ItemID = Entry->ItemID;
            Result.Count = FMath::RandRange(Entry->MinCount, Entry->MaxCount);
            return Result;
        }
    }

    // Fallback
    Result.ItemID = Entries.Last()->ItemID;
    Result.Count = Entries.Last()->MinCount;
    return Result;
}
