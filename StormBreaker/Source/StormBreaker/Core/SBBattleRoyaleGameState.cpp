// Copyright StormBreaker Games. All Rights Reserved.

#include "Core/SBBattleRoyaleGameState.h"
#include "StormBreaker.h"
#include "Net/UnrealNetwork.h"

ASBBattleRoyaleGameState::ASBBattleRoyaleGameState()
    : MatchPhase(ESBMatchPhase::WaitingForPlayers)
    , AlivePlayerCount(0)
    , MatchStartServerTime(0.0f)
    , bIsZoneShrinking(false)
    , ZoneShrinkElapsed(0.0f)
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASBBattleRoyaleGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASBBattleRoyaleGameState, ZoneData);
    DOREPLIFETIME(ASBBattleRoyaleGameState, MatchPhase);
    DOREPLIFETIME(ASBBattleRoyaleGameState, AlivePlayerCount);
    DOREPLIFETIME(ASBBattleRoyaleGameState, MatchStartServerTime);
}

void ASBBattleRoyaleGameState::SetMatchPhase(ESBMatchPhase NewPhase)
{
    MatchPhase = NewPhase;

    if (NewPhase == ESBMatchPhase::InProgress)
    {
        MatchStartServerTime = GetServerWorldTimeSeconds();
    }
}

void ASBBattleRoyaleGameState::SetAlivePlayerCount(int32 Count)
{
    AlivePlayerCount = Count;
}

float ASBBattleRoyaleGameState::GetMatchElapsedTime() const
{
    if (MatchStartServerTime <= 0.0f) return 0.0f;
    return GetServerWorldTimeSeconds() - MatchStartServerTime;
}

void ASBBattleRoyaleGameState::BeginZoneShrink()
{
    bIsZoneShrinking = true;
    ZoneShrinkElapsed = 0.0f;
    UE_LOG(LogSBBattleRoyale, Log, TEXT("Zone shrink started. Phase: %d, Target radius: %.0f"),
        ZoneData.CurrentPhaseIndex, ZoneData.TargetRadius);
}

void ASBBattleRoyaleGameState::AdvanceZonePhase()
{
    ZoneData.CurrentPhaseIndex++;
    ZoneData.CurrentRadius = ZoneData.TargetRadius;

    // Each phase: smaller radius, faster shrink, more damage
    ZoneData.TargetRadius *= 0.55f;
    ZoneData.ShrinkDuration = FMath::Max(30.0f, ZoneData.ShrinkDuration * 0.75f);
    ZoneData.DamagePerSecond = FMath::Min(20.0f, ZoneData.DamagePerSecond * 1.5f);

    // Shift center slightly for unpredictability
    float Offset = ZoneData.TargetRadius * 0.25f;
    ZoneData.Center += FVector(
        FMath::FRandRange(-Offset, Offset),
        FMath::FRandRange(-Offset, Offset),
        0.0f
    );

    bIsZoneShrinking = false;
    ZoneShrinkElapsed = 0.0f;

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Zone advanced to phase %d. New target radius: %.0f"),
        ZoneData.CurrentPhaseIndex, ZoneData.TargetRadius);
}

void ASBBattleRoyaleGameState::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (HasAuthority() && bIsZoneShrinking)
    {
        UpdateZoneShrink(DeltaSeconds);
    }
}

void ASBBattleRoyaleGameState::UpdateZoneShrink(float DeltaSeconds)
{
    ZoneShrinkElapsed += DeltaSeconds;

    float Alpha = FMath::Clamp(ZoneShrinkElapsed / ZoneData.ShrinkDuration, 0.0f, 1.0f);
    float StartRadius = ZoneData.CurrentRadius;
    float InterpRadius = FMath::Lerp(StartRadius, ZoneData.TargetRadius, Alpha);
    ZoneData.CurrentRadius = InterpRadius;

    if (Alpha >= 1.0f)
    {
        AdvanceZonePhase();
    }
}
