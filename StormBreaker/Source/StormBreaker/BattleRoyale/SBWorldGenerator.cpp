// Copyright StormBreaker Games. All Rights Reserved.

#include "BattleRoyale/SBWorldGenerator.h"
#include "BattleRoyale/SBZoneManager.h"
#include "BattleRoyale/SBLootSpawnPoint.h"
#include "BattleRoyale/SBDoorActor.h"
#include "StormBreaker.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerStart.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

ASBWorldGenerator::ASBWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASBWorldGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    UE_LOG(LogSBBattleRoyale, Log, TEXT("WorldGenerator: Building %.0fkm x %.0fkm island..."), MapSizeKm, MapSizeKm);

    GenerateDefaultPOIs();
    GenerateTerrain();
    GenerateWaterBodies();
    GenerateRoads();
    GeneratePOIBuildings();
    GenerateForestAndRocks();
    GenerateLootSpawnPoints();
    ConfigureZoneManager();
    SpawnPlayerStarts();

    UE_LOG(LogSBBattleRoyale, Log, TEXT("WorldGenerator: Island complete. %d POIs, %d trees, %d rocks."),
        POIs.Num(), TreeCount, RockCount);
}

// ============================================================================
// POI Definitions
// ============================================================================

void ASBWorldGenerator::GenerateDefaultPOIs()
{
    if (POIs.Num() > 0) return;

    float R = MapRadius() * 0.7f;

    POIs.Add({TEXT("Military Base"),  ESBPOIType::MilitaryBase, ESBLootTier::Military,  FVector(R * 0.6f, R * 0.2f, 0), 5000.0f, 12, 25});
    POIs.Add({TEXT("School"),         ESBPOIType::School,       ESBLootTier::Civilian,  FVector(-R * 0.3f, R * 0.5f, 0), 3000.0f, 6, 15});
    POIs.Add({TEXT("Hospital"),       ESBPOIType::Hospital,     ESBLootTier::Medical,   FVector(R * 0.1f, -R * 0.4f, 0), 3500.0f, 4, 18});
    POIs.Add({TEXT("Factory"),        ESBPOIType::Factory,      ESBLootTier::Industrial,FVector(-R * 0.5f, -R * 0.3f, 0), 4000.0f, 8, 20});
    POIs.Add({TEXT("Power Plant"),    ESBPOIType::PowerPlant,   ESBLootTier::Industrial,FVector(R * 0.4f, -R * 0.6f, 0), 3500.0f, 6, 16});
    POIs.Add({TEXT("Port"),           ESBPOIType::Port,         ESBLootTier::Military,  FVector(-R * 0.7f, R * 0.1f, 0), 4500.0f, 10, 22});
    POIs.Add({TEXT("Riverside Village"), ESBPOIType::Village,   ESBLootTier::Civilian,  FVector(R * 0.2f, R * 0.6f, 0), 3000.0f, 8, 12});
    POIs.Add({TEXT("Farm"),           ESBPOIType::Farm,         ESBLootTier::Civilian,  FVector(-R * 0.1f, -R * 0.7f, 0), 2500.0f, 4, 8});
    POIs.Add({TEXT("Warehouse"),      ESBPOIType::Warehouse,    ESBLootTier::Industrial,FVector(R * 0.5f, R * 0.5f, 0), 2000.0f, 3, 10});
    POIs.Add({TEXT("Watch Towers"),   ESBPOIType::WatchTower,   ESBLootTier::Military,  FVector(0.0f, 0.0f, 0), 8000.0f, 5, 8});
}

// ============================================================================
// Terrain
// ============================================================================

void ASBWorldGenerator::GenerateTerrain()
{
    float HalfSize = MapRadius();
    float CellSize = (HalfSize * 2.0f) / TerrainResolution;

    // Ground base plane
    SpawnBox(FVector(0, 0, -100.0f),
        FVector(HalfSize / 50.0f, HalfSize / 50.0f, 1.0f),
        FRotator::ZeroRotator,
        FLinearColor(0.15f, 0.35f, 0.1f));

    // Terrain heightmap using Perlin-like noise via sin/cos
    for (int32 X = 0; X < TerrainResolution; X++)
    {
        for (int32 Y = 0; Y < TerrainResolution; Y++)
        {
            float WorldX = -HalfSize + X * CellSize + CellSize * 0.5f;
            float WorldY = -HalfSize + Y * CellSize + CellSize * 0.5f;

            float Height = GetTerrainHeight(WorldX, WorldY);

            // Only spawn terrain blocks for elevated areas (hills/mountains)
            if (Height > 200.0f)
            {
                SpawnBox(FVector(WorldX, WorldY, Height * 0.5f),
                    FVector(CellSize / 100.0f, CellSize / 100.0f, Height / 100.0f),
                    FRotator::ZeroRotator,
                    FLinearColor(0.3f, 0.25f, 0.15f));
            }
        }
    }
}

