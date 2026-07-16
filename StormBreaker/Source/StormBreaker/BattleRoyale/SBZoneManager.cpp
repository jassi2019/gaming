// Copyright StormBreaker Games. All Rights Reserved.

#include "BattleRoyale/SBZoneManager.h"
#include "StormBreaker.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DamageEvents.h"

ASBZoneManager::ASBZoneManager()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;

    CurrentPhaseIndex = -1;
    bIsShrinking = false;
    bIsWaiting = false;
    WaitElapsed = 0.0f;
    ShrinkElapsed = 0.0f;
    CurrentRadius = 10000.0f;
    TargetRadius = 10000.0f;
    InitialMapRadius = 10000.0f;

    // Default phase table (PUBG-style)
    Phases.Add({0.50f, 300.0f, 300.0f, 0.4f, 0.25f});   // Phase 1
    Phases.Add({0.50f, 200.0f, 200.0f, 0.6f, 0.25f});   // Phase 2
    Phases.Add({0.50f, 150.0f, 150.0f, 1.0f, 0.20f});   // Phase 3
    Phases.Add({0.50f, 120.0f, 120.0f, 2.0f, 0.15f});   // Phase 4
    Phases.Add({0.50f, 90.0f,  90.0f,  3.0f, 0.10f});   // Phase 5
    Phases.Add({0.50f, 60.0f,  60.0f,  5.0f, 0.05f});   // Phase 6
    Phases.Add({0.50f, 30.0f,  30.0f,  8.0f, 0.0f});    // Phase 7
    Phases.Add({0.00f, 0.0f,   30.0f, 14.0f, 0.0f});    // Phase 8 — final collapse
}

void ASBZoneManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASBZoneManager, CurrentCenter);
    DOREPLIFETIME(ASBZoneManager, CurrentRadius);
    DOREPLIFETIME(ASBZoneManager, TargetCenter);
    DOREPLIFETIME(ASBZoneManager, TargetRadius);
    DOREPLIFETIME(ASBZoneManager, CurrentPhaseIndex);
    DOREPLIFETIME(ASBZoneManager, bIsShrinking);
    DOREPLIFETIME(ASBZoneManager, bIsWaiting);
}

void ASBZoneManager::InitZone(const FVector& MapCenter, float InitialRadius)
{
    CurrentCenter = MapCenter;
    CurrentRadius = InitialRadius;
    TargetCenter = MapCenter;
    TargetRadius = InitialRadius;
    InitialMapRadius = InitialRadius;
    CurrentPhaseIndex = -1;

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Zone initialized. Center: %s, Radius: %.0f"),
        *MapCenter.ToString(), InitialRadius);
}

void ASBZoneManager::StartNextPhase()
{
    if (!HasAuthority()) return;

    CurrentPhaseIndex++;
    if (!Phases.IsValidIndex(CurrentPhaseIndex))
    {
        UE_LOG(LogSBBattleRoyale, Log, TEXT("All zone phases complete."));
        return;
    }

    GenerateNextTarget();

    bIsWaiting = true;
    bIsShrinking = false;
    WaitElapsed = 0.0f;

    OnZonePhaseChanged.Broadcast(CurrentPhaseIndex);

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Zone phase %d started. Wait: %.0fs, Target radius: %.0f"),
        CurrentPhaseIndex, Phases[CurrentPhaseIndex].WaitDuration, TargetRadius);
}

void ASBZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasAuthority()) return;

    if (bIsWaiting)
    {
        TickWaiting(DeltaTime);
    }
    else if (bIsShrinking)
    {
        TickShrinking(DeltaTime);
    }

    ApplyZoneDamage(DeltaTime);
}

void ASBZoneManager::TickWaiting(float DeltaTime)
{
    if (!Phases.IsValidIndex(CurrentPhaseIndex)) return;

    WaitElapsed += DeltaTime;
    if (WaitElapsed >= Phases[CurrentPhaseIndex].WaitDuration)
    {
        bIsWaiting = false;
        bIsShrinking = true;
        ShrinkElapsed = 0.0f;
        ShrinkStartCenter = CurrentCenter;
        ShrinkStartRadius = CurrentRadius;

        OnZoneShrinkStarted.Broadcast();
    }
}

