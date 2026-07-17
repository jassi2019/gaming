// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;

/**
 * Base projectile actor for non-hitscan weapons and grenades.
 * Uses UProjectileMovementComponent for physics-based trajectory with gravity.
 * Server authoritative — spawned on server, replicated to clients.
 */
UCLASS()
class STORMBREAKER_API ASBProjectileBase : public AActor
{
    GENERATED_BODY()

public:
    ASBProjectileBase();

    void InitProjectile(float InDamage, APawn* InInstigator);

    // --- Components ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USphereComponent> CollisionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> MeshComp;

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float ExplosionRadius = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float ExplosionDamage = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    TObjectPtr<UNiagaraSystem> TrailVFX;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    TObjectPtr<UNiagaraSystem> ImpactVFX;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    TObjectPtr<USoundBase> ImpactSound;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
    void ApplyDirectDamage(AActor* HitActor, const FHitResult& Hit);
    void ApplyRadialDamage(const FVector& ExplosionCenter);
    void PlayImpactEffects(const FVector& Location, const FVector& Normal);

    float Damage;

    UPROPERTY()
    TObjectPtr<APawn> OwnerInstigator;
};
