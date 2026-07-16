// Copyright StormBreaker Games. All Rights Reserved.

#include "Weapon/SBWeaponBase.h"
#include "Weapon/SBWeaponDataAsset.h"
#include "Weapon/SBProjectileBase.h"
#include "Character/SBCharacterBase.h"
#include "Character/SBCharacterMovementComponent.h"
#include "Core/SBPlayerState.h"
#include "StormBreaker.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

ASBWeaponBase::ASBWeaponBase()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(false);

    WeaponMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMeshComp;
    WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMeshComp->CastShadow = true;

    WeaponState = ESBWeaponState::Idle;
    CurrentFireMode = ESBFireMode::Auto;
    CurrentMagazineAmmo = 0;
    CurrentReserveAmmo = 0;
    bWantsToFire = false;
    LastFireTime = 0.0f;
    CurrentSpreadAccumulator = 0.0f;
    BurstShotsRemaining = 0;
    ShotsFiredInBurst = 0;
    ShotIndex = 0;
    AccumulatedRecoilPitch = 0.0f;
    AccumulatedRecoilYaw = 0.0f;
    RecoilPatternIndex = 0;

    AttachDamageMul = 1.0f;
    AttachRecoilMul = 1.0f;
    AttachSpreadMul = 1.0f;
    AttachRangeMul = 1.0f;
    AttachReloadMul = 1.0f;
    AttachMagBonus = 0;
}

void ASBWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    if (WeaponData)
    {
        InitializeWeapon(WeaponData);
    }
}

void ASBWeaponBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    RecoverRecoil(DeltaTime);
    RecoverSpread(DeltaTime);
}

void ASBWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASBWeaponBase, WeaponState);
    DOREPLIFETIME(ASBWeaponBase, CurrentFireMode);
    DOREPLIFETIME_CONDITION(ASBWeaponBase, CurrentMagazineAmmo, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ASBWeaponBase, CurrentReserveAmmo, COND_OwnerOnly);
}

// ============================================================================
// Initialization
// ============================================================================

void ASBWeaponBase::InitializeWeapon(USBWeaponDataAsset* InData)
{
    if (!InData) return;
    WeaponData = InData;

    if (WeaponData->WeaponMesh)
    {
        WeaponMeshComp->SetSkeletalMeshAsset(WeaponData->WeaponMesh);
    }

    CurrentFireMode = WeaponData->DefaultFireMode;
    CurrentMagazineAmmo = WeaponData->MagazineSize;
    CurrentReserveAmmo = WeaponData->MaxReserveAmmo;

    UE_LOG(LogSBWeapon, Log, TEXT("Weapon '%s' initialized. Mag: %d, Reserve: %d"),
        *WeaponData->DisplayName.ToString(), CurrentMagazineAmmo, CurrentReserveAmmo);
}

// ============================================================================
// State Queries
// ============================================================================

bool ASBWeaponBase::CanFire() const
{
    if (!WeaponData) return false;
    if (WeaponState != ESBWeaponState::Idle && WeaponState != ESBWeaponState::Firing) return false;

    if (WeaponData->WeaponType == ESBWeaponType::Melee) return true;

    float TimeBetweenShots = WeaponData->GetTimeBetweenShots();
    float TimeSinceLastFire = GetWorld()->GetTimeSeconds() - LastFireTime;
    if (TimeSinceLastFire < TimeBetweenShots) return false;

    return CurrentMagazineAmmo > 0;
}

bool ASBWeaponBase::CanReload() const
{
    if (!WeaponData) return false;
    if (WeaponState == ESBWeaponState::Reloading) return false;
    if (WeaponState == ESBWeaponState::Equipping || WeaponState == ESBWeaponState::Unequipping) return false;
    if (WeaponData->WeaponType == ESBWeaponType::Melee) return false;
    if (WeaponData->WeaponType == ESBWeaponType::Grenade) return false;
    if (CurrentMagazineAmmo >= WeaponData->MagazineSize + AttachMagBonus) return false;
    if (CurrentReserveAmmo <= 0) return false;

    return true;
}

