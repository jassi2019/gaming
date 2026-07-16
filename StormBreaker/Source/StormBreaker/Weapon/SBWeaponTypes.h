// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SBWeaponTypes.generated.h"

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class ESBWeaponType : uint8
{
    AssaultRifle,
    SMG,
    Sniper,
    Shotgun,
    Pistol,
    Grenade,
    Melee
};

UENUM(BlueprintType)
enum class ESBFireMode : uint8
{
    Single,
    Burst,
    Auto
};

UENUM(BlueprintType)
enum class ESBDamageType : uint8
{
    Hitscan,
    Projectile
};

UENUM(BlueprintType)
enum class ESBWeaponSlot : uint8
{
    Primary   = 0,
    Secondary = 1,
    Sidearm   = 2,
    Melee     = 3,
    Throwable = 4,
    MAX       = 5  UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESBWeaponState : uint8
{
    Idle,
    Firing,
    Reloading,
    Equipping,
    Unequipping,
    Inspecting
};

UENUM(BlueprintType)
enum class ESBAmmoType : uint8
{
    Rifle_556,
    Rifle_762,
    SMG_9mm,
    Sniper_300,
    Shotgun_12g,
    Pistol_45,
    Grenade_Frag,
    None
};

UENUM(BlueprintType)
enum class ESBSurfaceType : uint8
{
    Default,
    Metal,
    Wood,
    Concrete,
    Dirt,
    Flesh,
    Glass,
    Water
};

// ============================================================================
// Structs
// ============================================================================

USTRUCT(BlueprintType)
struct FSBRecoilData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VerticalMin = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VerticalMax = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HorizontalMin = -0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HorizontalMax = 0.2f;

    // Multiplier applied when aiming down sights
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ADSMultiplier = 0.5f;

    // Multiplier applied when crouching
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CrouchMultiplier = 0.7f;

    // Multiplier applied when prone
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProneMultiplier = 0.4f;

    // How fast recoil recovers (degrees per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoverySpeed = 5.0f;

    // Recoil pattern: array of (pitch, yaw) offsets per shot, looped
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector2D> Pattern;
};

USTRUCT(BlueprintType)
struct FSBSpreadData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HipFireSpread = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ADSSpread = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpreadIncreasePerShot = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpread = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpreadRecoveryRate = 3.0f;

    // Multiplier when moving
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovingMultiplier = 1.5f;

    // Multiplier when jumping/in air
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AirborneMultiplier = 3.0f;
};

USTRUCT(BlueprintType)
struct FSBDamageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDamage = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeadshotMultiplier = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LimbMultiplier = 0.75f;

    // Damage falloff: no falloff within this range
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectiveRange = 5000.0f;

    // Damage drops to MinDamageMultiplier at this range
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxRange = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinDamageMultiplier = 0.3f;

    // Number of surfaces a bullet can penetrate (0 = none)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PenetrationCount = 0;

    // Damage multiplier after each penetration
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PenetrationDamageMultiplier = 0.5f;
};

USTRUCT(BlueprintType)
struct FSBProjectileData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InitialSpeed = 30000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 30000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LifeSpan = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CollisionRadius = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ProjectileClass;
};

USTRUCT(BlueprintType)
struct FSBAttachmentModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AttachmentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoilMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpreadMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RangeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ADSSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReloadSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MagazineSizeBonus = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UStaticMesh> AttachmentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AttachSocketName;
};

// Hit result sent from server to clients for visual effects
USTRUCT()
struct FSBHitResult
{
    GENERATED_BODY()

    UPROPERTY()
    FVector_NetQuantize ImpactPoint;

    UPROPERTY()
    FVector_NetQuantizeNormal ImpactNormal;

    UPROPERTY()
    uint8 SurfaceType;

    UPROPERTY()
    uint8 bIsHeadshot : 1;

    FSBHitResult()
        : SurfaceType(0)
        , bIsHeadshot(false)
    {}
};

// Compact fire event replicated to all clients
USTRUCT()
struct FSBFireEvent
{
    GENERATED_BODY()

    UPROPERTY()
    FVector_NetQuantize Origin;

    UPROPERTY()
    FVector_NetQuantizeNormal Direction;

    UPROPERTY()
    uint8 ShotIndex;

    UPROPERTY()
    float Timestamp;

    FSBFireEvent()
        : ShotIndex(0)
        , Timestamp(0.0f)
    {}
};
