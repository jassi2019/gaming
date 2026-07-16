// Copyright StormBreaker Games. All Rights Reserved.

#include "AI/SBBotController.h"
#include "Character/SBCharacterBase.h"
#include "Character/SBCharacterMovementComponent.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponBase.h"
#include "Inventory/SBInventoryComponent.h"
#include "BattleRoyale/SBZoneManager.h"
#include "BattleRoyale/SBKnockReviveComponent.h"
#include "Core/SBPlayerState.h"
#include "Core/SBAttributeSet.h"
#include "StormBreaker.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshPath.h"
#include "EngineUtils.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"

ASBBotController::ASBBotController()
{
    PrimaryActorTick.bCanEverTick = true;

    CurrentState = ESBBotState::Idle;
    StateTimer = 0.0f;
    FireTimer = 0.0f;
    AimJitterTimer = 0.0f;
    bIsFiring = false;

    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    SetupPerception();
}

void ASBBotController::SetupPerception()
{
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 8000.0f;
    SightConfig->LoseSightRadius = 10000.0f;
    SightConfig->PeripheralVisionAngleDegrees = 70.0f;
    SightConfig->SetMaxAge(15.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 5000.0f;
    HearingConfig->SetMaxAge(5.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*HearingConfig);

    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);
    PerceptionComp->ConfigureSense(*DamageConfig);

    PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void ASBBotController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    DifficultySettings = FSBBotDifficultySettings::GetPreset(Difficulty);

    PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ASBBotController::OnPerceptionUpdated);

    UE_LOG(LogSBAI, Log, TEXT("Bot %s possessed with difficulty %d"), *GetNameSafe(InPawn), (int32)Difficulty);
}

void ASBBotController::OnUnPossess()
{
    StopFiring();
    StopMovement();
    Super::OnUnPossess();
}

// ============================================================================
// Tick
// ============================================================================

void ASBBotController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!GetPawn()) return;

    ASBCharacterBase* BotChar = GetBotCharacter();
    if (!BotChar) return;

    USBKnockReviveComponent* KnockComp = BotChar->KnockReviveComponent;
    if (KnockComp && !KnockComp->IsAlive())
    {
        StopFiring();
        StopMovement();
        CurrentState = ESBBotState::Dead;
        return;
    }

    UpdateAILOD();
    Memory.Update(DeltaTime);
    DecideState(DeltaTime);

    switch (CurrentState)
    {
    case ESBBotState::Idle:       TickIdle(DeltaTime);       break;
    case ESBBotState::Looting:    TickLooting(DeltaTime);    break;
    case ESBBotState::Rotating:   TickRotating(DeltaTime);   break;
    case ESBBotState::Engaging:   TickEngaging(DeltaTime);   break;
    case ESBBotState::Healing:    TickHealing(DeltaTime);    break;
    case ESBBotState::Fleeing:    TickFleeing(DeltaTime);    break;
    case ESBBotState::TakingCover: TickTakingCover(DeltaTime); break;
    default: break;
    }
}

void ASBBotController::UpdateAILOD()
{
    float DistToPlayer = GetDistanceToNearestPlayer();
    float TickInterval;

    if (DistToPlayer < FullTickDistance)
    {
        TickInterval = MinTickInterval;
    }
    else if (DistToPlayer < ReducedTickDistance)
    {
        float Alpha = (DistToPlayer - FullTickDistance) / (ReducedTickDistance - FullTickDistance);
        TickInterval = FMath::Lerp(MinTickInterval, MaxTickInterval, Alpha);
    }
    else
    {
        TickInterval = MaxTickInterval;
    }

    PrimaryActorTick.TickInterval = TickInterval;
}

// ============================================================================
// State Decision
// ============================================================================