float ASBWeaponBase::GetCurrentSpread() const
{
    if (!WeaponData) return 0.0f;

    bool bAiming = OwningCharacter ? OwningCharacter->IsAiming() : false;
    float BaseSpread = bAiming ? WeaponData->Spread.ADSSpread : WeaponData->Spread.HipFireSpread;
    float TotalSpread = (BaseSpread + CurrentSpreadAccumulator) * AttachSpreadMul;

    if (OwningCharacter)
    {
        USBCharacterMovementComponent* CMC = OwningCharacter->GetSBMovement();
        if (CMC)
        {
            if (CMC->Velocity.Size2D() > 10.0f)
            {
                TotalSpread *= WeaponData->Spread.MovingMultiplier;
            }
            if (CMC->IsFalling())
            {
                TotalSpread *= WeaponData->Spread.AirborneMultiplier;
            }
            if (CMC->IsCrouching())
            {
                TotalSpread *= WeaponData->Recoil.CrouchMultiplier;
            }
            if (CMC->IsProning())
            {
                TotalSpread *= WeaponData->Recoil.ProneMultiplier;
            }
        }
    }

    return FMath::Min(TotalSpread, WeaponData->Spread.MaxSpread);
}

// ============================================================================
// Actions
// ============================================================================

void ASBWeaponBase::StartFire()
{
    bWantsToFire = true;

    if (CurrentMagazineAmmo <= 0 && WeaponData && WeaponData->WeaponType != ESBWeaponType::Melee)
    {
        // Dry fire
        if (WeaponData->DryFireSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, WeaponData->DryFireSound, GetActorLocation());
        }
        // Auto-reload
        Reload();
        return;
    }

    if (!CanFire()) return;

    switch (CurrentFireMode)
    {
    case ESBFireMode::Single:
        FireShot();
        break;

    case ESBFireMode::Burst:
        BurstShotsRemaining = WeaponData ? WeaponData->BurstCount : 3;
        ShotsFiredInBurst = 0;
        FireShot();
        break;

    case ESBFireMode::Auto:
    {
        FireShot();
        float TimeBetweenShots = WeaponData ? WeaponData->GetTimeBetweenShots() : 0.1f;
        GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this,
            &ASBWeaponBase::AutoFireTick, TimeBetweenShots, true);
        break;
    }
    }
}

void ASBWeaponBase::StopFire()
{
    bWantsToFire = false;
    GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
    GetWorldTimerManager().ClearTimer(BurstTimerHandle);
    BurstShotsRemaining = 0;
    ShotsFiredInBurst = 0;
}

void ASBWeaponBase::AutoFireTick()
{
    if (!bWantsToFire || !CanFire())
    {
        StopFire();
        if (CurrentMagazineAmmo <= 0)
        {
            Reload();
        }
        return;
    }

    FireShot();
}

void ASBWeaponBase::FireShot()
{
    if (!CanFire() || !WeaponData) return;

    FVector MuzzleLoc = GetMuzzleLocation();
    FVector AimDir = GetAimDirection();

    // Client: play local effects immediately
    if (OwningCharacter && OwningCharacter->IsLocallyControlled())
    {
        ApplyRecoil();
        Multicast_PlayFireEffects(MuzzleLoc, AimDir);
    }

    // Send to server for authoritative hit detection
    float ClientTime = GetWorld()->GetTimeSeconds();
    Server_Fire(MuzzleLoc, AimDir, ClientTime);

    LastFireTime = GetWorld()->GetTimeSeconds();
    CurrentSpreadAccumulator = FMath::Min(
        CurrentSpreadAccumulator + WeaponData->Spread.SpreadIncreasePerShot,
        WeaponData->Spread.MaxSpread);

    SetWeaponState(ESBWeaponState::Firing);
    ShotIndex++;

    // Burst logic
    if (CurrentFireMode == ESBFireMode::Burst)
    {
        ShotsFiredInBurst++;
        BurstShotsRemaining--;
        if (BurstShotsRemaining > 0 && CanFire())
        {
            float BurstDelay = WeaponData->GetTimeBetweenShots();
            GetWorldTimerManager().SetTimer(BurstTimerHandle, this,
                &ASBWeaponBase::FireShot, BurstDelay, false);
        }
        else
        {
            BurstShotsRemaining = 0;
        }
    }
}

