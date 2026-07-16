# StormBreaker — Coding Standards & Naming Conventions

## Naming Conventions

### C++ Classes
| Type | Prefix | Example |
|------|--------|---------|
| Actor | `ASB` | `ASBCharacterBase` |
| UObject / Component | `USB` | `USBWeaponComponent` |
| Interface | `ISB` | `ISBInteractable` |
| Struct | `FSB` | `FSBWeaponData` |
| Enum | `ESB` | `ESBWeaponType` |
| Delegate | `FOnSB` | `FOnSBHealthChanged` |
| Widget (UMG) | `USB` | `USBHealthBarWidget` |
| Subsystem | `USB` | `USBSettingsSubsystem` |
| Ability (GAS) | `USB` | `USBAbility_Fire` |
| GameplayEffect | `USB` | `USBEffect_Damage` |
| AttributeSet | `USB` | `USBAttributeSet` |

### Blueprints
| Type | Prefix | Example |
|------|--------|---------|
| Blueprint Actor | `BP_` | `BP_SBCharacter` |
| Widget Blueprint | `WBP_` | `WBP_HealthBar` |
| Animation Blueprint | `ABP_` | `ABP_SBCharacter` |
| Material | `M_` | `M_GunMetal` |
| Material Instance | `MI_` | `MI_GunMetal_Worn` |
| Texture | `T_` | `T_GunMetal_BaseColor` |
| Static Mesh | `SM_` | `SM_AK47_Body` |
| Skeletal Mesh | `SK_` | `SK_PlayerCharacter` |
| Niagara System | `NS_` | `NS_MuzzleFlash` |
| Niagara Emitter | `NE_` | `NE_BulletTrail` |
| Sound Cue | `SC_` | `SC_GunFire_AR` |
| Sound Wave | `SW_` | `SW_Footstep_Dirt_01` |
| Data Table | `DT_` | `DT_WeaponStats` |
| Data Asset | `DA_` | `DA_WeaponConfig_AK47` |
| Input Action | `IA_` | `IA_Fire` |
| Input Mapping | `IMC_` | `IMC_Default` |
| Montage | `AM_` | `AM_Reload_AR` |
| Blend Space | `BS_` | `BS_Locomotion` |
| Gameplay Cue | `GC_` | `GC_Impact_Bullet` |
| Gameplay Ability | `GA_` | `GA_Fire` |
| Gameplay Effect | `GE_` | `GE_Damage_Bullet` |

### Variables
```
bool bIsAiming;           // b prefix for booleans
int32 AmmoCount;          // PascalCase, no prefix
float SprintSpeed;        // PascalCase
FVector MuzzleLocation;   // UE types use F prefix already
TArray<AActor*> NearbyItems;  // T prefix built-in
```

### Functions
```
void FireWeapon();                    // Verb + Noun
bool CanFireWeapon() const;           // Can/Is/Has for queries
void Server_FireWeapon();             // Server_ prefix for server RPCs
void Client_OnHitConfirmed();         // Client_ prefix for client RPCs
void Multicast_PlayFireEffect();      // Multicast_ prefix for multicast RPCs
void OnWeaponFired();                 // On prefix for event handlers
```

## Code Organization

### Header File Order
```cpp
#pragma once

#include "CoreMinimal.h"                // 1. UE core
#include "GameFramework/Character.h"    // 2. Engine headers
#include "AbilitySystemInterface.h"     // 3. Plugin headers
#include "SBCharacterBase.generated.h"  // 4. Generated (always last include)

// Forward declarations before class body
class USBWeaponComponent;

UCLASS()
class STORMBREAKER_API ASBCharacterBase : public ACharacter
{
    GENERATED_BODY()

public:
    // Constructors
    // Engine overrides
    // Public API (BlueprintCallable)
    // Public properties (BlueprintReadOnly)

protected:
    // Protected overrides
    // Protected helpers
    // Protected properties (EditDefaultsOnly, BlueprintReadOnly)

private:
    // Private implementation
    // Private properties
};
```

### Source File Order
```cpp
#include "Character/SBCharacterBase.h"  // 1. Own header
#include "StormBreaker.h"               // 2. Module header
#include "Net/UnrealNetwork.h"          // 3. Engine
#include "Weapon/SBWeaponComponent.h"   // 4. Project headers
```

## Multiplayer Rules

1. **Never trust the client.** All gameplay-critical logic runs on the server.
2. **Use RPCs correctly:**
   - `Server` — client requests action from server
   - `Client` — server notifies specific client
   - `NetMulticast` — server broadcasts to all clients (cosmetic only)
3. **Replicated properties** for state, **RPCs** for events.
4. **Minimize bandwidth:** use `COND_OwnerOnly` for private data, `COND_SkipOwner` for third-person visuals.
5. **Net relevancy:** weapons, loot, vehicles must have proper `NetCullDistanceSquared`.

## SOLID Principles

- **S** — Each class has one job (WeaponComponent handles weapons, not inventory)
- **O** — Data-driven via DataTables/DataAssets, not hardcoded weapon stats
- **L** — All weapons derive from SBWeaponBase with consistent API
- **I** — Use interfaces (ISBInteractable, ISBDamageable) over fat base classes
- **D** — Systems reference abstractions (weapon interface), not concrete weapon classes

## Performance Rules

1. **Tick budget:** Avoid Tick where possible. Use timers or event-driven patterns.
2. **Object pooling:** Projectiles, VFX, and decals use pooling.
3. **LODs:** Every mesh needs at least 3 LOD levels for mobile.
4. **Draw calls:** Target < 800 for mobile. Use instanced meshes for foliage/buildings.
5. **Texture budget:** Max 2K for mobile, 4K for PC. Use texture streaming.
6. **Physics:** Use simple collision. No complex collision on mobile.
