// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SBBotTypes.generated.h"

UENUM(BlueprintType)
enum class ESBBotDifficulty : uint8
{
    Easy,
    Normal,
    Hard,
    Pro
};

UENUM(BlueprintType)
enum class ESBBotState : uint8
{
    Idle,
    Looting,
    Rotating,
    Engaging,
    Healing,
    Reviving,
    Fleeing,
    TakingCover,
    Flanking,
    Dead
};

USTRUCT(BlueprintType)
struct FSBBotDifficultySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AimAccuracy = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AimSpeed = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReactionTime = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BurstDuration = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BurstCooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HealThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EngageDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeHealthThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ZoneMoveAheadTime = 30.0f;

    static FSBBotDifficultySettings GetPreset(ESBBotDifficulty Difficulty)
    {
        FSBBotDifficultySettings S;
        switch (Difficulty)
        {
        case ESBBotDifficulty::Easy:
            S.AimAccuracy = 0.25f; S.AimSpeed = 1.5f; S.ReactionTime = 0.8f;
            S.BurstDuration = 0.4f; S.BurstCooldown = 2.0f; S.EngageDistance = 3000.0f;
            break;
        case ESBBotDifficulty::Normal:
            S.AimAccuracy = 0.5f; S.AimSpeed = 3.0f; S.ReactionTime = 0.4f;
            S.BurstDuration = 0.8f; S.BurstCooldown = 1.0f; S.EngageDistance = 5000.0f;
            break;
        case ESBBotDifficulty::Hard:
            S.AimAccuracy = 0.75f; S.AimSpeed = 5.0f; S.ReactionTime = 0.2f;
            S.BurstDuration = 1.2f; S.BurstCooldown = 0.5f; S.EngageDistance = 7000.0f;
            break;
        case ESBBotDifficulty::Pro:
            S.AimAccuracy = 0.95f; S.AimSpeed = 8.0f; S.ReactionTime = 0.1f;
            S.BurstDuration = 2.0f; S.BurstCooldown = 0.3f; S.EngageDistance = 10000.0f;
            break;
        }
        return S;
    }
};

USTRUCT(BlueprintType)
struct FSBBotMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<AActor> LastSeenEnemy;

    UPROPERTY(BlueprintReadOnly)
    FVector LastKnownEnemyLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float TimeSinceLastSeen = 999.0f;

    UPROPERTY(BlueprintReadOnly)
    FVector LastDamageDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float TimeSinceLastDamage = 999.0f;

    UPROPERTY(BlueprintReadOnly)
    FVector LastHeardLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float TimeSinceLastHeard = 999.0f;

    void Update(float DeltaTime)
    {
        TimeSinceLastSeen += DeltaTime;
        TimeSinceLastDamage += DeltaTime;
        TimeSinceLastHeard += DeltaTime;
    }

    bool HasRecentEnemyInfo(float Threshold = 10.0f) const
    {
        return TimeSinceLastSeen < Threshold || TimeSinceLastDamage < Threshold;
    }

    FVector GetBestEnemyLocation() const
    {
        if (TimeSinceLastSeen < TimeSinceLastDamage)
            return LastKnownEnemyLocation;
        return LastKnownEnemyLocation + LastDamageDirection * 500.0f;
    }
};
