# StormBreaker — Battle Royale Game

A complete Battle Royale mobile game built with **Unreal Engine 5.6**, targeting **Android** and **Windows**.

Original assets, branding, characters, maps, UI, sounds, and mechanics. No copyrighted content.

---

## Tech Stack

| Technology | Usage |
|-----------|-------|
| Unreal Engine 5.6 | Game engine |
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
| 2 | Third Person Character | ⬜ Pending |
| 3 | Weapon System | ⬜ Pending |
| 4 | Inventory System | ⬜ Pending |
| 5 | Battle Royale Mechanics | ⬜ Pending |
| 6 | Multiplayer | ⬜ Pending |
| 7 | AI Bots | ⬜ Pending |
| 8 | UI | ⬜ Pending |
| 9 | Optimization | ⬜ Pending |
| 10 | Backend | ⬜ Pending |

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
│   └── 04_BlueprintSetup.md
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

1. Install Unreal Engine 5.6 via Epic Games Launcher
2. Clone this repository
3. Open `StormBreaker/StormBreaker.uproject`
4. Follow [Blueprint Setup Instructions](StormBreaker/Docs/04_BlueprintSetup.md)
5. Build and run

## Next: Phase 2 — Third Person Character

Will include: Walk, Run, Sprint, Jump, Crouch, Prone, Vault, Mantle, Swimming, Climbing, Camera System with full C++ classes, Animation Blueprint, and input bindings.

