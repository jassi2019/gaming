# StormBreaker — Battle Royale Game

A complete Battle Royale mobile game built with **Unreal Engine 5.8**, targeting **Android** and **Windows**.

Original assets, branding, characters, maps, UI, sounds, and mechanics. No copyrighted content.

---

## Tech Stack

| Technology | Usage |
|-----------|-------|
| Unreal Engine 5.8 | Game engine |
| C++ | Core gameplay systems |
| Blueprints | Visual scripting layer |
| Enhanced Input System | Input handling (mobile + KB/M) |
| GAS (Gameplay Ability System) | Abilities, damage, health, effects |
| EOS (Epic Online Services) | Multiplayer sessions, matchmaking |
| Niagara | VFX (muzzle flash, explosions, impacts) |
| Chaos Vehicles | Vehicle physics |

## Development Phases

| Phase | Module | Status |
|-------|--------|--------|
| 1 | Project Architecture | ✅ Complete |
| 2 | Third Person Character | ✅ Complete |
| 3 | Weapon System | ✅ Complete |
| 4 | Inventory & Loot & BR Core | ✅ Complete |
| 5 | Battle Royale Match Flow | ✅ Complete |
| 6 | Multiplayer & Networking | ✅ Complete |
| 7 | AI Bots (99-bot BR) | ✅ Complete |
| 8 | Production UI (PUBG-style) | ✅ Complete |
| 9 | Mobile Optimization | ✅ Complete |
| 10 | Backend & Live Service | ✅ Complete |

---

## Phase 1 — Project Architecture (Complete)

### What Was Created

**Project Structure:**
- Complete UE5 project with `.uproject`, Build.cs, Target files
- Client module (`StormBreaker`) + dedicated server module (`StormBreakerServer`)
- 11 organized source code directories (Core, Character, Weapon, Inventory, BattleRoyale, Multiplayer, AI, UI, Vehicle, Backend, Subsystems)
- Full Content folder hierarchy for all asset types

**Core C++ Classes:**

| Class | Purpose |
|-------|---------|
| `SBGameInstance` | Session management, EOS integration, quality settings |
| `SBBattleRoyaleGameMode` | Match lifecycle, zone control, win conditions (server-only) |
| `SBBattleRoyaleGameState` | Replicated match state — zone data, alive count, timers |
| `SBPlayerState` | Per-player stats (kills, damage), team, status, GAS owner |
| `SBPlayerController` | Enhanced Input binding, HUD management, RPCs |
| `SBAttributeSet` | GAS attributes — Health, Shield, Stamina, damage pipeline |
| `SBSettingsSubsystem` | Graphics/Audio/Controls settings with persistence |
| `SBGameModeBase` | Base game mode for lobby/menus |

**Module & Tag System:**
- 9 log categories (`LogStormBreaker`, `LogSBWeapon`, `LogSBCharacter`, etc.)
- 4 custom collision channels (Bullet, Projectile, Interaction, Vehicle)
- Native gameplay tags (State.Dead, State.Downed, State.Sprinting, Slot.Primary, etc.)

**Config Files:**
- `DefaultEngine.ini` — collision profiles, EOS config, physics
- `DefaultGame.ini` — packaging, cooking, localization
- `DefaultInput.ini` — Enhanced Input settings
- `DefaultScalability.ini` — 4-tier quality presets (Low→Ultra) for mobile

**Documentation:**
- `01_CodingStandards.md` — naming conventions, SOLID principles, code organization
- `02_MultiplayerArchitecture.md` — network topology, authority model, replication strategy
- `03_AssetPipeline.md` — import settings, texture budgets, mobile optimization targets
- `04_BlueprintSetup.md` — step-by-step editor setup with verification checklist

### Folder Structure