float ASBWorldGenerator::GetTerrainHeight(float X, float Y) const
{
    float R = MapRadius();
    float DistFromCenter = FMath::Sqrt(X * X + Y * Y);
    float EdgeFalloff = FMath::Clamp(1.0f - (DistFromCenter / R), 0.0f, 1.0f);

    // Multi-octave noise approximation
    float Noise = 0.0f;
    Noise += FMath::Sin(X * 0.00005f) * FMath::Cos(Y * 0.00005f) * TerrainMaxHeight * 0.5f;
    Noise += FMath::Sin(X * 0.0002f + 1.5f) * FMath::Cos(Y * 0.00015f + 2.3f) * TerrainMaxHeight * 0.25f;
    Noise += FMath::Sin(X * 0.0005f + 4.7f) * FMath::Cos(Y * 0.0004f + 3.1f) * TerrainMaxHeight * 0.1f;

    return FMath::Max(0.0f, Noise * EdgeFalloff);
}

// ============================================================================
// Water
// ============================================================================

void ASBWorldGenerator::GenerateWaterBodies()
{
    // River running through the map
    float R = MapRadius() * 0.8f;
    int32 RiverSegments = 20;
    for (int32 i = 0; i < RiverSegments; i++)
    {
        float T = (float)i / RiverSegments;
        float X = FMath::Lerp(-R, R, T);
        float Y = FMath::Sin(T * UE_PI * 2.0f) * R * 0.15f + R * 0.1f;

        SpawnBox(FVector(X, Y, -20.0f),
            FVector(R / (RiverSegments * 40.0f), 8.0f, 0.2f),
            FRotator::ZeroRotator,
            FLinearColor(0.1f, 0.3f, 0.6f, 0.7f));
    }

    // Lake near center
    SpawnBox(FVector(MapRadius() * 0.15f, -MapRadius() * 0.15f, -30.0f),
        FVector(40.0f, 30.0f, 0.3f),
        FRotator::ZeroRotator,
        FLinearColor(0.08f, 0.25f, 0.5f, 0.8f));
}

// ============================================================================
// Roads
// ============================================================================

void ASBWorldGenerator::GenerateRoads()
{
    // Connect POIs with simple road segments
    for (int32 i = 0; i < POIs.Num() - 1; i++)
    {
        FVector Start = POIs[i].Location;
        FVector End = POIs[i + 1].Location;
        FVector Mid = (Start + End) * 0.5f;
        float Length = FVector::Dist2D(Start, End);
        FRotator Rotation = (End - Start).Rotation();

        SpawnBox(FVector(Mid.X, Mid.Y, 2.0f),
            FVector(Length / 100.0f, 0.6f, 0.02f),
            Rotation,
            FLinearColor(0.3f, 0.3f, 0.3f));
    }
}

// ============================================================================
// POI Buildings
// ============================================================================

void ASBWorldGenerator::GeneratePOIBuildings()
{
    for (const FSBPOI& POI : POIs)
    {
        for (int32 i = 0; i < POI.BuildingCount; i++)
        {
            float Angle = (UE_TWO_PI / POI.BuildingCount) * i + FMath::FRandRange(-0.3f, 0.3f);
            float Dist = FMath::FRandRange(POI.Radius * 0.1f, POI.Radius * 0.8f);
            FVector Loc = POI.Location + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0);
            FRotator Rot(0, FMath::FRandRange(0.0f, 360.0f), 0);

            switch (POI.Type)
            {
            case ESBPOIType::WatchTower:
                SpawnWatchTower(Loc);
                break;

            case ESBPOIType::MilitaryBase:
            case ESBPOIType::PowerPlant:
                SpawnBuilding(Loc, Rot, FMath::RandRange(1, 2), 1500.0f, 800.0f);
                break;

            case ESBPOIType::Hospital:
            case ESBPOIType::School:
                SpawnBuilding(Loc, Rot, FMath::RandRange(2, 4), 1200.0f, 1000.0f);
                break;

            case ESBPOIType::Factory:
            case ESBPOIType::Warehouse:
                SpawnBuilding(Loc, Rot, 1, 2000.0f, 1500.0f);
                break;

            default:
                SpawnBuilding(Loc, Rot, FMath::RandRange(1, 2), 800.0f, 600.0f);
                break;
            }
        }
    }
}