void ASBBotController::DecideState(float DeltaTime)
{
    StateTimer += DeltaTime;

    // Priority 1: Fleeing when very low HP
    if (GetHealthPercent() < DifficultySettings.FleeHealthThreshold && Memory.HasRecentEnemyInfo(5.0f))
    {
        if (CurrentState != ESBBotState::Fleeing)
        {
            CurrentState = ESBBotState::Fleeing;
            StateTimer = 0.0f;
        }
        return;
    }

    // Priority 2: Healing when HP below threshold
    if (NeedsHealing() && CurrentState != ESBBotState::Engaging)
    {
        if (CurrentState != ESBBotState::Healing)
        {
            CurrentState = ESBBotState::Healing;
            StateTimer = 0.0f;
        }
        return;
    }

    // Priority 3: Engage if enemy visible
    if (CurrentTarget.IsValid() && Memory.TimeSinceLastSeen < 3.0f)
    {
        if (CurrentState != ESBBotState::Engaging)
        {
            CurrentState = ESBBotState::Engaging;
            StateTimer = 0.0f;
        }
        return;
    }

    // Priority 4: Take cover if recently damaged but no target
    if (Memory.TimeSinceLastDamage < 5.0f && !CurrentTarget.IsValid())
    {
        if (CurrentState != ESBBotState::TakingCover)
        {
            CurrentState = ESBBotState::TakingCover;
            StateTimer = 0.0f;
        }
        return;
    }

    // Priority 5: Rotate to safe zone
    if (!IsInSafeZone())
    {
        if (CurrentState != ESBBotState::Rotating)
        {
            CurrentState = ESBBotState::Rotating;
            StateTimer = 0.0f;
        }
        return;
    }

    // Priority 6: Loot if needed
    if (NeedsLoot())
    {
        if (CurrentState != ESBBotState::Looting)
        {
            CurrentState = ESBBotState::Looting;
            StateTimer = 0.0f;
        }
        return;
    }

    // Default: Idle patrol
    if (CurrentState != ESBBotState::Idle)
    {
        CurrentState = ESBBotState::Idle;
        StateTimer = 0.0f;
    }
}

// ============================================================================
// State Ticks
// ============================================================================

void ASBBotController::TickIdle(float DeltaTime)
{
    // Wander within safe zone
    if (StateTimer > 5.0f)
    {
        StateTimer = 0.0f;
        FVector WanderTarget = GetPawn()->GetActorLocation() +
            FVector(FMath::FRandRange(-2000.0f, 2000.0f), FMath::FRandRange(-2000.0f, 2000.0f), 0.0f);
        MoveToLocation(WanderTarget);
    }
}

void ASBBotController::TickLooting(float DeltaTime)
{
    FVector LootLoc = FindLootLocation();
    if (!LootLoc.IsNearlyZero())
    {
        MoveToLocation(LootLoc);
    }

    if (StateTimer > 15.0f)
    {
        CurrentState = ESBBotState::Idle;
        StateTimer = 0.0f;
    }
}

void ASBBotController::TickRotating(float DeltaTime)
{
    MoveToSafeZone();

    ASBCharacterBase* BotChar = GetBotCharacter();
    if (BotChar)
    {
        USBCharacterMovementComponent* CMC = BotChar->GetSBMovement();
        if (CMC)
        {
            CMC->StartSprinting();
        }
    }
}

void ASBBotController::TickEngaging(float DeltaTime)
{
    if (!CurrentTarget.IsValid())
    {
        StopFiring();
        CurrentState = ESBBotState::Idle;
        return;
    }

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());

    AimAtTarget(DeltaTime);

    // Fire in bursts
    FireTimer += DeltaTime;
    if (!bIsFiring && FireTimer >= DifficultySettings.ReactionTime)
    {
        FireAtTarget();
        FireTimer = 0.0f;
    }
    else if (bIsFiring && FireTimer >= DifficultySettings.BurstDuration)
    {
        StopFiring();
        FireTimer = -DifficultySettings.BurstCooldown;
    }

    // Close distance or take cover based on range
    if (Distance > DifficultySettings.EngageDistance)
    {
        MoveToLocation(CurrentTarget->GetActorLocation());
    }
    else if (Distance < 1000.0f && GetHealthPercent() < 50.0f)
    {
        FVector CoverPos = FindCoverLocation(
            (CurrentTarget->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal());
        MoveToLocation(CoverPos);
    }
}

void ASBBotController::TickHealing(float DeltaTime)
{
    StopFiring();
    StopMovement();

    ASBCharacterBase* BotChar = GetBotCharacter();
    if (!BotChar) return;

    USBInventoryComponent* Inv = BotChar->InventoryComponent;
    if (!Inv || Inv->IsUsingConsumable()) return;

    // Try to use healing items in priority order
    static const TArray<FName> HealPriority = {
        FName("MedKit"), FName("FirstAidKit"), FName("Bandage")
    };

    for (const FName& ItemID : HealPriority)
    {
        if (Inv->HasItem(ItemID, 1))
        {
            Inv->UseConsumable(ItemID);
            break;
        }
    }

    if (!NeedsHealing())
    {
        CurrentState = ESBBotState::Idle;
        StateTimer = 0.0f;
    }
}