void ASBZoneManager::TickShrinking(float DeltaTime)
{
    if (!Phases.IsValidIndex(CurrentPhaseIndex)) return;

    const FSBZonePhase& Phase = Phases[CurrentPhaseIndex];
    ShrinkElapsed += DeltaTime;

    float Alpha = FMath::Clamp(ShrinkElapsed / Phase.ShrinkDuration, 0.0f, 1.0f);
    float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

    CurrentRadius = FMath::Lerp(ShrinkStartRadius, TargetRadius, EasedAlpha);
    CurrentCenter = FMath::Lerp(ShrinkStartCenter, TargetCenter, EasedAlpha);

    if (Alpha >= 1.0f)
    {
        CompleteShrink();
    }
}

void ASBZoneManager::CompleteShrink()
{
    bIsShrinking = false;
    CurrentRadius = TargetRadius;
    CurrentCenter = TargetCenter;

    OnZoneShrinkCompleted.Broadcast();

    // Auto-start next phase
    StartNextPhase();
}

void ASBZoneManager::GenerateNextTarget()
{
    if (!Phases.IsValidIndex(CurrentPhaseIndex)) return;

    const FSBZonePhase& Phase = Phases[CurrentPhaseIndex];
    TargetRadius = CurrentRadius * Phase.RadiusMultiplier;

    // Shift center randomly within current zone
    float ShiftAmount = CurrentRadius * Phase.CenterShiftFactor;
    FVector Shift(
        FMath::FRandRange(-ShiftAmount, ShiftAmount),
        FMath::FRandRange(-ShiftAmount, ShiftAmount),
        0.0f);

    TargetCenter = CurrentCenter + Shift;

    // Clamp to keep target within current zone
    float MaxCenterDist = CurrentRadius - TargetRadius;
    FVector CenterDelta = TargetCenter - CurrentCenter;
    CenterDelta.Z = 0.0f;
    if (CenterDelta.Size2D() > MaxCenterDist)
    {
        CenterDelta = CenterDelta.GetSafeNormal2D() * MaxCenterDist;
        TargetCenter = CurrentCenter + CenterDelta;
    }
    TargetCenter.Z = CurrentCenter.Z;
}

void ASBZoneManager::ApplyZoneDamage(float DeltaTime)
{
    if (CurrentPhaseIndex < 0) return;

    float DPS = GetCurrentDPS();
    if (DPS <= 0.0f) return;

    float Damage = DPS * DeltaTime;

    for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
    {
        ACharacter* Character = *It;
        if (!Character || !Character->GetController()) continue;

        if (!IsLocationInSafeZone(Character->GetActorLocation()))
        {
            FDamageEvent DamageEvent;
            Character->TakeDamage(Damage, DamageEvent, nullptr, this);
        }
    }
}

// ============================================================================
// Queries
// ============================================================================

float ASBZoneManager::GetTimeUntilShrink() const
{
    if (!bIsWaiting || !Phases.IsValidIndex(CurrentPhaseIndex)) return 0.0f;
    return FMath::Max(0.0f, Phases[CurrentPhaseIndex].WaitDuration - WaitElapsed);
}

float ASBZoneManager::GetShrinkProgress() const
{
    if (!bIsShrinking || !Phases.IsValidIndex(CurrentPhaseIndex)) return 0.0f;
    return FMath::Clamp(ShrinkElapsed / Phases[CurrentPhaseIndex].ShrinkDuration, 0.0f, 1.0f);
}

bool ASBZoneManager::IsLocationInSafeZone(const FVector& Location) const
{
    float Dist2D = FVector::Dist2D(Location, CurrentCenter);
    return Dist2D <= CurrentRadius;
}

float ASBZoneManager::GetCurrentDPS() const
{
    if (!Phases.IsValidIndex(CurrentPhaseIndex)) return 0.0f;
    return Phases[CurrentPhaseIndex].DamagePerSecond;
}
