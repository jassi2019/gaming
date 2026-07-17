// Copyright Island Of Death Games. All Rights Reserved.

#include "Subsystems/SBObjectPool.h"
#include "StormBreaker.h"

void USBObjectPool::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogStormBreaker, Log, TEXT("Object Pool initialized."));
}

void USBObjectPool::Deinitialize()
{
    for (auto& Pair : Pools)
    {
        for (AActor* Actor : Pair.Value)
        {
            if (Actor) Actor->Destroy();
        }
    }
    Pools.Empty();
    Super::Deinitialize();
}

AActor* USBObjectPool::Acquire(TSubclassOf<AActor> ActorClass, const FVector& Location, const FRotator& Rotation)
{
    if (!ActorClass) return nullptr;

    TArray<TObjectPtr<AActor>>& Pool = Pools.FindOrAdd(ActorClass.Get());

    // Try to reuse from pool
    for (int32 i = Pool.Num() - 1; i >= 0; i--)
    {
        AActor* Actor = Pool[i];
        if (Actor && Actor->IsHidden())
        {
            Pool.RemoveAt(i);
            Actor->SetActorLocationAndRotation(Location, Rotation);
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorEnableCollision(true);
            Actor->SetActorTickEnabled(true);
            return Actor;
        }
    }

    // Spawn new if pool empty
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    return World->SpawnActor<AActor>(ActorClass, Location, Rotation, Params);
}

void USBObjectPool::Release(AActor* Actor)
{
    if (!Actor) return;

    Actor->SetActorHiddenInGame(true);
    Actor->SetActorEnableCollision(false);
    Actor->SetActorTickEnabled(false);
    Actor->SetActorLocation(FVector(0, 0, -10000.0f));

    TArray<TObjectPtr<AActor>>& Pool = Pools.FindOrAdd(Actor->GetClass());
    Pool.Add(Actor);
}

void USBObjectPool::PreWarm(TSubclassOf<AActor> ActorClass, int32 Count)
{
    if (!ActorClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    for (int32 i = 0; i < Count; i++)
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* Actor = World->SpawnActor<AActor>(ActorClass, FVector(0, 0, -10000.0f), FRotator::ZeroRotator, Params);
        if (Actor)
        {
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorEnableCollision(false);
            Actor->SetActorTickEnabled(false);

            TArray<TObjectPtr<AActor>>& Pool = Pools.FindOrAdd(ActorClass.Get());
            Pool.Add(Actor);
        }
    }

    UE_LOG(LogStormBreaker, Log, TEXT("Pre-warmed %d actors of class %s"), Count, *ActorClass->GetName());
}

void USBObjectPool::DrainPool(TSubclassOf<AActor> ActorClass)
{
    TArray<TObjectPtr<AActor>>* Pool = Pools.Find(ActorClass.Get());
    if (!Pool) return;

    for (AActor* Actor : *Pool)
    {
        if (Actor) Actor->Destroy();
    }
    Pool->Empty();
}

int32 USBObjectPool::GetPoolSize(TSubclassOf<AActor> ActorClass) const
{
    const TArray<TObjectPtr<AActor>>* Pool = Pools.Find(ActorClass.Get());
    return Pool ? Pool->Num() : 0;
}
