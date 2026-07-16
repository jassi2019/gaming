// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/SBInventoryTypes.h"
#include "SBAirDrop.generated.h"

class UBoxComponent;
class UNiagaraComponent;

UENUM(BlueprintType)
enum class ESBAirDropState : uint8
{
    Flying,
    Dropping,
    Landed
};

/**
 * Air drop crate — spawned by game mode at random safe zone locations.
 * Falls from altitude with smoke trail, contains high-tier loot.
 */
UCLASS()
class STORMBREAKER_API ASBAirDrop : public AActor
{
    GENERATED_BODY()

public:
    ASBAirDrop();

    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Setup ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|AirDrop")
    void InitAirDrop(const TArray<FSBLootDrop>& InLoot, const FVector& DropLocation);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|AirDrop")
    void BeginDrop();

    // --- Loot ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|AirDrop")
    bool TakeItem(APawn* Looter, int32 ItemIndex);

    UFUNCTION(BlueprintPure, Category = "StormBreaker|AirDrop")
    const TArray<FSBLootDrop>& GetLoot() const { return Loot; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|AirDrop")
    ESBAirDropState GetDropState() const { return DropState; }

    // --- Components ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> CrateMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UBoxComponent> InteractionBox;

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|AirDrop")
    float DropSpeed = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|AirDrop")
    float DropAltitude = 15000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|AirDrop")
    TObjectPtr<UNiagaraSystem> SmokeTrailVFX;

protected:
    UPROPERTY(Replicated)
    ESBAirDropState DropState;

    UPROPERTY(Replicated)
    TArray<FSBLootDrop> Loot;

private:
    FVector TargetLocation;
    float GroundZ;

    UPROPERTY()
    TObjectPtr<UNiagaraComponent> ActiveSmokeTrail;
};
