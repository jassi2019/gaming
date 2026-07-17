// Copyright Island Of Death Games. All Rights Reserved.

#include "BattleRoyale/SBAircraftSystem.h"
#include "StormBreaker.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

ASBAircraftSystem::ASBAircraftSystem()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    AircraftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AircraftMesh"));
    RootComponent = AircraftMesh;
    AircraftMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    bIsFlying = false;
    FlightDistance = 0.0f;
    DistanceTraveled = 0.0f;
}

void ASBAircraftSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASBAircraftSystem, PathStart);
    DOREPLIFETIME(ASBAircraftSystem, PathEnd);
    DOREPLIFETIME(ASBAircraftSystem, bIsFlying);
}

void ASBAircraftSystem::InitFlightPath(const FVector& MapCenter, float MapRadius)
{
    // Random angle for flight direction
    float Angle = FMath::FRandRange(0.0f, 360.0f);
    float AngleRad = FMath::DegreesToRadians(Angle);

    FVector Direction2D = FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.0f);

    // Offset from center for path variation
    FVector Perpendicular = FVector(-Direction2D.Y, Direction2D.X, 0.0f);
    float LateralOffset = FMath::FRandRange(-MapRadius * 0.3f, MapRadius * 0.3f);

    FVector OffsetCenter = MapCenter + Perpendicular * LateralOffset;

    PathStart = OffsetCenter - Direction2D * (MapRadius + PathOvershoot);
    PathStart.Z = FlightAltitude;

    PathEnd = OffsetCenter + Direction2D * (MapRadius + PathOvershoot);
    PathEnd.Z = FlightAltitude;

    FlightDirection = (PathEnd - PathStart).GetSafeNormal();
    FlightDistance = FVector::Dist(PathStart, PathEnd);
    CurrentPosition = PathStart;
    DistanceTraveled = 0.0f;

    SetActorLocation(PathStart);
    SetActorRotation(FlightDirection.Rotation());

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Aircraft path: %s -> %s (%.0fm, angle: %.0f°)"),
        *PathStart.ToString(), *PathEnd.ToString(), FlightDistance / 100.0f, Angle);
}

void ASBAircraftSystem::StartFlight()
{
    if (!HasAuthority()) return;

    bIsFlying = true;
    DistanceTraveled = 0.0f;
    SetActorTickEnabled(true);

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Aircraft flight started."));
}

void ASBAircraftSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsFlying || !HasAuthority()) return;

    float DeltaDistance = FlightSpeed * DeltaTime;
    DistanceTraveled += DeltaDistance;
    CurrentPosition = PathStart + FlightDirection * DistanceTraveled;

    SetActorLocation(CurrentPosition);

    if (DistanceTraveled >= FlightDistance)
    {
        EjectAllRemainingPlayers();
        bIsFlying = false;
        SetActorTickEnabled(false);
        OnPathComplete.Broadcast();

        UE_LOG(LogSBBattleRoyale, Log, TEXT("Aircraft flight complete."));

        // Destroy after a short delay
        SetLifeSpan(5.0f);
    }
}

float ASBAircraftSystem::GetFlightProgress() const
{
    if (FlightDistance <= 0.0f) return 0.0f;
    return FMath::Clamp(DistanceTraveled / FlightDistance, 0.0f, 1.0f);
}

void ASBAircraftSystem::PlayerJump(AController* Player)
{
    if (!HasAuthority() || !Player || !bIsFlying) return;

    PlayersOnBoard.Remove(Player);
    OnPlayerJumped.Broadcast(Player);

    // Detach pawn and enable parachute physics
    APawn* Pawn = Player->GetPawn();
    if (Pawn)
    {
        Pawn->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        Pawn->SetActorLocation(GetActorLocation());
        Pawn->SetActorEnableCollision(true);
        Pawn->SetActorHiddenInGame(false);
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Player %s jumped from aircraft."), *GetNameSafe(Player));
}

void ASBAircraftSystem::EjectAllRemainingPlayers()
{
    TArray<TObjectPtr<AController>> Remaining = PlayersOnBoard;
    for (AController* PC : Remaining)
    {
        if (PC)
        {
            PlayerJump(PC);
        }
    }
    PlayersOnBoard.Empty();
}