void ASBWeaponBase::Reload()
{
    if (!CanReload()) return;

    Server_Reload();

    SetWeaponState(ESBWeaponState::Reloading);
    StopFire();

    // Play reload montage
    if (OwningCharacter && WeaponData)
    {
        UAnimMontage* Montage = (CurrentMagazineAmmo == 0 && WeaponData->ReloadEmptyMontage)
            ? WeaponData->ReloadEmptyMontage
            : WeaponData->ReloadMontage;

        if (Montage)
        {
            OwningCharacter->PlayAnimMontage(Montage);
        }
    }

    float ReloadDuration = WeaponData ?
        ((CurrentMagazineAmmo == 0) ? WeaponData->ReloadTimeEmpty : WeaponData->ReloadTime) : 2.0f;
    ReloadDuration *= AttachReloadMul;

    GetWorldTimerManager().SetTimer(ReloadTimerHandle, this,
        &ASBWeaponBase::FinishReload, ReloadDuration, false);
}

void ASBWeaponBase::FinishReload()
{
    if (!WeaponData) return;

    if (HasAuthority())
    {
        int32 MaxMag = WeaponData->MagazineSize + AttachMagBonus;
        int32 AmmoNeeded = MaxMag - CurrentMagazineAmmo;
        int32 AmmoToLoad = FMath::Min(AmmoNeeded, CurrentReserveAmmo);
        CurrentMagazineAmmo += AmmoToLoad;
        CurrentReserveAmmo -= AmmoToLoad;
        OnAmmoChanged.Broadcast(CurrentMagazineAmmo, CurrentReserveAmmo);
    }

    SetWeaponState(ESBWeaponState::Idle);
    UE_LOG(LogSBWeapon, Verbose, TEXT("Reload complete. Mag: %d, Reserve: %d"),
        CurrentMagazineAmmo, CurrentReserveAmmo);
}

void ASBWeaponBase::CycleFireMode()
{
    if (!WeaponData || WeaponData->AvailableFireModes.Num() <= 1) return;

    Server_CycleFireMode();
}

void ASBWeaponBase::Equip(ASBCharacterBase* NewOwner)
{
    OwningCharacter = NewOwner;
    SetOwner(NewOwner);
    SetWeaponState(ESBWeaponState::Equipping);

    if (WeaponData && WeaponData->EquipSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, WeaponData->EquipSound, GetActorLocation());
    }

    if (OwningCharacter && WeaponData && WeaponData->EquipMontage)
    {
        OwningCharacter->PlayAnimMontage(WeaponData->EquipMontage);
    }

    float Duration = WeaponData ? WeaponData->EquipTime : 0.6f;
    GetWorldTimerManager().SetTimer(EquipTimerHandle, this,
        &ASBWeaponBase::FinishEquip, Duration, false);
}

void ASBWeaponBase::FinishEquip()
{
    SetWeaponState(ESBWeaponState::Idle);
}

void ASBWeaponBase::Unequip()
{
    StopFire();
    SetWeaponState(ESBWeaponState::Unequipping);

    if (OwningCharacter && WeaponData && WeaponData->UnequipMontage)
    {
        OwningCharacter->PlayAnimMontage(WeaponData->UnequipMontage);
    }

    float Duration = WeaponData ? WeaponData->UnequipTime : 0.4f;
    GetWorldTimerManager().SetTimer(UnequipTimerHandle, this,
        &ASBWeaponBase::FinishUnequip, Duration, false);
}

void ASBWeaponBase::FinishUnequip()
{
    SetWeaponState(ESBWeaponState::Idle);
}

void ASBWeaponBase::Inspect()
{
    if (WeaponState != ESBWeaponState::Idle) return;
    SetWeaponState(ESBWeaponState::Inspecting);

    if (OwningCharacter && WeaponData && WeaponData->InspectMontage)
    {
        float Duration = OwningCharacter->PlayAnimMontage(WeaponData->InspectMontage);
        FTimerHandle InspectTimer;
        GetWorldTimerManager().SetTimer(InspectTimer, [this]()
        {
            SetWeaponState(ESBWeaponState::Idle);
        }, Duration, false);
    }
    else
    {
        SetWeaponState(ESBWeaponState::Idle);
    }
}

void ASBWeaponBase::AddAmmo(int32 Amount)
{
    if (!HasAuthority() || !WeaponData) return;
    CurrentReserveAmmo = FMath::Min(CurrentReserveAmmo + Amount, WeaponData->MaxReserveAmmo);
    OnAmmoChanged.Broadcast(CurrentMagazineAmmo, CurrentReserveAmmo);
}

