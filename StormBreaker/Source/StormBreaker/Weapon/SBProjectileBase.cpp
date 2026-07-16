// Copyright StormBreaker Games. All Rights Reserved.

#include "Weapon/SBProjectileBase.h"
#include "StormBreaker.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

ASBProjectileBase::ASBProjectileBase()
{
    bReplicates = true;
    SetReplicatingMovement(true);
    InitialLifeSpan = 5.0f;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    CollisionComp->InitSphereRadius(5.0f);
    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
    CollisionComp->SetGenerateOverlapEvents(false);
    RootComponent = CollisionComp;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 30000.0f;
    ProjectileMovement->MaxSpeed = 30000.0f;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    Damage = 0.0f;
}

void ASBProjectileBase::InitProjectile(float InDamage, APawn* InInstigator)
{
    Damage = InDamage;
    OwnerInstigator = InInstigator;

    if (InInstigator)
    {
        SetInstigator(InInstigator);
        SetOwner(InInstigator);
    }
}

void ASBProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    CollisionComp->OnComponentHit.AddDynamic(this, &ASBProjectileBase::OnHit);

    // Spawn trail VFX
    if (TrailVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            TrailVFX, CollisionComp, NAME_None,
            FVector::ZeroVector, FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset, true);
    }
}

void ASBProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HasAuthority()) return;

    if (ExplosionRadius > 0.0f)
    {
        ApplyRadialDamage(Hit.ImpactPoint);
    }
    else if (OtherActor)
    {
        ApplyDirectDamage(OtherActor, Hit);
    }

    PlayImpactEffects(Hit.ImpactPoint, Hit.ImpactNormal);
    Destroy();
}

void ASBProjectileBase::ApplyDirectDamage(AActor* HitActor, const FHitResult& Hit)
{
    if (!HitActor) return;

    FPointDamageEvent DamageEvent;
    DamageEvent.Damage = Damage;
    DamageEvent.HitInfo = Hit;
    DamageEvent.ShotDirection = GetVelocity().GetSafeNormal();

    AController* InstigatorController = OwnerInstigator ?
        OwnerInstigator->GetController() : nullptr;

    HitActor->TakeDamage(Damage, DamageEvent, InstigatorController, this);
}

void ASBProjectileBase::ApplyRadialDamage(const FVector& ExplosionCenter)
{
    float DmgToApply = (ExplosionDamage > 0.0f) ? ExplosionDamage : Damage;

    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);

    AController* InstigatorController = OwnerInstigator ?
        OwnerInstigator->GetController() : nullptr;

    UGameplayStatics::ApplyRadialDamageWithFalloff(
        this,
        DmgToApply,
        DmgToApply * 0.1f,
        ExplosionCenter,
        ExplosionRadius * 0.3f,
        ExplosionRadius,
        1.0f,
        nullptr,
        IgnoreActors,
        this,
        InstigatorController,
        ECC_Pawn);
}

void ASBProjectileBase::PlayImpactEffects(const FVector& Location, const FVector& Normal)
{
    if (ImpactVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, ImpactVFX, Location, Normal.Rotation());
    }

    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Location);
    }
}
