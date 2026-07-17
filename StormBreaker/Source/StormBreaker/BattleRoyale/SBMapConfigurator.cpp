// Copyright Island Of Death Games. All Rights Reserved.

#include "BattleRoyale/SBMapConfigurator.h"
#include "BattleRoyale/SBZoneManager.h"
#include "StormBreaker.h"
#include "GameFramework/PlayerStart.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/BrushComponent.h"

ASBMapConfigurator::ASBMapConfigurator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASBMapConfigurator::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    UE_LOG(LogSBBattleRoyale, Log, TEXT("MapConfigurator: Building arena (radius %.0f)..."), MapRadius);

    if (bSpawnGround) SpawnGround();
    SpawnPlayerStarts();
    SpawnCoverStructures();
    if (bSpawnZoneManager) SpawnZoneManager();

    UE_LOG(LogSBBattleRoyale, Log, TEXT("MapConfigurator: Arena built. %d starts, %d cover structures."),
        NumPlayerStarts, NumCoverStructures);
}

void ASBMapConfigurator::SpawnGround()
{
    // Spawn a large flat cube as ground
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* Ground = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(),
        FVector(0, 0, -50.0f), FRotator::ZeroRotator, Params);

    if (Ground)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Ground);
        MeshComp->RegisterComponent();
        Ground->SetRootComponent(MeshComp);

        // Use engine cube mesh
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            MeshComp->SetStaticMesh(CubeMesh);
            float GroundScale = MapRadius / 50.0f;
            MeshComp->SetWorldScale3D(FVector(GroundScale, GroundScale, 1.0f));
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        }
    }
}

void ASBMapConfigurator::SpawnPlayerStarts()
{
    float AngleStep = 360.0f / FMath::Max(1, NumPlayerStarts);

    for (int32 i = 0; i < NumPlayerStarts; i++)
    {
        float Angle = FMath::DegreesToRadians(AngleStep * i);
        float SpawnRadius = 1000.0f;
        FVector Location(FMath::Cos(Angle) * SpawnRadius, FMath::Sin(Angle) * SpawnRadius, 100.0f);

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
            Location, FRotator(0, FMath::RadiansToDegrees(Angle) + 180.0f, 0), Params);
    }
}

void ASBMapConfigurator::SpawnCoverStructures()
{
    for (int32 i = 0; i < NumCoverStructures; i++)
    {
        float Angle = FMath::FRandRange(0.0f, UE_TWO_PI);
        float Radius = FMath::FRandRange(500.0f, CoverSpreadRadius);
        FVector Location(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

        int32 Type = FMath::RandRange(0, 3);
        switch (Type)
        {
        case 0: // Low wall (vault height ~80cm)
            SpawnCoverBox(Location, FVector(3.0f, 0.5f, 0.8f));
            break;
        case 1: // Medium wall (mantle height ~180cm)
            SpawnCoverBox(Location, FVector(3.0f, 0.5f, 1.8f));
            break;
        case 2: // Building-like block
            SpawnCoverBox(Location, FVector(4.0f, 4.0f, 3.0f));
            break;
        case 3: // Tall thin cover
            SpawnCoverBox(Location, FVector(2.0f, 0.3f, 2.5f));
            break;
        }
    }
}

void ASBMapConfigurator::SpawnCoverBox(const FVector& Location, const FVector& Scale)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* Cover = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(),
        Location + FVector(0, 0, Scale.Z * 50.0f), FRotator(0, FMath::FRandRange(0.0f, 360.0f), 0), Params);

    if (Cover)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Cover);
        MeshComp->RegisterComponent();
        Cover->SetRootComponent(MeshComp);

        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            MeshComp->SetStaticMesh(CubeMesh);
            MeshComp->SetWorldScale3D(Scale);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        }
    }
}

void ASBMapConfigurator::SpawnZoneManager()
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ASBZoneManager* Zone = GetWorld()->SpawnActor<ASBZoneManager>(
        ASBZoneManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);

    if (Zone)
    {
        Zone->InitZone(FVector::ZeroVector, MapRadius);
    }
}

void ASBMapConfigurator::SpawnNavMeshVolume()
{
    // NavMesh bounds must be placed in-editor or via World Settings
    // Runtime spawning of ANavMeshBoundsVolume is not supported by UE
    UE_LOG(LogSBBattleRoyale, Log, TEXT("NavMesh: Place ANavMeshBoundsVolume in editor for AI navigation."));
}
