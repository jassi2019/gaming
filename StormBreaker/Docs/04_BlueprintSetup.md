# StormBreaker — Blueprint Setup Instructions (Phase 1)

## Step-by-Step Editor Setup

### 1. Create Unreal Project
1. Open Unreal Engine 5.6
2. Create new **Blank C++ Project** named `StormBreaker`
3. Copy the generated `Source/` files with the ones from this repository
4. Enable required plugins (Project Settings → Plugins):
   - Enhanced Input ✓
   - Gameplay Abilities ✓
   - Online Subsystem EOS ✓
   - Niagara ✓
   - Water ✓
   - Chaos Vehicles ✓
5. Regenerate project files (right-click .uproject → Generate Visual Studio Files)
6. Build the project in Visual Studio / Rider

### 2. Content Folder Mirroring
Create the Content folder structure in the editor to match the Git repo:
```
Content/
├── Characters/Player/      — meshes, textures, anims, BPs
├── Characters/Bot/
├── Weapons/{type}/         — per weapon type
├── Maps/MainMap/
├── Maps/Lobby/
├── Maps/TestMap/
├── Environment/
├── Vehicles/
├── UI/HUD/
├── UI/Lobby/
├── UI/Inventory/
├── Effects/Niagara/
├── Audio/
├── Loot/
├── BattleRoyale/
├── Input/
└── DataTables/
```

### 3. Input Setup (Enhanced Input System)

#### Create Input Actions (Content/Input/)
| Asset | Name | Type |
|-------|------|------|
| Input Action | `IA_Move` | Axis2D (Vector2D) |
| Input Action | `IA_Look` | Axis2D (Vector2D) |
| Input Action | `IA_Jump` | Digital (Bool) |
| Input Action | `IA_Sprint` | Digital (Bool) |
| Input Action | `IA_Crouch` | Digital (Bool) |
| Input Action | `IA_Prone` | Digital (Bool) |
| Input Action | `IA_Fire` | Digital (Bool) |
| Input Action | `IA_ADS` | Digital (Bool) |
| Input Action | `IA_Reload` | Digital (Bool) |
| Input Action | `IA_Interact` | Digital (Bool) |
| Input Action | `IA_Inventory` | Digital (Bool) |
| Input Action | `IA_Map` | Digital (Bool) |
| Input Action | `IA_WeaponSlot1` | Digital (Bool) |
| Input Action | `IA_WeaponSlot2` | Digital (Bool) |
| Input Action | `IA_WeaponSlot3` | Digital (Bool) |

#### Create Input Mapping Context (Content/Input/)
| Asset | Name |
|-------|------|
| Input Mapping Context | `IMC_Default` |

Bind keys in `IMC_Default`:
| Action | Keyboard/Mouse | Mobile (touch zones) |
|--------|---------------|---------------------|
| IA_Move | WASD | Left virtual joystick |
| IA_Look | Mouse Delta | Right touch zone |
| IA_Jump | Space | Jump button |
| IA_Sprint | Left Shift (hold) | Sprint toggle button |
| IA_Crouch | C | Crouch button |
| IA_Prone | Z | Prone button |
| IA_Fire | Left Mouse | Fire button |
| IA_ADS | Right Mouse | ADS button |
| IA_Reload | R | Reload button |
| IA_Interact | F | Interact button |
| IA_Inventory | Tab | Inventory button |
| IA_Map | M | Map button |

### 4. Create Blueprint Classes

#### BP_SBGameInstance
1. Content Browser → Blueprint Class → Parent: `SBGameInstance`
2. Name: `BP_SBGameInstance`
3. Save to: `Content/BattleRoyale/Blueprints/`
4. Set in Project Settings → Maps & Modes → Game Instance Class → `BP_SBGameInstance`

#### BP_SBPlayerController
1. Blueprint Class → Parent: `SBPlayerController`
2. Name: `BP_SBPlayerController`
3. Save to: `Content/Characters/Player/Blueprints/`
4. Set `Default Mapping Context` to `IMC_Default`

#### BP_SBBattleRoyaleGameMode
1. Blueprint Class → Parent: `SBBattleRoyaleGameMode`
2. Name: `BP_SBBattleRoyaleGameMode`
3. Save to: `Content/BattleRoyale/Blueprints/`
4. Set `Player Controller Class` to `BP_SBPlayerController`
5. Set match config: Max Players = 100, Min To Start = 2

#### BP_SBBattleRoyaleGameState
1. Blueprint Class → Parent: `SBBattleRoyaleGameState`
2. Save to: `Content/BattleRoyale/Blueprints/`

### 5. Project Settings

#### Maps & Modes
- Default GameMode: `BP_SBGameModeBase` (lobby)
- Server Default Game Mode: `BP_SBBattleRoyaleGameMode`
- Game Instance: `BP_SBGameInstance`
- Default Map: `/Game/Maps/Lobby/Lobby`

#### Collision
Add custom object channels:
| Channel | Name | Default Response |
|---------|------|-----------------|
| ECC_GameTraceChannel1 | Bullet | Block |
| ECC_GameTraceChannel2 | Projectile | Block |
| ECC_GameTraceChannel3 | Interaction | Overlap |
| ECC_GameTraceChannel4 | Vehicle | Block |

#### Online
- Default Platform Service: EOS
- Configure EOS credentials when ready

### 6. Test Map
1. Create a new level: `Content/Maps/TestMap/TestMap`
2. Add a floor plane (10000x10000)
3. Add a Player Start actor
4. Add a Nav Mesh Bounds Volume (for AI testing later)
5. Set World Settings → GameMode Override → `BP_SBBattleRoyaleGameMode`
6. Play in editor (PIE) with 2 players to verify multiplayer

### 7. Verification Checklist
- [ ] Project compiles without errors
- [ ] BP_SBGameInstance is set as Game Instance
- [ ] BP_SBBattleRoyaleGameMode loads on TestMap
- [ ] Input mapping context binds on BeginPlay
- [ ] Two PIE clients can connect (listen server)
- [ ] GameState replicates match phase to both clients
- [ ] Player state shows correct Kills/Status
- [ ] Settings subsystem initializes on game start
- [ ] Log categories appear in Output Log (LogStormBreaker, etc.)