void ASBWeaponBase::SetAmmo(int32 MagAmmo, int32 ReserveAmmo)
{
    CurrentMagazineAmmo = MagAmmo;
    CurrentReserveAmmo = ReserveAmmo;
    OnAmmoChanged.Broadcast(CurrentMagazineAmmo, CurrentReserveAmmo);
}

// ============================================================================
// Server RPCs
// ============================================================================

bool ASBWeaponBase::Server_Fire_Validate(FVector Origin, FVector Direction, float ClientTimestamp)
{
    // Anti-cheat: validate fire rate
    float TimeBetweenShots = WeaponData ? WeaponData->GetTimeBetweenShots() : 0.1f;
    float ServerTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLast = ServerTime - LastFireTime;

    // Allow small tolerance for network jitter
    if (TimeSinceLast < TimeBetweenShots * 0.8f)
    {
        UE_LOG(LogSBWeapon, Warning, TEXT("Fire rate violation from %s. Delta: %.3f, Min: %.3f"),
            *GetNameSafe(GetOwner()), TimeSinceLast, TimeBetweenShots);
        return false;
    }

    // Validate ammo
    if (CurrentMagazineAmmo <= 0 && WeaponData && WeaponData->WeaponType != ESBWeaponType::Melee)
    {
        return false;
    }

    return true;
}

void ASBWeaponBase::Server_Fire_Implementation(FVector Origin, FVector Direction, float ClientTimestamp)
{
    if (!WeaponData) return;

    // Consume ammo on server
    if (WeaponData->WeaponType != ESBWeaponType::Melee)
    {
        CurrentMagazineAmmo = FMath::Max(0, CurrentMagazineAmmo - 1);
    }

    LastFireTime = GetWorld()->GetTimeSeconds();
    OnAmmoChanged.Broadcast(CurrentMagazineAmmo, CurrentReserveAmmo);

    switch (WeaponData->WeaponType)
    {
    case ESBWeaponType::Melee:
        FireMelee();
        break;

    case ESBWeaponType::Grenade:
        FireGrenade();
        break;

    default:
        if (WeaponData->DamageMethod == ESBDamageType::Hitscan)
        {
            for (int32 i = 0; i < WeaponData->PelletsPerShot; i++)
            {
                FVector SpreadDir = CalculateSpreadDirection(Direction);
                FireHitscan(Origin, SpreadDir);
            }
        }
        else
        {
            FVector SpreadDir = CalculateSpreadDirection(Direction);
            FireProjectile(Origin, SpreadDir);
        }
        break;
    }

    // Multicast fire effects to other clients (skip owner who already played locally)
    Multicast_PlayFireEffects(Origin, Direction);
}

void ASBWeaponBase::Server_Reload_Implementation()
{
    // Server-side reload validation already done by CanReload in client call
    UE_LOG(LogSBWeapon, Verbose, TEXT("Server processing reload for %s"), *GetNameSafe(GetOwner()));
}

void ASBWeaponBase::Server_CycleFireMode_Implementation()
{
    if (!WeaponData || WeaponData->AvailableFireModes.Num() <= 1) return;

    int32 CurrentIndex = WeaponData->AvailableFireModes.IndexOfByKey(CurrentFireMode);
    int32 NextIndex = (CurrentIndex + 1) % WeaponData->AvailableFireModes.Num();
    CurrentFireMode = WeaponData->AvailableFireModes[NextIndex];
}

// ============================================================================
// Combat - Hitscan
// ============================================================================

void ASBWeaponBase::FireHitscan(const FVector& MuzzleLocation, const FVector& AimDirection)
{
    if (!WeaponData) return;

    float MaxRange = WeaponData->Damage.MaxRange * AttachRangeMul;
    FVector TraceEnd = MuzzleLocation + AimDirection * MaxRange;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (OwningCharacter) Params.AddIgnoredActor(OwningCharacter);
    Params.bTraceComplex = true;
    Params.bReturnPhysicalMaterial = true;

    TArray<FSBHitResult> HitResults;

    int32 PenRemaining = WeaponData->Damage.PenetrationCount;
    float DamageMultiplier = 1.0f;
    FVector CurrentStart = MuzzleLocation;

    // Trace with penetration support
    for (int32 i = 0; i <= PenRemaining; i++)
    {
        FHitResult Hit;
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit, CurrentStart, TraceEnd, ECC_Bullet, Params);

        if (!bHit) break;

        ProcessHitscanHit(Hit, AimDirection, DamageMultiplier);

        // Collect hit for visual effects
        FSBHitResult VisualHit;
        VisualHit.ImpactPoint = Hit.ImpactPoint;
        VisualHit.ImpactNormal = Hit.ImpactNormal;
        VisualHit.bIsHeadshot = (Hit.BoneName == FName("head"));

        // Surface type from physical material
        if (Hit.PhysMaterial.IsValid())
        {
            VisualHit.SurfaceType = static_cast<uint8>(Hit.PhysMaterial->SurfaceType.GetValue());
        }
        HitResults.Add(VisualHit);

        // Setup for next penetration trace
        Params.AddIgnoredActor(Hit.GetActor());
        CurrentStart = Hit.ImpactPoint + AimDirection * 10.0f;
        DamageMultiplier *= WeaponData->Damage.PenetrationDamageMultiplier;
    }

    if (HitResults.Num() > 0)
    {
        Multicast_PlayImpactEffects(HitResults);
    }
}

