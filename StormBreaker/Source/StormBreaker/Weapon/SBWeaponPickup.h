// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/SBWeaponTypes.h"
#include "SBWeaponPickup.generated.h"

class USBWeaponDataAsset;
class USphereComponent;
class UWidgetComponent;

/**
 * World pickup actor for weapons and ammo.
 * Spawned by loot system or dropped by players.
 * Interaction triggers weapon addition to the player's inventory.
 */
UCLASS()
class STORMBREAKER_API ASBWeaponPickup : public AActor
{
    GENERATED_BODY()

public:
    ASBWeaponPickup();

    void InitPickup(USBWeaponDataAsset* InData, int32 InMagAmmo, int32 InReserveAmmo);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Components ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> PickupMeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USphereComponent> InteractionSphere;

    // --- Data ---

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "IslandOfDeath|Pickup")
    TObjectPtr<USBWeaponDataAsset> WeaponData;

    UPROPERTY(Replicated)
    int32 StoredMagazineAmmo;

    UPROPERTY(Replicated)
    int32 StoredReserveAmmo;

    // --- Interaction ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Pickup")
    void Interact(APawn* InteractingPawn);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    float BobTimer;
    float BaseZ;
};