```
StormBreaker/
├── Config/
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   ├── DefaultInput.ini
│   └── DefaultScalability.ini
├── Content/
│   ├── Characters/{Player,Bot}/{Meshes,Textures,Animations,Blueprints}
│   ├── Weapons/{AssaultRifle,Sniper,SMG,Shotgun,Pistol,Grenade,Melee}/
│   ├── Maps/{MainMap,Lobby,TestMap}
│   ├── Environment/{Buildings,Terrain,Foliage,Props}
│   ├── Vehicles/{Car,Boat,Motorcycle}
│   ├── UI/{HUD,Lobby,Inventory,Store,Common}
│   ├── Effects/{Niagara,Materials}
│   ├── Audio/{Weapons,Character,Environment,UI,Music}
│   ├── Loot/{Blueprints,DataTables}
│   ├── BattleRoyale/{Blueprints,DataTables}
│   ├── Input/
│   └── DataTables/
├── Source/
│   ├── StormBreaker/
│   │   ├── Core/              — GameMode, GameState, PlayerState, Controller
│   │   ├── Character/         — (Phase 2)
│   │   ├── Weapon/            — (Phase 3)
│   │   ├── Inventory/         — (Phase 4)
│   │   ├── BattleRoyale/      — (Phase 5)
│   │   ├── Multiplayer/       — (Phase 6)
│   │   ├── AI/                — (Phase 7)
│   │   ├── UI/                — (Phase 8)
│   │   ├── Vehicle/           — (Phase 5/9)
│   │   ├── Backend/           — (Phase 10)
│   │   └── Subsystems/        — Settings, services
│   ├── StormBreakerServer/    — Dedicated server module
│   ├── StormBreaker.Target.cs
│   ├── StormBreakerEditor.Target.cs
│   └── StormBreakerServer.Target.cs
├── Docs/
│   ├── 01_CodingStandards.md
│   ├── 02_MultiplayerArchitecture.md
│   ├── 03_AssetPipeline.md
│   ├── 04_BlueprintSetup.md
│   ├── 05_Phase2_CharacterSetup.md
│   ├── 06_Phase3_WeaponSystem.md
│   ├── 07_Phase4_InventoryLoot.md
│   └── 08_Phase5_MatchFlow.md
├── Plugins/
├── Tests/
├── .gitignore
└── StormBreaker.uproject
```

### Performance Targets

| Metric | Android (Mid) | Android (High) | Windows |
|--------|--------------|----------------|---------|
| FPS | 30 stable | 60 stable | 60-120 |
| Draw Calls | < 500 | < 800 | < 2000 |
| RAM | < 2 GB | < 3 GB | < 6 GB |

---

## How to Build

1. Install Unreal Engine 5.8 via Epic Games Launcher
2. Clone this repository
3. Open `StormBreaker/StormBreaker.uproject`
4. Follow [Blueprint Setup Instructions](StormBreaker/Docs/04_BlueprintSetup.md)
5. Build and run

## Phase 2 — Third Person Character (Complete)

### New C++ Classes

| Class | Purpose |
|-------|---------|
| `USBCharacterMovementComponent` | Custom CMC — Sprint (800), Prone (100), Vault (0.4s arc), Mantle (0.8s climb), ADS speed, full network prediction via FSBSavedMove |
| `ASBCharacterBase` | Main character — Enhanced Input bindings, third-person camera with smooth ADS transition, all movement states, GAS integration |
| `USBCharacterAnimInstance` | Animation driver — speed, direction, aim offset (pitch/yaw), lean, foot IK, state flags for all movement modes |
| `USBMobileTouchWidget` | Mobile touch — virtual joystick, look zone, action button delegates for Fire/ADS/Reload/Interact |

### Movement Specs

| Action | Speed (cm/s) | Key | Network |
|--------|-------------|-----|---------|
| Walk | 250 | WASD (slow stick) | CMC built-in |
| Run | 500 | WASD | CMC built-in |
| Sprint | 800 | Left Shift (hold) | FLAG_Custom_0 |
| Jump | 500 Z vel | Space | CMC built-in |
| Crouch | 200 | C (toggle) | CMC built-in |
| Prone | 100 | Z (toggle) | FLAG_Custom_1 |
| Vault | N/A | Space (auto) | Server authority |
| Mantle | N/A | Space (auto) | Server authority |
| Swim | 300 | Auto in water | MOVE_Swimming |
| ADS | 200 | Right Mouse (hold) | FLAG_Custom_2 + COND_SkipOwner |

### Camera System
- Third-person spring arm with collision detection
- Default: 300 cm boom, 90 FOV, right shoulder offset
- ADS: 100 cm boom, 65 FOV, tighter shoulder offset
- Smooth interpolation at 12 units/sec
- Camera lag (15 speed) + rotation lag (20 speed)

## Phase 3 — Weapon & Combat System (Complete)

### New C++ Classes