void ASBWeaponBase::ProcessHitscanHit(const FHitResult& Hit, const FVector& ShotDirection, float DamageMultiplier)
{
    AActor* HitActor = Hit.GetActor();
    if (!HitActor || !WeaponData) return;

    APawn* HitPawn = Cast<APawn>(HitActor);
    if (!HitPawn) return;

    float Distance = FVector::Dist(GetMuzzleLocation(), Hit.ImpactPoint);
    float BaseDamage = WeaponData->Damage.BaseDamage * AttachDamageMul * DamageMultiplier;
    float FinalDamage = BaseDamage * CalculateDamageAtDistance(Distance);

    // Headshot check
    bool bHeadshot = (Hit.BoneName == FName("head"));
    if (bHeadshot)
    {
        FinalDamage *= WeaponData->Damage.HeadshotMultiplier;
    }

    // Limb check
    static const TArray<FName> LimbBones = {
        FName("hand_l"), FName("hand_r"),
        FName("foot_l"), FName("foot_r"),
        FName("calf_l"), FName("calf_r"),
        FName("lowerarm_l"), FName("lowerarm_r")
    };
    if (LimbBones.Contains(Hit.BoneName))
    {
        FinalDamage *= WeaponData->Damage.LimbMultiplier;
    }

    // Apply damage via UE damage system (integrates with GAS in receiving character)
    FPointDamageEvent DamageEvent;
    DamageEvent.Damage = FinalDamage;
    DamageEvent.HitInfo = Hit;
    DamageEvent.ShotDirection = ShotDirection;

    AController* InstigatorController = OwningCharacter ? OwningCharacter->GetController() : nullptr;
    HitActor->TakeDamage(FinalDamage, DamageEvent, InstigatorController, this);

    UE_LOG(LogSBWeapon, Verbose, TEXT("Hit %s for %.1f damage (headshot: %s, dist: %.0f)"),
        *GetNameSafe(HitActor), FinalDamage, bHeadshot ? TEXT("YES") : TEXT("NO"), Distance);
}

// ============================================================================
// Combat - Projectile
// ============================================================================

void ASBWeaponBase::FireProjectile(const FVector& MuzzleLocation, const FVector& AimDirection)
{
    if (!WeaponData || !WeaponData->Projectile.ProjectileClass) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = OwningCharacter;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    FRotator SpawnRotation = AimDirection.Rotation();
    AActor* Proj = GetWorld()->SpawnActor<AActor>(
        WeaponData->Projectile.ProjectileClass, MuzzleLocation, SpawnRotation, SpawnParams);

    if (ASBProjectileBase* SBProj = Cast<ASBProjectileBase>(Proj))
    {
        SBProj->InitProjectile(WeaponData->Damage.BaseDamage * AttachDamageMul, OwningCharacter);
    }
}

// ============================================================================
// Combat - Melee
// ============================================================================

