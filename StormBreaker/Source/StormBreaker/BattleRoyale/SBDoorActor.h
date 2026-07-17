// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBDoorActor.generated.h"

/**
 * Interactive door — opens/closes on interaction.
 * Rotates 90 degrees, replicated, supports both push directions.
 */
UCLASS()
class STORMBREAKER_API ASBDoorActor : public AActor
{
    GENERATED_BODY()

public:
    ASBDoorActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Door")
    void ToggleDoor(AActor* Interactor);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> DoorMesh;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Door")
    float OpenAngle = 90.0f;

    UPROPERTY(EditAnywhere, Category = "IslandOfDeath|Door")
    float OpenSpeed = 4.0f;

protected:
    UPROPERTY(ReplicatedUsing = OnRep_IsOpen)
    uint8 bIsOpen : 1;

    UFUNCTION()
    void OnRep_IsOpen();

private:
    float CurrentAngle;
    UPROPERTY(Replicated)
    float TargetAngle;
    FRotator ClosedRotation;
};
