// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBMapConfigurator.generated.h"

class ASBZoneManager;
class APlayerStart;
class ANavMeshBoundsVolume;

/**
 * Drop this actor into any empty level to instantly create a playable BR arena.
 * On BeginPlay (server only), it spawns:
 * - Ground plane
 * - Cover structures (boxes at various heights for vault/mantle testing)
 * - PlayerStart actors
 * - Zone manager with configured phases
 * - NavMesh bounds volume
 *
 * This removes the need for manual level design to test the full BR loop.
 */
UCLASS()
class STORMBREAKER_API ASBMapConfigurator : public AActor
{
    GENERATED_BODY()

public:
    ASBMapConfigurator();

    virtual void BeginPlay() override;

    // --- Map Config ---

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Map")
    float MapRadius = 10000.0f;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Map")
    int32 NumPlayerStarts = 2;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Map")
    int32 NumCoverStructures = 10;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Map")
    float CoverSpreadRadius = 8000.0f;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Map")
    bool bSpawnZoneManager = true;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Map")
    bool bSpawnNavMesh = true;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Map")
    bool bSpawnGround = true;

private:
    void SpawnGround();
    void SpawnPlayerStarts();
    void SpawnCoverStructures();
    void SpawnZoneManager();
    void SpawnNavMeshVolume();
    void SpawnCoverBox(const FVector& Location, const FVector& Scale);
};
