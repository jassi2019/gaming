// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBAircraftSystem.generated.h"

class USplineComponent;
class UStaticMeshComponent;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAircraftPathComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJumped, AController*, Player);

/**
 * Aircraft that flies across the map carrying players at match start.
 * Generates a random flight path, allows players to jump, and automatically
 * ejects remaining players at the end of the path.
 * Server-authoritative — spawned and driven by GameMode.
 */
UCLASS()
class STORMBREAKER_API ASBAircraftSystem : public AActor
{
    GENERATED_BODY()

public:
    ASBAircraftSystem();

    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Setup ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Aircraft")
    void InitFlightPath(const FVector& MapCenter, float MapRadius);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Aircraft")
    void StartFlight();

    // --- Player Jump ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Aircraft")
    void PlayerJump(AController* Player);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Aircraft")
    void EjectAllRemainingPlayers();

    // --- Queries ---

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Aircraft")
    FVector GetFlightPathStart() const { return PathStart; }

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Aircraft")
    FVector GetFlightPathEnd() const { return PathEnd; }

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Aircraft")
    float GetFlightProgress() const;

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Aircraft")
    bool IsInFlight() const { return bIsFlying; }

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Aircraft")
    float FlightSpeed = 5000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Aircraft")
    float FlightAltitude = 15000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Aircraft")
    float PathOvershoot = 2000.0f;

    // --- Components ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> AircraftMesh;

    // --- Delegates ---

    UPROPERTY(BlueprintAssignable)
    FOnAircraftPathComplete OnPathComplete;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerJumped OnPlayerJumped;

protected:
    UPROPERTY(Replicated)
    FVector PathStart;

    UPROPERTY(Replicated)
    FVector PathEnd;

    UPROPERTY(Replicated)
    uint8 bIsFlying : 1;

private:
    FVector CurrentPosition;
    FVector FlightDirection;
    float FlightDistance;
    float DistanceTraveled;

    UPROPERTY()
    TArray<TObjectPtr<AController>> PlayersOnBoard;
};