void ASBWeaponBase::FireMelee()
{
    if (!WeaponData || !OwningCharacter) return;

    FVector Origin = OwningCharacter->GetActorLocation();
    FVector Forward = OwningCharacter->GetActorForwardVector();
    float Range = WeaponData->MeleeRange;
    float HalfAngleRad = FMath::DegreesToRadians(WeaponData->MeleeAngle * 0.5f);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(OwningCharacter);

    // Sphere sweep in front of character
    TArray<FHitResult> Hits;
    FCollisionShape Shape = FCollisionShape::MakeSphere(Range);
    GetWorld()->SweepMultiByChannel(Hits, Origin, Origin + Forward * 10.0f,
        FQuat::Identity, ECC_Pawn, Shape, Params);

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor) continue;

        // Check angle
        FVector ToTarget = (HitActor->GetActorLocation() - Origin).GetSafeNormal();
        float DotProduct = FVector::DotProduct(Forward, ToTarget);
        if (DotProduct < FMath::Cos(HalfAngleRad)) continue;

        float MeleeDamage = WeaponData->Damage.BaseDamage * AttachDamageMul;
        FPointDamageEvent DamageEvent;
        DamageEvent.Damage = MeleeDamage;
        DamageEvent.HitInfo = Hit;
        DamageEvent.ShotDirection = Forward;

        AController* InstigatorController = OwningCharacter->GetController();
        HitActor->TakeDamage(MeleeDamage, DamageEvent, InstigatorController, this);
    }

    if (WeaponData->MeleeMontage && OwningCharacter)
    {
        OwningCharacter->PlayAnimMontage(WeaponData->MeleeMontage);
    }
}

// ============================================================================
// Combat - Grenade
// ============================================================================

void ASBWeaponBase::FireGrenade()
{
    if (!WeaponData || !OwningCharacter) return;

    if (WeaponData->Projectile.ProjectileClass)
    {
        FVector SpawnLoc = GetMuzzleLocation();
        FVector ThrowDir = GetAimDirection();
        FRotator SpawnRot = ThrowDir.Rotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = OwningCharacter;

        AActor* Grenade = GetWorld()->SpawnActor<AActor>(
            WeaponData->Projectile.ProjectileClass, SpawnLoc, SpawnRot, SpawnParams);

        if (ASBProjectileBase* SBProj = Cast<ASBProjectileBase>(Grenade))
        {
            SBProj->InitProjectile(WeaponData->ExplosionDamage, OwningCharacter);
        }
    }
}

// ============================================================================
// Spread & Recoil
// ============================================================================

FVector ASBWeaponBase::CalculateSpreadDirection(const FVector& BaseDirection) const
{
    float CurrentSpread = GetCurrentSpread();
    if (CurrentSpread <= 0.0f) return BaseDirection;

    float HalfAngleRad = FMath::DegreesToRadians(CurrentSpread * 0.5f);
    return FMath::VRandCone(BaseDirection, HalfAngleRad);
}

float ASBWeaponBase::CalculateDamageAtDistance(float Distance) const
{
    if (!WeaponData) return 1.0f;

    float EffRange = WeaponData->Damage.EffectiveRange * AttachRangeMul;
    float MaxRange = WeaponData->Damage.MaxRange * AttachRangeMul;

    if (Distance <= EffRange) return 1.0f;
    if (Distance >= MaxRange) return WeaponData->Damage.MinDamageMultiplier;

    float Alpha = (Distance - EffRange) / (MaxRange - EffRange);
    return FMath::Lerp(1.0f, WeaponData->Damage.MinDamageMultiplier, Alpha);
}

void ASBWeaponBase::ApplyRecoil()
{
    if (!OwningCharacter || !WeaponData) return;

    APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
    if (!PC) return;

    float VerticalRecoil, HorizontalRecoil;

    // Use pattern if available
    if (WeaponData->Recoil.Pattern.Num() > 0)
    {
        int32 PatternIdx = RecoilPatternIndex % WeaponData->Recoil.Pattern.Num();
        FVector2D PatternOffset = WeaponData->Recoil.Pattern[PatternIdx];
        VerticalRecoil = PatternOffset.X;
        HorizontalRecoil = PatternOffset.Y;
        RecoilPatternIndex++;
    }
    else
    {
        VerticalRecoil = FMath::FRandRange(WeaponData->Recoil.VerticalMin, WeaponData->Recoil.VerticalMax);
        HorizontalRecoil = FMath::FRandRange(WeaponData->Recoil.HorizontalMin, WeaponData->Recoil.HorizontalMax);
    }

    // Apply stance multipliers
    float Multiplier = AttachRecoilMul;
    if (OwningCharacter->IsAiming())
    {
        Multiplier *= WeaponData->Recoil.ADSMultiplier;
    }

    USBCharacterMovementComponent* CMC = OwningCharacter->GetSBMovement();
    if (CMC)
    {
        if (CMC->IsCrouching()) Multiplier *= WeaponData->Recoil.CrouchMultiplier;
        if (CMC->IsProning()) Multiplier *= WeaponData->Recoil.ProneMultiplier;
    }

    VerticalRecoil *= Multiplier;
    HorizontalRecoil *= Multiplier;

    PC->AddPitchInput(-VerticalRecoil);
    PC->AddYawInput(HorizontalRecoil);

    AccumulatedRecoilPitch += VerticalRecoil;
    AccumulatedRecoilYaw += HorizontalRecoil;
}

