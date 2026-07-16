// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Inventory/SBInventoryTypes.h"
#include "SBLootManager.generated.h"

class UDataTable;

USTRUCT(BlueprintType)
struct FSBLootDrop
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FName ItemID;

    UPROPERTY(BlueprintReadOnly)
    int32 Count = 1;
};

/**
 * World subsystem managing loot table queries and random loot generation.
 * Used by spawn points, air drops, and death crates.
 */
UCLASS()
class STORMBREAKER_API USBLootManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // --- Loot Generation ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Loot")
    TArray<FSBLootDrop> GenerateLootFromTable(UDataTable* LootTable, int32 NumItems) const;

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Loot")
    TArray<FSBLootDrop> GenerateAirDropLoot() const;

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Loot")
    TArray<FSBLootDrop> GenerateBuildingLoot() const;

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Loot")
    FSBLootDrop RollSingleItem(UDataTable* LootTable) const;

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Loot")
    TObjectPtr<UDataTable> DefaultLootTable;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Loot")
    TObjectPtr<UDataTable> AirDropLootTable;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Loot")
    int32 MinBuildingLoot = 2;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Loot")
    int32 MaxBuildingLoot = 5;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Loot")
    int32 AirDropItemCount = 4;

private:
    FSBLootDrop RollFromEntries(const TArray<FSBLootTableEntry*>& Entries) const;
};