void ASBWorldGenerator::SpawnBuilding(const FVector& Location, const FRotator& Rotation, int32 Floors, float Width, float Depth)
{
    float FloorHeight = 350.0f;
    float WallThickness = 20.0f;

    for (int32 Floor = 0; Floor < Floors; Floor++)
    {
        float BaseZ = Location.Z + Floor * FloorHeight;

        // Floor slab
        SpawnBox(FVector(Location.X, Location.Y, BaseZ + 5.0f),
            FVector(Width / 100.0f, Depth / 100.0f, 0.1f), Rotation,
            FLinearColor(0.4f, 0.4f, 0.4f));

        // Four walls with gap for door
        float HalfW = Width * 0.5f;
        float HalfD = Depth * 0.5f;
        float WallH = FloorHeight - 20.0f;

        FVector Forward = Rotation.Vector();
        FVector Right = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

        // Front wall (with door gap)
        SpawnBox(Location + Forward * HalfD + FVector(0, 0, BaseZ + WallH * 0.5f) + Right * HalfW * 0.5f,
            FVector(HalfW / 200.0f, WallThickness / 100.0f, WallH / 100.0f), Rotation,
            FLinearColor(0.6f, 0.55f, 0.45f));

        // Back wall
        SpawnBox(Location - Forward * HalfD + FVector(0, 0, BaseZ + WallH * 0.5f),
            FVector(Width / 200.0f, WallThickness / 100.0f, WallH / 100.0f), Rotation,
            FLinearColor(0.6f, 0.55f, 0.45f));

        // Left wall
        SpawnBox(Location - Right * HalfW + FVector(0, 0, BaseZ + WallH * 0.5f),
            FVector(WallThickness / 100.0f, Depth / 200.0f, WallH / 100.0f), Rotation,
            FLinearColor(0.55f, 0.5f, 0.4f));

        // Right wall
        SpawnBox(Location + Right * HalfW + FVector(0, 0, BaseZ + WallH * 0.5f),
            FVector(WallThickness / 100.0f, Depth / 200.0f, WallH / 100.0f), Rotation,
            FLinearColor(0.55f, 0.5f, 0.4f));

        // Staircase between floors
        if (Floor < Floors - 1)
        {
            int32 StairSteps = 8;
            for (int32 S = 0; S < StairSteps; S++)
            {
                float StepZ = BaseZ + (FloorHeight / StairSteps) * S;
                float StepOffset = (Depth * 0.3f / StairSteps) * S;
                SpawnBox(Location + Forward * (-HalfD * 0.6f + StepOffset) + Right * HalfW * 0.6f + FVector(0, 0, StepZ),
                    FVector(1.5f, 0.8f, 0.15f), Rotation,
                    FLinearColor(0.45f, 0.4f, 0.35f));
            }
        }
    }

    // Roof
    float RoofZ = Location.Z + Floors * FloorHeight;
    SpawnBox(FVector(Location.X, Location.Y, RoofZ),
        FVector(Width / 90.0f, Depth / 90.0f, 0.15f), Rotation,
        FLinearColor(0.35f, 0.3f, 0.25f));

    // Spawn a door at the front
    FVector DoorLoc = Location + Rotation.Vector() * (Depth * 0.5f);
    DoorLoc.Z = Location.Z + 150.0f;
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    GetWorld()->SpawnActor<ASBDoorActor>(ASBDoorActor::StaticClass(), DoorLoc, Rotation, Params);
}

void ASBWorldGenerator::SpawnWatchTower(const FVector& Location)
{
    float TowerHeight = 1500.0f;

    // Four legs
    for (int32 i = 0; i < 4; i++)
    {
        float Angle = UE_PI * 0.25f + (UE_PI * 0.5f) * i;
        FVector LegPos = Location + FVector(FMath::Cos(Angle) * 150.0f, FMath::Sin(Angle) * 150.0f, TowerHeight * 0.5f);
        SpawnBox(LegPos, FVector(0.2f, 0.2f, TowerHeight / 100.0f), FRotator::ZeroRotator, FLinearColor(0.4f, 0.35f, 0.25f));
    }

    // Platform
    SpawnBox(Location + FVector(0, 0, TowerHeight),
        FVector(3.5f, 3.5f, 0.15f), FRotator::ZeroRotator, FLinearColor(0.45f, 0.4f, 0.3f));

    // Railing
    for (int32 i = 0; i < 4; i++)
    {
        float Angle = (UE_PI * 0.5f) * i;
        FVector RailPos = Location + FVector(FMath::Cos(Angle) * 170.0f, FMath::Sin(Angle) * 170.0f, TowerHeight + 50.0f);
        SpawnBox(RailPos, FVector(3.5f, 0.05f, 0.5f), FRotator(0, FMath::RadiansToDegrees(Angle), 0), FLinearColor(0.35f, 0.3f, 0.2f));
    }
}