void ASBWeaponBase::RecoverRecoil(float DeltaTime)
{
    if (!WeaponData) return;
    if (FMath::IsNearlyZero(AccumulatedRecoilPitch) && FMath::IsNearlyZero(AccumulatedRecoilYaw)) return;
    if (bWantsToFire) return;

    float RecoveryRate = WeaponData->Recoil.RecoverySpeed * DeltaTime;

    if (!FMath::IsNearlyZero(AccumulatedRecoilPitch))
    {
        float PitchRecovery = FMath::Min(RecoveryRate, FMath::Abs(AccumulatedRecoilPitch));
        float PitchSign = FMath::Sign(AccumulatedRecoilPitch);

        if (OwningCharacter)
        {
            APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
            if (PC) PC->AddPitchInput(PitchRecovery * PitchSign);
        }

        AccumulatedRecoilPitch -= PitchRecovery * PitchSign;
    }

    if (!FMath::IsNearlyZero(AccumulatedRecoilYaw))
    {
        float YawRecovery = FMath::Min(RecoveryRate, FMath::Abs(AccumulatedRecoilYaw));
        float YawSign = FMath::Sign(AccumulatedRecoilYaw);

        if (OwningCharacter)
        {
            APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
            if (PC) PC->AddYawInput(-YawRecovery * YawSign);
        }

        AccumulatedRecoilYaw -= YawRecovery * YawSign;
    }

    // Reset pattern index when not firing
    RecoilPatternIndex = 0;
}

void ASBWeaponBase::RecoverSpread(float DeltaTime)
{
    if (!WeaponData) return;
    if (CurrentSpreadAccumulator > 0.0f)
    {
        CurrentSpreadAccumulator = FMath::Max(0.0f,
            CurrentSpreadAccumulator - WeaponData->Spread.SpreadRecoveryRate * DeltaTime);
    }
}

// ============================================================================
// Aim Assist (Mobile)
// ============================================================================

FVector ASBWeaponBase::ApplyAimAssist(const FVector& AimDirection) const
{
    if (!WeaponData || !OwningCharacter) return AimDirection;
    if (WeaponData->AimAssistStrength <= 0.0f) return AimDirection;

    FVector Origin = GetMuzzleLocation();
    float MaxRange = WeaponData->AimAssistRange;
    float MaxAngle = FMath::DegreesToRadians(WeaponData->AimAssistAngle);

    AActor* BestTarget = nullptr;
    float BestDot = FMath::Cos(MaxAngle);

    for (TActorIterator<APawn> It(GetWorld()); It; ++It)
    {
        APawn* OtherPawn = *It;
        if (!OtherPawn || OtherPawn == OwningCharacter) continue;
        if (OtherPawn->IsPlayerControlled() == false) continue;

        FVector ToPawn = (OtherPawn->GetActorLocation() - Origin);
        float Distance = ToPawn.Size();
        if (Distance > MaxRange) continue;

        FVector DirToPawn = ToPawn.GetSafeNormal();
        float Dot = FVector::DotProduct(AimDirection, DirToPawn);
        if (Dot > BestDot)
        {
            BestDot = Dot;
            BestTarget = OtherPawn;
        }
    }

    if (BestTarget)
    {
        FVector ToTarget = (BestTarget->GetActorLocation() - Origin).GetSafeNormal();
        return FMath::Lerp(AimDirection, ToTarget, WeaponData->AimAssistStrength).GetSafeNormal();
    }

    return AimDirection;
}

// ============================================================================
// Helpers
// ============================================================================

FVector ASBWeaponBase::GetMuzzleLocation() const
{
    if (WeaponData && WeaponMeshComp && WeaponMeshComp->DoesSocketExist(WeaponData->MuzzleSocketName))
    {
        return WeaponMeshComp->GetSocketLocation(WeaponData->MuzzleSocketName);
    }

    return GetActorLocation() + GetActorForwardVector() * 50.0f;
}

