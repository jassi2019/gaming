// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SBMinimapDataComponent.generated.h"

class ASBZoneManager;
class ASBAircraftSystem;

UENUM(BlueprintType)
enum class ESBMapMarkerType : uint8
{
    Player,
    Teammate,
    Vehicle,
    AirDrop,
    DeathCrate,
    SafeZone,
    BlueZone,
    AircraftPath,
    PingLocation
};

USTRUCT(BlueprintType)
struct FSBMapMarker
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    ESBMapMarkerType Type = ESBMapMarkerType::Player;

    UPROPERTY(BlueprintReadOnly)
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float Rotation = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FLinearColor Color = FLinearColor::White;

    UPROPERTY(BlueprintReadOnly)
    bool bIsVisible = true;
};

/**
 * Provides data for minimap and compass HUD widgets.
 * Collects markers from zone, aircraft, teammates, vehicles, air drops.
 * Polled each frame by the minimap widget.
 */
UCLASS(ClassGroup = "StormBreaker", meta = (BlueprintSpawnableComponent))
class STORMBREAKER_API USBMinimapDataComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USBMinimapDataComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Markers ---

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Minimap")
    const TArray<FSBMapMarker>& GetMarkers() const { return Markers; }

    // --- Compass ---

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Minimap")
    float GetCompassHeading() const;

    // --- Ping ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Minimap")
    void SetPingLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Minimap")
    void ClearPing();

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Minimap")
    float MarkerUpdateRate = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Minimap")
    float TeammateVisibleRange = 50000.0f;

private:
    void CollectMarkers();

    TArray<FSBMapMarker> Markers;
    FVector PingLocation;
    bool bHasPing;
    float MarkerUpdateTimer;
};