| Class | Lines | Purpose |
|-------|-------|---------|
| `SBWeaponTypes.h` | 250 | All enums (WeaponType, FireMode, DamageType, Slot, AmmoType, Surface), structs (RecoilData, SpreadData, DamageData, ProjectileData, AttachmentModifier, HitResult, FireEvent) |
| `USBWeaponDataAsset` | 220 | Data asset defining all weapon properties — no subclassing needed, fully data-driven |
| `ASBWeaponBase` | 650 | Weapon actor: fire (hitscan with penetration + projectile), recoil patterns, spread, damage falloff, headshot/limb multipliers, reload, equip/unequip, fire modes (Single/Burst/Auto), attachments, aim assist |
| `USBWeaponComponent` | 280 | Inventory manager: 5 slots (Primary/Secondary/Sidearm/Melee/Throwable), weapon switching, drop/pickup, ammo pool, action forwarding |
| `ASBProjectileBase` | 130 | Projectile: sphere collision, gravity, direct + radial damage, trail/impact VFX |
| `ASBWeaponPickup` | 100 | World pickup: interaction sphere, stored ammo, bob animation, replicated |

### Combat Pipeline
```
Client: press Fire → local effects (muzzle flash, recoil, sound)
                    → Server_Fire RPC
Server: validate fire rate + ammo (anti-cheat)
       → consume ammo
       → hitscan trace with penetration OR spawn projectile
       → apply damage (shield → health via GAS)
       → Multicast_PlayFireEffects to other clients
```

### Weapon Specs

| Weapon | Type | RPM | Damage | Range | Mag | Special |
|--------|------|-----|--------|-------|-----|---------|
| AK47 | AR | 600 | 36 | 4000cm | 30 | 1 pen, Auto/Single |
| MP5 | SMG | 800 | 24 | 2500cm | 25 | Low recoil |
| AWM | Sniper | 45 | 120 | 10000cm | 5 | 3x headshot |
| S12 | Shotgun | 120 | 18x8 | 1000cm | 8 | 8 pellets |
| M9 | Pistol | 400 | 30 | 3000cm | 15 | Single only |
| Frag | Grenade | — | 150 | 500r | 1+4 | Radial damage |
| Knife | Melee | 120 | 50 | 200cm | — | 60° arc |

## Phase 4 — Inventory, Loot & Battle Royale Core (Complete)

### New C++ Classes

| Class | Purpose |
|-------|---------|
| `FSBItemDefinition` | DataTable row defining all item properties — weapon, ammo, consumable, armor, backpack, attachment |
| `USBInventoryComponent` | Backpack inventory with weight/capacity, equipment (helmet/vest/backpack), consumable use (timed+cancellable), boost system (passive regen), auto-loot settings |
| `USBLootManager` | WorldSubsystem for weighted random loot generation from DataTables — building loot, air drop loot |
| `ASBDeathCrate` | Spawned on death with all player items, lootable by other players, auto-destroys when empty |
| `USBKnockReviveComponent` | Alive → Knocked (90s bleed-out, crawl only) → Revive (10s, 300u range) → Dead → Spectator |

### Systems

- **Armor:** L1/L2/L3 helmet+vest with 30%/40%/55% damage reduction, durability that depletes and breaks
- **Consumables:** Bandage (10HP/4s/cap75), FirstAid (75HP/6s/cap75), MedKit (100HP/8s), EnergyDrink (+40 boost), Painkiller (+60 boost), Adrenaline (+100 boost)
- **Boost:** 0-100 scale, decays 3/s, tiers at 20/40/60 give 2/4/6 HP regen/s, 60+ gives 6.5% speed bonus
- **Knock/Revive:** Team-mode aware, solo = instant death, knocked players crawl at 50cm/s

## Phase 5 — Battle Royale Match Flow (Complete)

### New Classes

| Class | Purpose |
|-------|---------|
| `ASBAircraftSystem` | Random flight path generation, player jump/eject, replicated path |
| `USBParachuteComponent` | FreeFall (5000cm/s) → Deploy (800cm/s descent) → Landing, auto-deploy at 500m, wind, steering |
| `ASBZoneManager` | 8-phase PUBG-style zone with wait/shrink/damage, ease-in-out interpolation, random center shift |
| `ASBAirDrop` | Cargo crate falling from 15000cm, smoke trail, high-tier loot, lootable after landing |
| `USBMinimapDataComponent` | Collects markers (player, team, zone, aircraft, drops, crates, ping) + compass heading |

### Zone Phases (8 total)
Phase 1-3: 0.4-1.0 DPS, 300-150s wait, 50% radius reduction each
Phase 4-6: 2.0-5.0 DPS, 120-60s wait, increasing intensity
Phase 7-8: 8.0-14.0 DPS, 30s wait, final collapse to 0

## Next: Phase 6 — Multiplayer & Networking

Will include: Dedicated server, lobby, matchmaking, voice chat ready, team mode, spectator mode.
