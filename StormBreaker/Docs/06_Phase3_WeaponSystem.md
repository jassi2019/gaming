# StormBreaker — Phase 3: Weapon & Combat System Setup

## Architecture

```
USBWeaponDataAsset (Data)        ASBWeaponBase (Actor)
├── Identity                     ├── WeaponMeshComp
├── Combat (damage, recoil,      ├── Fire logic (hitscan/projectile)
│   spread, fire modes)          ├── Reload / Equip / Unequip
├── Magazine                     ├── Recoil / Spread state
├── ADS / Timing                 ├── Attachment modifiers
├── Visuals / Audio              ├── Server RPCs (fire, reload)
├── Animations                   ├── Multicast effects
└── Attachments                  └── Aim assist (mobile)

USBWeaponComponent (on Character)    ASBWeaponPickup (World)
├── 5 weapon slots                   ├── Pickup mesh + bobbing
├── Active weapon management         ├── Interaction sphere
├── Switch / Drop / Add              ├── Stored ammo
├── Ammo pool per type               └── Replicated data
├── Action forwarding
└── Server RPCs

ASBProjectileBase (for non-hitscan)
├── Sphere collision
├── ProjectileMovement (gravity, speed)
├── Direct or radial damage
└── Trail + impact VFX
```

## Blueprint Setup

### 1. Create Weapon Data Assets

Create these in `Content/Weapons/Shared/DataTables/`:

#### DA_AssaultRifle_AK
| Property | Value |
|----------|-------|
| WeaponID | `AK47` |
| DisplayName | `Assault Rifle` |
| WeaponType | AssaultRifle |
| DefaultSlot | Primary |
| AmmoType | Rifle_762 |
| DamageMethod | Hitscan |
| FireModes | Auto, Single |
| DefaultFireMode | Auto |
| FireRate | 600 |
| PelletsPerShot | 1 |
| BaseDamage | 36 |
| HeadshotMultiplier | 2.5 |
| EffectiveRange | 4000 |
| MaxRange | 15000 |
| PenetrationCount | 1 |
| MagazineSize | 30 |
| MaxReserveAmmo | 120 |
| ReloadTime | 2.3 |
| Recoil: VerticalMin/Max | 0.4 / 0.7 |
| Recoil: HorizontalMin/Max | -0.2 / 0.2 |
| Spread: HipFire | 4.0 |
| Spread: ADS | 0.5 |

#### DA_SMG_MP5
| Property | Value |
|----------|-------|
| WeaponID | `MP5` |
| WeaponType | SMG |
| DefaultSlot | Primary |
| AmmoType | SMG_9mm |
| FireRate | 800 |
| BaseDamage | 24 |
| EffectiveRange | 2500 |
| MagazineSize | 25 |
| Recoil: Vertical | 0.2 / 0.4 |

#### DA_Sniper_AWM
| Property | Value |
|----------|-------|
| WeaponID | `AWM` |
| WeaponType | Sniper |
| DefaultSlot | Primary |
| AmmoType | Sniper_300 |
| FireModes | Single |
| FireRate | 45 |
| BaseDamage | 120 |
| HeadshotMultiplier | 3.0 |
| EffectiveRange | 10000 |
| MaxRange | 30000 |
| MagazineSize | 5 |
| MaxReserveAmmo | 25 |
| Spread: HipFire | 8.0 |
| Spread: ADS | 0.1 |

#### DA_Shotgun_S12
| Property | Value |
|----------|-------|
| WeaponID | `S12` |
| WeaponType | Shotgun |
| DefaultSlot | Secondary |
| AmmoType | Shotgun_12g |
| FireRate | 120 |
| PelletsPerShot | 8 |
| BaseDamage | 18 (per pellet) |
| EffectiveRange | 1000 |
| MaxRange | 3000 |
| Spread: HipFire | 6.0 |

#### DA_Pistol_M9
| Property | Value |
|----------|-------|
| WeaponID | `M9` |
| WeaponType | Pistol |
| DefaultSlot | Sidearm |
| AmmoType | Pistol_45 |
| FireModes | Single |
| FireRate | 400 |
| BaseDamage | 30 |
| MagazineSize | 15 |

#### DA_Grenade_Frag
| Property | Value |
|----------|-------|
| WeaponID | `FragGrenade` |
| WeaponType | Grenade |
| DefaultSlot | Throwable |
| ThrowForce | 2000 |
| FuseTime | 4.0 |
| ExplosionRadius | 500 |
| ExplosionDamage | 150 |
| MagazineSize | 1 |
| MaxReserveAmmo | 4 |