// ============================================================================
// Forest & Rocks
// ============================================================================

void ASBWorldGenerator::GenerateForestAndRocks()
{
    float R = MapRadius() * 0.85f;

    for (int32 i = 0; i < TreeCount; i++)
    {
        float Angle = FMath::FRandRange(0.0f, UE_TWO_PI);
        float Dist = FMath::FRandRange(1000.0f, R);
        FVector Loc(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0);
        float Height = FMath::FRandRange(500.0f, 900.0f);

        // Trunk
        SpawnBox(Loc + FVector(0, 0, Height * 0.5f),
            FVector(0.3f, 0.3f, Height / 100.0f), FRotator::ZeroRotator,
            FLinearColor(0.35f, 0.2f, 0.1f));

        // Canopy
        SpawnBox(Loc + FVector(0, 0, Height * 0.85f),
            FVector(2.5f, 2.5f, 2.0f), FRotator(0, FMath::FRandRange(0.0f, 360.0f), 0),
            FLinearColor(0.1f, 0.4f, 0.08f));
    }

    for (int32 i = 0; i < RockCount; i++)
    {
        float Angle = FMath::FRandRange(0.0f, UE_TWO_PI);
        float Dist = FMath::FRandRange(500.0f, R);
        FVector Loc(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0);
        float Scale = FMath::FRandRange(0.5f, 3.0f);

        SpawnBox(Loc + FVector(0, 0, Scale * 25.0f),
            FVector(Scale, Scale * 0.8f, Scale * 0.6f),
            FRotator(FMath::FRandRange(-10.0f, 10.0f), FMath::FRandRange(0.0f, 360.0f), 0),
            FLinearColor(0.4f, 0.38f, 0.35f));
    }
}

// ============================================================================
// Loot Spawn Points
// ============================================================================

void ASBWorldGenerator::GenerateLootSpawnPoints()
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (const FSBPOI& POI : POIs)
    {
        for (int32 i = 0; i < POI.LootSpawnCount; i++)
        {
            float Angle = FMath::FRandRange(0.0f, UE_TWO_PI);
            float Dist = FMath::FRandRange(0.0f, POI.Radius * 0.7f);
            FVector Loc = POI.Location + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 50.0f);

            ASBLootSpawnPoint* SpawnPt = GetWorld()->SpawnActor<ASBLootSpawnPoint>(
                ASBLootSpawnPoint::StaticClass(), Loc, FRotator::ZeroRotator, Params);

            if (SpawnPt)
            {
                SpawnPt->BuildingTag = FName(*POI.Name);
                SpawnPt->MinItems = (POI.LootTier == ESBLootTier::Military) ? 2 : 1;
                SpawnPt->MaxItems = (POI.LootTier == ESBLootTier::Military) ? 4 : 2;
                SpawnPt->SpawnLoot();
            }
        }
    }
}

// ============================================================================
// Zone & Player Starts
// ============================================================================

void ASBWorldGenerator::ConfigureZoneManager()
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ASBZoneManager* Zone = GetWorld()->SpawnActor<ASBZoneManager>(
        ASBZoneManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);

    if (Zone)
    {
        Zone->InitZone(FVector::ZeroVector, MapRadius());
    }
}

void ASBWorldGenerator::SpawnPlayerStarts()
{
    float R = MapRadius() * 0.3f;
    int32 Count = 10;

    for (int32 i = 0; i < Count; i++)
    {
        float Angle = (UE_TWO_PI / Count) * i;
        FVector Loc(FMath::Cos(Angle) * R, FMath::Sin(Angle) * R, 100.0f);

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), Loc,
            FRotator(0, FMath::RadiansToDegrees(Angle) + 180.0f, 0), Params);
    }
}

// ============================================================================
// Helpers
// ============================================================================

void ASBWorldGenerator::SpawnBox(const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* Box = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation, Params);
    if (!Box) return;

    UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Box);
    Mesh->RegisterComponent();
    Box->SetRootComponent(Mesh);

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh)
    {
        Mesh->SetStaticMesh(CubeMesh);
        Mesh->SetWorldScale3D(Scale);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetCollisionResponseToAllChannels(ECR_Block);
    }
}
