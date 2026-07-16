# StormBreaker — Phase 5: Battle Royale Match Flow

## Match Lifecycle

```
WaitingForPlayers → WarmUp (lobby, 60s countdown)
    → PlanePhase (aircraft flight, players jump)
    → InProgress (zone shrinking, combat)
    → EndGame (winner detected, stats displayed)
```

## Systems Created

### Aircraft System (`ASBAircraftSystem`)
- Random flight path across map (random angle + lateral offset)
- Configurable speed (5000 cm/s), altitude (15000 cm)
- Players jump via `PlayerJump()`, pawn detached and placed at aircraft location
- Auto-ejects remaining players at path end
- Fully replicated: path start/end, flying state

### Parachute Component (`USBParachuteComponent`)
- **FreeFall:** 5000 cm/s descent, 3000 cm/s forward, full steering
- **Deployed:** 800 cm/s descent, 1500 cm/s forward, wind effect
- Auto-deploy at 500m above ground via line trace
- Landing at 50m restores normal movement (gravity, walking)
- Parachute mesh spawned/destroyed dynamically
- Server-authoritative deployment

### Zone Manager (`ASBZoneManager`)
- 8-phase PUBG-style zone table:

| Phase | Radius× | Wait | Shrink | DPS | Shift |
|-------|---------|------|--------|-----|-------|
| 1 | 50% | 300s | 300s | 0.4 | 25% |
| 2 | 50% | 200s | 200s | 0.6 | 25% |
| 3 | 50% | 150s | 150s | 1.0 | 20% |
| 4 | 50% | 120s | 120s | 2.0 | 15% |
| 5 | 50% | 90s | 90s | 3.0 | 10% |
| 6 | 50% | 60s | 60s | 5.0 | 5% |
| 7 | 50% | 30s | 30s | 8.0 | 0% |
| 8 | 0% | 0s | 30s | 14.0 | 0% |

- Random center shifting within constraints
- Ease-in-out shrink interpolation
- Damage applied to all characters outside safe zone
- All zone state replicated

### Air Drop System (`ASBAirDrop`)
- Spawned at random safe zone locations
- Falls from 15000cm at 500 cm/s
- Smoke trail VFX during descent
- Contains high-tier loot from air drop table
- Lootable by any player after landing

### Minimap Data (`USBMinimapDataComponent`)
- Collects map markers every 0.1s:
  - Player (green), Teammates (blue), Zone circles
  - Aircraft path, Air drops (yellow), Death crates (orange)
  - Ping location (yellow, player-set)
- Compass heading from camera rotation
- Team-aware marker filtering

## Blueprint Setup

### 1. Aircraft
- Create `BP_Aircraft` from `ASBAircraftSystem`
- Assign aircraft mesh
- Set in GameMode's plane phase handler

### 2. Zone
- Create `BP_ZoneManager` from `ASBZoneManager`
- Spawned by GameMode at match start
- Override phase table in Blueprint if needed
- Create zone visualization Material (translucent blue dome)

### 3. Air Drop
- Create `BP_AirDrop` from `ASBAirDrop`
- Assign crate mesh + smoke trail Niagara
- GameMode spawns periodically during InProgress phase

### 4. Parachute
- Auto-created on character (default subobject)
- Assign parachute mesh in `BP_SBCharacter` defaults
- Aircraft calls `BeginFreeFall()` on jump

### 5. Minimap
- Auto-created on character
- Create `WBP_Minimap` widget reading from `MinimapDataComponent`
- Draw markers from `GetMarkers()` array
- Draw compass from `GetCompassHeading()`

## Testing Checklist

- [ ] Aircraft spawns with random path across map
- [ ] Players can jump from aircraft during flight
- [ ] Remaining players ejected at path end
- [ ] Free fall at correct speed with steering
- [ ] Parachute auto-deploys at 500m altitude
- [ ] Landing restores normal movement, no fall damage
- [ ] Zone displays target circle during wait phase
- [ ] Zone shrinks smoothly with ease-in-out
- [ ] Zone damage applied outside safe zone
- [ ] Zone damage increases per phase
- [ ] Air drop falls and lands correctly
- [ ] Air drop lootable after landing
- [ ] Minimap shows player, teammates, zones
- [ ] Compass heading updates with camera rotation
- [ ] All systems replicate to other clients