void ASBBotController::TickFleeing(float DeltaTime)
{
    StopFiring();

    FVector FleeDir = GetPawn()->GetActorLocation() - Memory.GetBestEnemyLocation();
    FleeDir.Z = 0.0f;
    FleeDir.Normalize();

    FVector FleeTarget = GetPawn()->GetActorLocation() + FleeDir * 3000.0f;
    MoveToLocation(FleeTarget);

    ASBCharacterBase* BotChar = GetBotCharacter();
    if (BotChar)
    {
        USBCharacterMovementComponent* CMC = BotChar->GetSBMovement();
        if (CMC) CMC->StartSprinting();
    }

    if (StateTimer > 5.0f)
    {
        CurrentState = ESBBotState::Healing;
        StateTimer = 0.0f;
    }
}

void ASBBotController::TickTakingCover(float DeltaTime)
{
    StopFiring();

    FVector CoverPos = FindCoverLocation(Memory.LastDamageDirection);
    MoveToLocation(CoverPos);

    if (StateTimer > 5.0f)
    {
        CurrentState = ESBBotState::Idle;
        StateTimer = 0.0f;
    }
}

// ============================================================================
// Combat
// ============================================================================

void ASBBotController::AimAtTarget(float DeltaTime)
{
    if (!CurrentTarget.IsValid() || !GetPawn()) return;

    FVector AimPoint = CalculateAimPoint(CurrentTarget.Get());
    FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(
        GetPawn()->GetActorLocation(), AimPoint);

    // Human-like aim interpolation
    FRotator CurrentRotation = GetPawn()->GetActorRotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, DesiredRotation,
        DeltaTime, DifficultySettings.AimSpeed);

    GetPawn()->SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
    SetControlRotation(NewRotation);
}

FVector ASBBotController::CalculateAimPoint(AActor* Target) const
{
    FVector TargetLocation = Target->GetActorLocation();
    TargetLocation.Z += 60.0f; // Aim at chest height

    // Add accuracy jitter based on difficulty
    float InaccuracyRadius = (1.0f - DifficultySettings.AimAccuracy) * 100.0f;
    FVector Jitter(
        FMath::FRandRange(-InaccuracyRadius, InaccuracyRadius),
        FMath::FRandRange(-InaccuracyRadius, InaccuracyRadius),
        FMath::FRandRange(-InaccuracyRadius * 0.5f, InaccuracyRadius * 0.5f));

    return TargetLocation + Jitter;
}

void ASBBotController::FireAtTarget()
{
    ASBCharacterBase* BotChar = GetBotCharacter();
    if (!BotChar || !HasWeapon()) return;

    USBWeaponComponent* WeaponComp = BotChar->WeaponComponent;
    if (WeaponComp)
    {
        WeaponComp->StartFire();
        bIsFiring = true;
    }
}

void ASBBotController::StopFiring()
{
    if (!bIsFiring) return;

    ASBCharacterBase* BotChar = GetBotCharacter();
    if (BotChar && BotChar->WeaponComponent)
    {
        BotChar->WeaponComponent->StopFire();
    }
    bIsFiring = false;
}

// ============================================================================
// Navigation
// ============================================================================

void ASBBotController::MoveToLocation(const FVector& Location)
{
    AAIController::MoveToLocation(Location, 100.0f);
}

void ASBBotController::MoveToSafeZone()
{
    if (!CachedZoneManager.IsValid())
    {
        for (TActorIterator<ASBZoneManager> It(GetWorld()); It; ++It)
        {
            CachedZoneManager = *It;
            break;
        }
    }

    if (!CachedZoneManager.IsValid()) return;

    FVector ZoneCenter = CachedZoneManager->GetTargetCenter();
    float ZoneRadius = CachedZoneManager->GetTargetRadius();

    // Move toward a random point within the safe zone
    FVector Direction = (ZoneCenter - GetPawn()->GetActorLocation()).GetSafeNormal2D();
    float DistToCenter = FVector::Dist2D(GetPawn()->GetActorLocation(), ZoneCenter);

    FVector MoveTarget;
    if (DistToCenter > ZoneRadius * 0.8f)
    {
        MoveTarget = ZoneCenter + FVector(
            FMath::FRandRange(-ZoneRadius * 0.3f, ZoneRadius * 0.3f),
            FMath::FRandRange(-ZoneRadius * 0.3f, ZoneRadius * 0.3f), 0.0f);
    }
    else
    {
        MoveTarget = GetPawn()->GetActorLocation() + Direction * 500.0f;
    }

    AAIController::MoveToLocation(MoveTarget, 200.0f);
}

