// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBLootSpawnPoint.generated.h"

class UDataTable;

/**
 * Placed in the level to mark a loot spawn location.
 * On match start, generates random loot from its loot table and spawns pickup actors.
 * Can be grouped by building via BuildingTag for coordinated spawns.
 */
UCLASS()
class STORMBREAKER_API ASBLootSpawnPoint : public AActor
{
    GENERATED_BODY()

public:
    ASBLootSpawnPoint();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Loot")
    void SpawnLoot();

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Loot")
    TObjectPtr<UDataTable> LootTable;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Loot")
    int32 MinItems = 1;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Loot")
    int32 MaxItems = 3;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Loot")
    FName BuildingTag;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Loot")
    bool bHasSpawned = false;
};
