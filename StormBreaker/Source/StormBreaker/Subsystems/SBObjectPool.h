// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SBObjectPool.generated.h"

/**
 * Generic actor object pool for recycling frequently spawned/destroyed actors.
 * Used for: projectiles, impact decals, shell casings, VFX actors.
 * Reduces GC pressure and allocation hitches on mobile.
 */
UCLASS()
class STORMBREAKER_API USBObjectPool : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Pool")
    AActor* Acquire(TSubclassOf<AActor> ActorClass, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Pool")
    void Release(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Pool")
    void PreWarm(TSubclassOf<AActor> ActorClass, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Pool")
    void DrainPool(TSubclassOf<AActor> ActorClass);

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Pool")
    int32 GetPoolSize(TSubclassOf<AActor> ActorClass) const;

private:
    TMap<UClass*, TArray<TObjectPtr<AActor>>> Pools;
};
