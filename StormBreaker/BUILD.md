# StormBreaker — Build Instructions

Complete setup guide for building the StormBreaker Battle Royale project from a fresh Windows machine.

## Prerequisites

### Required Software

| Software | Version | Purpose |
|----------|---------|---------|
| Unreal Engine | 5.8 | Game engine |
| Visual Studio | 2022 (17.8+) or 2026 (18.x) | C++ compiler (MSVC v143+) |
| Windows SDK | 10.0.22621.0+ | Platform SDK |
| .NET SDK | 6.0+ (UE 5.8 bundles .NET 10) | UnrealBuildTool |
| Git | 2.40+ | Version control |
| Android Studio | 2024.1+ | Android builds (optional) |
| Android NDK | r25c+ | Android native compilation (optional) |

### Visual Studio 2022 Workloads

Install via Visual Studio Installer:

1. **Desktop development with C++**
   - MSVC v143 build tools
   - Windows 10/11 SDK (10.0.22621.0)
   - C++ CMake tools
   - C++ ATL for v143

2. **Game development with C++**
   - Unreal Engine installer (optional — engine can be installed separately)

3. **.NET desktop development**
   - .NET 6.0 Runtime

### Unreal Engine 5.8 Installation

1. Install Epic Games Launcher from [epicgames.com](https://www.epicgames.com/store/en-US/download)
2. Open Epic Games Launcher → Unreal Engine → Library
3. Click **+** → Select version **5.8**
4. Install with these options enabled:
   - Core Components
   - Starter Content (optional)
   - Templates (optional)
   - Target Platforms: **Windows**, **Android**

### Android Setup (Optional — for mobile builds)

1. Install Android Studio
2. Open Android Studio → SDK Manager
3. Install:
   - Android SDK Platform 33 (API 33)
   - Android SDK Build-Tools 33.0.2
   - Android NDK r25c (25.2.9519653)
   - Android SDK Command-line Tools
4. In UE5 Editor → Project Settings → Platforms → Android:
   - Set SDK/NDK paths
   - Accept SDK license

## Clone and Setup

```bash
git clone https://github.com/jassi2019/gaming.git
cd gaming/StormBreaker
```

## Generate Project Files

### Option A: Via File Explorer
1. Right-click `StormBreaker.uproject`
2. Select **Generate Visual Studio project files**
3. Wait for completion
4. Open `StormBreaker.sln`

### Option B: Via Command Line
```bash
# Set UE_INSTALL_PATH to your Unreal Engine installation
# Example: C:\Users\HP\Music\Own Game\UE_5.8
# Or: C:\Program Files\Epic Games\UE_5.8
"<UE_INSTALL_PATH>\Engine\Build\BatchFiles\GenerateProjectFiles.bat" StormBreaker.uproject
```

### Option C: Via UnrealBuildTool
```bash
"<UE_INSTALL_PATH>\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="StormBreaker.uproject" -game -engine
```

## Build Configurations

### Development Editor (daily development)
```bash
# Visual Studio: Build → Build Solution (F5 to debug)
# Configuration: Development Editor
# Platform: Win64

# Or via command line:
"<UE_INSTALL_PATH>\Engine\Build\BatchFiles\Build.bat" StormBreakerEditor Win64 Development "StormBreaker.uproject"
```

### Development Game (testing)
```bash
"<UE_INSTALL_PATH>\Engine\Build\BatchFiles\Build.bat" StormBreaker Win64 Development "StormBreaker.uproject"
```

### Development Server (dedicated server)
```bash
"<UE_INSTALL_PATH>\Engine\Build\BatchFiles\Build.bat" StormBreakerServer Win64 Development "StormBreaker.uproject"
```

### Shipping (release)
```bash
"<UE_INSTALL_PATH>\Engine\Build\BatchFiles\Build.bat" StormBreaker Win64 Shipping "StormBreaker.uproject"
```

### Android (mobile)
```bash
"<UE_INSTALL_PATH>\Engine\Build\BatchFiles\Build.bat" StormBreaker Android Development "StormBreaker.uproject"
```

## Project Structure

```
StormBreaker/
├── StormBreaker.uproject          — Engine 5.8, all plugins
├── BUILD.md                       — This file
├── Config/
│   ├── DefaultEngine.ini          — Collision channels, EOS, physics
│   ├── DefaultGame.ini            — Packaging, cooking
│   ├── DefaultInput.ini           — Enhanced Input settings
│   └── DefaultScalability.ini     — 4-tier quality presets
├── Source/
│   ├── StormBreaker.Target.cs     — Game build target
│   ├── StormBreakerEditor.Target.cs — Editor build target
│   ├── StormBreakerServer.Target.cs — Dedicated server build target
│   ├── StormBreaker/              — Main game module (57 files)
│   │   ├── StormBreaker.Build.cs  — Module dependencies
│   │   ├── StormBreaker.h/.cpp    — Module root, log categories, gameplay tags
│   │   ├── Core/                  — GameInstance, GameMode, GameState, PlayerState, PlayerController, AttributeSet
│   │   ├── Character/             — CharacterBase, MovementComponent, AnimInstance
│   │   ├── Weapon/                — WeaponBase, WeaponComponent, ProjectileBase, WeaponPickup, WeaponDataAsset, WeaponTypes
│   │   ├── Inventory/             — InventoryComponent, InventoryTypes, LootManager, DeathCrate
│   │   ├── BattleRoyale/          — KnockRevive, Aircraft, Parachute, ZoneManager, AirDrop, MinimapData
│   │   ├── UI/                    — MobileTouchWidget
│   │   └── Subsystems/            — SettingsSubsystem
│   └── StormBreakerServer/        — Dedicated server module (3 files)
│       ├── StormBreakerServer.Build.cs
│       ├── StormBreakerServer.h
│       └── StormBreakerServer.cpp
└── Docs/                          — Phase documentation
```

## Module Dependencies

### StormBreaker (main module)

**Public Dependencies:**
Core, CoreUObject, Engine, InputCore, EnhancedInput, GameplayAbilities, GameplayTags, GameplayTasks, UMG, SlateCore, Slate, Niagara, PhysicsCore, ChaosVehicles, NavigationSystem, AIModule, OnlineSubsystem, OnlineSubsystemUtils, NetCore

**Private Dependencies:**
OnlineSubsystemEOS, Http, Json, JsonUtilities

### StormBreakerServer (server module)

**Public Dependencies:**
Core, CoreUObject, Engine, StormBreaker, OnlineSubsystem, OnlineSubsystemEOS

### Required Plugins

| Plugin | Purpose |
|--------|---------|
| EnhancedInput | Input system |
| GameplayAbilities | GAS (health, damage, effects) |
| OnlineSubsystemEOS | Epic Online Services multiplayer |
| OnlineSubsystem | Session management |
| Niagara | VFX (muzzle flash, explosions, trails) |
| Water | Water volumes for swimming |
| ChaosVehiclesPlugin | Vehicle physics |
| AudioModulation | Dynamic audio |
| Metasound | Procedural audio |
| NavigationSystem | AI navigation |

## Build Targets

| Target | Type | Modules | Use |
|--------|------|---------|-----|
| StormBreakerEditor | Editor | StormBreaker | Development in PIE |
| StormBreaker | Game | StormBreaker | Standalone/packaged game |
| StormBreakerServer | Server | StormBreaker + StormBreakerServer | Headless dedicated server |

## Source Statistics

| Metric | Count |
|--------|-------|
| C++ source files (.h + .cpp) | 60 |
| C# build files (.cs) | 3 |
| Total source files | 63 |
| Total lines of C++ | ~9,935 |
| Classes (UCLASS) | 26 |
| Structs (USTRUCT) | 18 |
| Enums (UENUM) | 17 |
| Components | 7 |
| Actors | 8 |
| Data Assets | 1 |
| Subsystems | 2 |

## Troubleshooting

### "Module not found" errors
Ensure all plugins are enabled in the .uproject. Open the editor, go to Edit → Plugins, and verify each plugin is enabled.

### "Unresolved external symbol" for GAS
Ensure `GameplayAbilities`, `GameplayTags`, and `GameplayTasks` are in Build.cs public dependencies.

### EOS linker errors
If you haven't configured EOS credentials, the project still compiles but EOS-dependent features won't connect. Set credentials in `Config/DefaultEngine.ini` under `[OnlineSubsystemEOS]`.

### Android build fails
1. Verify NDK/SDK paths in Project Settings → Platforms → Android
2. Run `SetupAndroid.bat` from `Engine/Extras/Android/`
3. Accept SDK license: `sdkmanager --licenses`

### "Cannot open include file" in Visual Studio
1. Right-click StormBreaker project in Solution Explorer
2. Set as Startup Project
3. Build → Rebuild Solution
4. If still failing, re-generate project files

### IntelliSense errors (red squiggles) but project builds
This is normal. UE5 uses generated headers (.generated.h) that IntelliSense can't always parse. If the project builds successfully, ignore IntelliSense errors.

## Packaging

### Windows
```
Editor → Platforms → Windows → Package Project
```
Or via command line:
```bash
"<UE_INSTALL_PATH>\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="StormBreaker.uproject" -platform=Win64 -configuration=Shipping -cook -stage -pak -package
```

### Android
```bash
"<UE_INSTALL_PATH>\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="StormBreaker.uproject" -platform=Android -configuration=Shipping -cook -stage -pak -package
```

### Dedicated Server
```bash
"<UE_INSTALL_PATH>\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="StormBreaker.uproject" -platform=Win64 -server -serverconfig=Shipping -cook -stage -pak -package
```
