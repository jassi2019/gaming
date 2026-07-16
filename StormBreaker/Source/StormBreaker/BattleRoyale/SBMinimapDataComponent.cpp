// Copyright StormBreaker Games. All Rights Reserved.

#include "BattleRoyale/SBMinimapDataComponent.h"
#include "BattleRoyale/SBZoneManager.h"
#include "BattleRoyale/SBAircraftSystem.h"
#include "BattleRoyale/SBAirDrop.h"
#include "Character/SBCharacterBase.h"
#include "Core/SBPlayerState.h"
#include "Inventory/SBDeathCrate.h"
#include "StormBreaker.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

USBMinimapDataComponent::USBMinimapDataComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bHasPing = false;
    MarkerUpdateTimer = 0.0f;
}

void USBMinimapDataComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    MarkerUpdateTimer += DeltaTime;
    if (MarkerUpdateTimer >= MarkerUpdateRate)
    {
        MarkerUpdateTimer = 0.0f;
        CollectMarkers();
    }
}

float USBMinimapDataComponent::GetCompassHeading() const
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return 0.0f;

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!PC) return 0.0f;

    FRotator ViewRotation = PC->GetControlRotation();
    float Heading = ViewRotation.Yaw;
    if (Heading < 0.0f) Heading += 360.0f;
    return Heading;
}

void USBMinimapDataComponent::SetPingLocation(const FVector& WorldLocation)
{
    PingLocation = WorldLocation;
    bHasPing = true;
}

void USBMinimapDataComponent::ClearPing()
{
    bHasPing = false;
}

void USBMinimapDataComponent::CollectMarkers()
{
    Markers.Reset();

    UWorld* World = GetWorld();
    if (!World) return;

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    int32 MyTeamId = -1;
    ASBPlayerState* MyPS = OwnerPawn->GetPlayerState<ASBPlayerState>();
    if (MyPS)
    {
        MyTeamId = MyPS->GetTeamId();
    }

    // Self marker
    {
        FSBMapMarker Self;
        Self.Type = ESBMapMarkerType::Player;
        Self.WorldLocation = OwnerPawn->GetActorLocation();
        Self.Rotation = OwnerPawn->GetActorRotation().Yaw;
        Self.Color = FLinearColor::Green;
        Markers.Add(Self);
    }

    // Teammates
    for (TActorIterator<ASBCharacterBase> It(World); It; ++It)
    {
        ASBCharacterBase* OtherChar = *It;
        if (!OtherChar || OtherChar == OwnerPawn) continue;

        ASBPlayerState* OtherPS = OtherChar->GetPlayerState<ASBPlayerState>();
        if (!OtherPS || OtherPS->GetTeamId() != MyTeamId || MyTeamId < 0) continue;

        float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), OtherChar->GetActorLocation());
        if (Distance > TeammateVisibleRange) continue;

        FSBMapMarker Teammate;
        Teammate.Type = ESBMapMarkerType::Teammate;
        Teammate.WorldLocation = OtherChar->GetActorLocation();
        Teammate.Rotation = OtherChar->GetActorRotation().Yaw;
        Teammate.Color = FLinearColor(0.0f, 0.5f, 1.0f);
        Markers.Add(Teammate);
    }

    // Zone
    for (TActorIterator<ASBZoneManager> It(World); It; ++It)
    {
        ASBZoneManager* Zone = *It;
        if (!Zone) continue;

        FSBMapMarker SafeZone;
        SafeZone.Type = ESBMapMarkerType::SafeZone;
        SafeZone.WorldLocation = Zone->GetTargetCenter();
        SafeZone.Rotation = Zone->GetTargetRadius();
        SafeZone.Color = FLinearColor::White;
        Markers.Add(SafeZone);

        FSBMapMarker BlueZone;
        BlueZone.Type = ESBMapMarkerType::BlueZone;
        BlueZone.WorldLocation = Zone->GetCurrentCenter();
        BlueZone.Rotation = Zone->GetCurrentRadius();
        BlueZone.Color = FLinearColor(0.0f, 0.3f, 1.0f, 0.5f);
        Markers.Add(BlueZone);
    }

    // Aircraft
    for (TActorIterator<ASBAircraftSystem> It(World); It; ++It)
    {
        ASBAircraftSystem* Aircraft = *It;
        if (!Aircraft || !Aircraft->IsInFlight()) continue;

        FSBMapMarker AircraftMarker;
        AircraftMarker.Type = ESBMapMarkerType::AircraftPath;
        AircraftMarker.WorldLocation = Aircraft->GetActorLocation();
        AircraftMarker.Color = FLinearColor::Red;
        Markers.Add(AircraftMarker);
    }

    // Air drops
    for (TActorIterator<ASBAirDrop> It(World); It; ++It)
    {
        ASBAirDrop* Drop = *It;
        if (!Drop) continue;

        FSBMapMarker DropMarker;
        DropMarker.Type = ESBMapMarkerType::AirDrop;
        DropMarker.WorldLocation = Drop->GetActorLocation();
        DropMarker.Color = FLinearColor::Yellow;
        Markers.Add(DropMarker);
    }

    // Death crates
    for (TActorIterator<ASBDeathCrate> It(World); It; ++It)
    {
        ASBDeathCrate* Crate = *It;
        if (!Crate || Crate->IsEmpty()) continue;

        float Distance = FVector::Dist2D(OwnerPawn->GetActorLocation(), Crate->GetActorLocation());
        if (Distance > 5000.0f) continue;

        FSBMapMarker CrateMarker;
        CrateMarker.Type = ESBMapMarkerType::DeathCrate;
        CrateMarker.WorldLocation = Crate->GetActorLocation();
        CrateMarker.Color = FLinearColor(1.0f, 0.5f, 0.0f);
        Markers.Add(CrateMarker);
    }

    // Ping
    if (bHasPing)
    {
        FSBMapMarker PingMarker;
        PingMarker.Type = ESBMapMarkerType::PingLocation;
        PingMarker.WorldLocation = PingLocation;
        PingMarker.Color = FLinearColor::Yellow;
        Markers.Add(PingMarker);
    }
}