FVector ASBBotController::FindCoverLocation(const FVector& ThreatDirection) const
{
    FVector Perpendicular = FVector(-ThreatDirection.Y, ThreatDirection.X, 0.0f);
    float Side = FMath::RandBool() ? 1.0f : -1.0f;
    return GetPawn()->GetActorLocation() + Perpendicular * Side * 1500.0f - ThreatDirection * 500.0f;
}

FVector ASBBotController::FindLootLocation() const
{
    // Simple: move toward a random nearby point
    return GetPawn()->GetActorLocation() +
        FVector(FMath::FRandRange(-3000.0f, 3000.0f), FMath::FRandRange(-3000.0f, 3000.0f), 0.0f);
}

// ============================================================================
// Perception
// ============================================================================

void ASBBotController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    AActor* BestTarget = nullptr;
    float BestDistance = DifficultySettings.EngageDistance;

    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor || Actor == GetPawn()) continue;

        APawn* OtherPawn = Cast<APawn>(Actor);
        if (!OtherPawn) continue;

        // Don't target teammates
        ASBPlayerState* MyPS = GetPawn()->GetPlayerState<ASBPlayerState>();
        ASBPlayerState* OtherPS = OtherPawn->GetPlayerState<ASBPlayerState>();
        if (MyPS && OtherPS && MyPS->GetTeamId() >= 0 && MyPS->GetTeamId() == OtherPS->GetTeamId())
            continue;

        FActorPerceptionBlueprintInfo Info;
        PerceptionComp->GetActorsPerception(Actor, Info);

        for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
        {
            if (!Stimulus.WasSuccessfullySensed()) continue;

            if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
            {
                Memory.LastSeenEnemy = Actor;
                Memory.LastKnownEnemyLocation = Actor->GetActorLocation();
                Memory.TimeSinceLastSeen = 0.0f;
            }
            else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
            {
                Memory.LastHeardLocation = Stimulus.StimulusLocation;
                Memory.TimeSinceLastHeard = 0.0f;
            }
            else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
            {
                Memory.LastDamageDirection = (Stimulus.StimulusLocation - GetPawn()->GetActorLocation()).GetSafeNormal();
                Memory.TimeSinceLastDamage = 0.0f;
                Memory.LastKnownEnemyLocation = Stimulus.StimulusLocation;
            }
        }

        float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
        if (Dist < BestDistance)
        {
            BestDistance = Dist;
            BestTarget = Actor;
        }
    }

    if (BestTarget)
    {
        CurrentTarget = BestTarget;
    }
}

// ============================================================================
// Helpers
// ============================================================================

ASBCharacterBase* ASBBotController::GetBotCharacter() const
{
    return Cast<ASBCharacterBase>(GetPawn());
}

float ASBBotController::GetDistanceToNearestPlayer() const
{
    float MinDist = 999999.0f;
    for (TActorIterator<APawn> It(GetWorld()); It; ++It)
    {
        APawn* OtherPawn = *It;
        if (!OtherPawn || OtherPawn == GetPawn() || !OtherPawn->IsPlayerControlled()) continue;
        float Dist = FVector::Dist(GetPawn()->GetActorLocation(), OtherPawn->GetActorLocation());
        MinDist = FMath::Min(MinDist, Dist);
    }
    return MinDist;
}

bool ASBBotController::IsInSafeZone() const
{
    if (!CachedZoneManager.IsValid()) return true;
    return CachedZoneManager->IsLocationInSafeZone(GetPawn()->GetActorLocation());
}

bool ASBBotController::NeedsHealing() const
{
    return GetHealthPercent() < DifficultySettings.HealThreshold;
}

bool ASBBotController::NeedsLoot() const
{
    return !HasWeapon();
}

bool ASBBotController::HasWeapon() const
{
    ASBCharacterBase* BotChar = GetBotCharacter();
    if (!BotChar || !BotChar->WeaponComponent) return false;
    return BotChar->WeaponComponent->GetActiveWeapon() != nullptr;
}

float ASBBotController::GetHealthPercent() const
{
    ASBCharacterBase* BotChar = GetBotCharacter();
    if (!BotChar) return 100.0f;

    ASBPlayerState* PS = BotChar->GetPlayerState<ASBPlayerState>();
    if (!PS || !PS->AttributeSet) return 100.0f;

    float MaxHP = PS->AttributeSet->GetMaxHealth();
    return MaxHP > 0.0f ? (PS->AttributeSet->GetHealth() / MaxHP) * 100.0f : 0.0f;
}
