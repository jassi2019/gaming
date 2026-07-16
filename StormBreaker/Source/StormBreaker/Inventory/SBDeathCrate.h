// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/SBInventoryTypes.h"
#include "SBDeathCrate.generated.h"

class UBoxComponent;

/**
 * Spawned when a player dies. Contains all their inventory items.
 * Other players can interact to loot. Replicated to all clients.
 */
UCLASS()
class STORMBREAKER_API ASBDeathCrate : public AActor
{
    GENERATED_BODY()

public:
    ASBDeathCrate();

    void InitFromPlayerItems(const TArray<FSBInventoryItem>& PlayerItems, const FString& PlayerName);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Loot Interface ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Loot")
    TArray<FSBInventoryItem> GetContents() const { return Contents; }

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Loot")
    bool TakeItem(APawn* Looter, const FName& ItemID, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Loot")
    void TakeAll(APawn* Looter);

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Loot")
    bool IsEmpty() const { return Contents.Num() == 0; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Loot")
    FString GetPlayerName() const { return DeadPlayerName; }

    // --- Components ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> CrateMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UBoxComponent> InteractionBox;

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|DeathCrate")
    float LifeSpan = 300.0f;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(Replicated)
    TArray<FSBInventoryItem> Contents;

    UPROPERTY(Replicated)
    FString DeadPlayerName;
};