FVector ASBWeaponBase::GetAimDirection() const
{
    if (OwningCharacter)
    {
        APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
        if (PC)
        {
            FVector CamLoc;
            FRotator CamRot;
            PC->GetPlayerViewPoint(CamLoc, CamRot);

#if PLATFORM_ANDROID || PLATFORM_IOS
            return ApplyAimAssist(CamRot.Vector());
#else
            return CamRot.Vector();
#endif
        }
    }

    return GetActorForwardVector();
}

void ASBWeaponBase::SetWeaponState(ESBWeaponState NewState)
{
    if (WeaponState == NewState) return;
    WeaponState = NewState;
    OnWeaponStateChanged.Broadcast(NewState);
}

void ASBWeaponBase::OnRep_WeaponState()
{
    OnWeaponStateChanged.Broadcast(WeaponState);
}

void ASBWeaponBase::OnRep_Ammo()
{
    OnAmmoChanged.Broadcast(CurrentMagazineAmmo, CurrentReserveAmmo);
}

// ============================================================================
// Attachments
// ============================================================================

void ASBWeaponBase::ApplyAttachment(const FSBAttachmentModifier& Attachment)
{
    ActiveAttachments.Add(Attachment);
    RecalculateAttachmentModifiers();

    if (Attachment.AttachmentMesh && WeaponMeshComp &&
        WeaponMeshComp->DoesSocketExist(Attachment.AttachSocketName))
    {
        UStaticMeshComponent* AttachMesh = NewObject<UStaticMeshComponent>(this);
        AttachMesh->SetStaticMesh(Attachment.AttachmentMesh);
        AttachMesh->AttachToComponent(WeaponMeshComp,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, Attachment.AttachSocketName);
        AttachMesh->RegisterComponent();
    }
}

void ASBWeaponBase::RemoveAttachment(const FName& AttachmentName)
{
    ActiveAttachments.RemoveAll([&](const FSBAttachmentModifier& A)
    {
        return A.AttachmentName == AttachmentName;
    });
    RecalculateAttachmentModifiers();
}

void ASBWeaponBase::RecalculateAttachmentModifiers()
{
    AttachDamageMul = 1.0f;
    AttachRecoilMul = 1.0f;
    AttachSpreadMul = 1.0f;
    AttachRangeMul = 1.0f;
    AttachReloadMul = 1.0f;
    AttachMagBonus = 0;

    for (const FSBAttachmentModifier& A : ActiveAttachments)
    {
        AttachDamageMul *= A.DamageMultiplier;
        AttachRecoilMul *= A.RecoilMultiplier;
        AttachSpreadMul *= A.SpreadMultiplier;
        AttachRangeMul *= A.RangeMultiplier;
        AttachReloadMul *= A.ReloadSpeedMultiplier;
        AttachMagBonus += A.MagazineSizeBonus;
    }
}

// ============================================================================
// Multicast Effects
// ============================================================================

void ASBWeaponBase::Multicast_PlayFireEffects_Implementation(FVector MuzzleLocation, FVector Direction)
{
    if (!WeaponData) return;

    // Skip for locally controlled owner (already played locally)
    if (OwningCharacter && OwningCharacter->IsLocallyControlled()) return;

    // Muzzle flash VFX
    if (WeaponData->MuzzleFlashVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, WeaponData->MuzzleFlashVFX, MuzzleLocation, Direction.Rotation());
    }

    // Fire sound
    if (WeaponData->FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FireSound, MuzzleLocation);
    }

    // Fire montage on third-person mesh
    if (OwningCharacter && WeaponData->FireMontage)
    {
        OwningCharacter->PlayAnimMontage(WeaponData->FireMontage);
    }
}

void ASBWeaponBase::Multicast_PlayImpactEffects_Implementation(const TArray<FSBHitResult>& Hits)
{
    for (const FSBHitResult& Hit : Hits)
    {
        // Impact VFX and decals will be spawned per surface type
        // Full implementation in Phase 8 (UI/VFX polish)
    }
}

void ASBWeaponBase::Multicast_PlayReloadEffects_Implementation()
{
    if (!WeaponData) return;

    if (WeaponData->ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, WeaponData->ReloadSound, GetActorLocation());
    }
}