#### DA_Melee_Knife
| Property | Value |
|----------|-------|
| WeaponID | `Knife` |
| WeaponType | Melee |
| DefaultSlot | Melee |
| BaseDamage | 50 |
| MeleeRange | 200 |
| MeleeAngle | 60 |
| FireRate | 120 |

### 2. Create Input Actions (if not already from Phase 2)

Add to `Content/Input/`:
| Asset | Key | Type |
|-------|-----|------|
| `IA_Fire` | Left Mouse / Touch Fire | Digital |
| `IA_Reload` | R / Touch Reload | Digital |
| `IA_WeaponSlot1` | 1 | Digital |
| `IA_WeaponSlot2` | 2 | Digital |
| `IA_WeaponSlot3` | 3 | Digital |

Add to `IMC_Default` mappings.

### 3. Assign to Character Blueprint

Open `BP_SBCharacter`:
- Set `IA_Fire`, `IA_Reload`, `IA_WeaponSlot1/2/3` references
- Weapon Component is auto-created (C++ default subobject)
- Set `Weapon Attach Socket` to `weapon_r` (or your skeleton's right-hand socket)
- Set `Backpack Socket` to `spine_03` (or your back socket)

### 4. Firing Range Test Map

Create `Content/Maps/TestMap/FiringRange`:
1. **Spawn Area** — Player Start
2. **Target Dummies** — Static mesh pawns at 10m, 25m, 50m, 100m, 200m
3. **Weapon Pickup Rack** — Place `ASBWeaponPickup` actors with each weapon data asset
4. **Ammo Crates** — Pickups that call `AddAmmoForType`
5. **Damage Number Display** — Place text render actors showing last damage
6. **Surface Test Wall** — Sections of metal, wood, concrete, glass for impact VFX
7. **Penetration Test** — Thin wall to shoot through (test penetration)
8. **Moving Target** — Blueprint with simple patrol for lead testing

### 5. Debug Overlay (Blueprint)

Create `WBP_WeaponDebug` widget:
- Bind to `WeaponComponent->GetActiveWeapon()`:
  - Current spread (circle visualization)
  - Recoil accumulation
  - Ammo: `Mag / Reserve`
  - Fire mode
  - Weapon state
  - DPS at current fire rate
  - Last hit distance + damage

### 6. Testing Checklist

**Firing:**
- [ ] Single fire mode: one shot per click
- [ ] Burst mode: exactly 3 shots per click
- [ ] Auto mode: continuous fire while holding
- [ ] Dry fire sound when magazine empty
- [ ] Auto-reload on empty magazine fire attempt
- [ ] Shotgun fires 8 pellets per shot

**Recoil & Spread:**
- [ ] Recoil kicks camera up with each shot
- [ ] Recoil reduces when ADS
- [ ] Recoil reduces when crouching/prone
- [ ] Spread increases during sustained fire
- [ ] Spread recovers when not firing
- [ ] Spread increases while moving / airborne
- [ ] Recoil pattern follows data asset pattern array

**Damage:**
- [ ] Base damage applied at close range
- [ ] Damage falls off at distance
- [ ] Headshot multiplier (2.5x default)
- [ ] Limb multiplier (0.75x)
- [ ] Penetration: bullet passes through thin wall with reduced damage
- [ ] Grenade radial damage with falloff

**Reload & Switching:**
- [ ] Reload animation plays for correct duration
- [ ] Empty reload takes longer than tactical reload
- [ ] Cannot fire during reload
- [ ] Weapon switching plays equip/unequip montages
- [ ] Slot 1/2/3 keys switch weapons
- [ ] Scroll wheel switches next/previous
- [ ] Drop weapon spawns pickup with current ammo

**Networking:**
- [ ] Fire effects visible to other players
- [ ] Server validates fire rate (rejects rapid fire)
- [ ] Server validates ammo count
- [ ] Weapon state replicates to all clients
- [ ] Magazine ammo replicates to owner only (COND_OwnerOnly)
- [ ] Weapon pickup replicates to all clients
- [ ] Two PIE clients can see each other's fire effects

**Mobile:**
- [ ] Touch fire button triggers StartFire/StopFire
- [ ] Aim assist gently pulls toward nearby targets
- [ ] Aim assist configurable via data asset
