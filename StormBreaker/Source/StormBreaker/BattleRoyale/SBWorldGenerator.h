// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBWorldGenerator.generated.h"

class ASBZoneManager;
class ASBLootSpawnPoint;

UENUM(BlueprintType)
enum class ESBPOIType : uint8
{
    MilitaryBase,
    School,
    Hospital,
    Factory,
    PowerPlant,
    Port,
    Village,
    Farm,
    Warehouse,
    WatchTower
};

UENUM(BlueprintType)
enum class ESBLootTier : uint8
{
    Civilian,
    Industrial,
    Medical,
    Military
};

USTRUCT(BlueprintType)
struct FSBPOI
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESBPOIType Type = ESBPOIType::Village;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESBLootTier LootTier = ESBLootTier::Civilian;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BuildingCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LootSpawnCount = 10;
};

/**
 * Procedural 4x4km Battle Royale island generator.
 * Creates: terrain, 10 named POIs, buildings with interiors,
 * roads, water bodies, cover, loot spawn points, and NavMesh markers.
 * Uses engine primitives — no external assets required.
 */
UCLASS()
class STORMBREAKER_API ASBWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    ASBWorldGenerator();

    virtual void BeginPlay() override;

    // --- Config ---

    UPROPERTY(EditAnywhere, Category = "StormBreaker|World")
    float MapSizeKm = 4.0f;

    UPROPERTY(EditAnywhere, Category = "StormBreaker|World")
    int32 TerrainResolution = 64;

    UPROPERTY(EditAnywhere, Category = "StormBreaker|World")
    float TerrainMaxHeight = 3000.0f;

    UPROPERTY(EditAnywhere, Category = "StormBreaker|World")
    int32 TreeCount = 500;

    UPROPERTY(EditAnywhere, Category = "StormBreaker|World")
    int32 RockCount = 200;

    UPROPERTY(EditAnywhere, Category = "StormBreaker|World")
    TArray<FSBPOI> POIs;

    // --- Generated Data ---

    UFUNCTION(BlueprintPure, Category = "StormBreaker|World")
    const TArray<FSBPOI>& GetPOIs() const { return POIs; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|World")
    float GetMapRadius() const { return MapSizeKm * 50000.0f; }

private:
    void GenerateDefaultPOIs();
    void GenerateTerrain();
    void GenerateWaterBodies();
    void GenerateRoads();
    void GeneratePOIBuildings();
    void GenerateForestAndRocks();
    void GenerateLootSpawnPoints();
    void ConfigureZoneManager();
    void SpawnPlayerStarts();

    void SpawnBox(const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color);
    void SpawnBuilding(const FVector& Location, const FRotator& Rotation, int32 Floors, float Width, float Depth);
    void SpawnWatchTower(const FVector& Location);

    float GetTerrainHeight(float X, float Y) const;
    float MapRadius() const { return MapSizeKm * 50000.0f; }
};
