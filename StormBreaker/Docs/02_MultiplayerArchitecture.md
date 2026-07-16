# StormBreaker — Multiplayer Architecture

## Network Topology

```
┌─────────────────────────────────────────────────────┐
│                  DEDICATED SERVER                    │
│  ┌─────────────────────────────────────────────┐    │
│  │         SBBattleRoyaleGameMode              │    │
│  │  • Match lifecycle (WaitingForPlayers →     │    │
│  │    WarmUp → PlanePhase → InProgress →       │    │
│  │    EndGame)                                  │    │
│  │  • Spawn / Elimination / Win condition       │    │
│  │  • Zone shrink timer                         │    │
│  │  • Loot table queries                        │    │
│  │  • Bot spawning                              │    │
│  └─────────────────────────────────────────────┘    │
│  ┌─────────────────────────────────────────────┐    │
│  │        SBBattleRoyaleGameState               │    │
│  │  (Replicated to ALL clients)                 │    │
│  │  • Zone data (center, radius, damage)        │    │
│  │  • Match phase                               │    │
│  │  • Alive player count                        │    │
│  │  • Match timer                               │    │
│  └─────────────────────────────────────────────┘    │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐          │
│  │PlayerState│  │PlayerState│  │PlayerState│  x100  │
│  │ Kills     │  │ Kills     │  │ Kills     │        │
│  │ Status    │  │ Status    │  │ Status    │        │
│  │ TeamId    │  │ TeamId    │  │ TeamId    │        │
│  │ ASC (GAS) │  │ ASC (GAS) │  │ ASC (GAS) │        │
│  └──────────┘  └──────────┘  └──────────┘          │
└──────────────────┬──────────────┬───────────────────┘
                   │              │
        ┌──────────┴──┐    ┌─────┴───────┐
        │   Client 1   │    │   Client N   │
        │ PlayerCtrl   │    │ PlayerCtrl   │
        │ SBCharacter  │    │ SBCharacter  │
        │ Local Input  │    │ Local Input  │
        │ HUD/UI       │    │ HUD/UI       │
        └──────────────┘    └──────────────┘
```

## Authority Model

| System | Runs On | Replicated? |
|--------|---------|-------------|
| Match phase transitions | Server only | Via GameState |
| Zone shrink | Server only | Via GameState |
| Player health/damage | Server authoritative | Via GAS + AttributeSet |
| Weapon fire (hitscan) | Client predicts → Server validates | Hit results replicated |
| Weapon fire (projectile) | Server spawns authoritative projectile | Projectile replicated |
| Inventory changes | Server only | Via PlayerState/Controller RPC |
| Loot spawning | Server only | Actors replicated |
| Vehicle physics | Server simulates → Client interpolates | Transform replicated |
| Kill feed | Server → Client RPC | Client_ShowKillFeed |
| Parachute/Jump | Client predicts → Server validates | Movement replicated |

## Replication Strategy

### Movement
- Use `CharacterMovementComponent` with built-in client prediction
- Custom movement modes: Swimming, Climbing, Parachuting
- `NetUpdateFrequency = 60` for owning client, `30` for others
- Smooth interpolation for remote characters

### Weapons
```
Client presses Fire
  → Client plays local effects (muzzle flash, sound) immediately
  → Client sends Server_FireWeapon RPC
  → Server validates (ammo, fire rate, state checks)
  → Server performs line trace (hitscan) or spawns projectile
  → Server applies damage via GAS
  → Server calls Multicast_PlayFireEffect for remote clients
```

### Damage Pipeline
```
Server receives hit event
  → Create GE_Damage GameplayEffect
  → Apply to target's AbilitySystemComponent
  → AttributeSet::PostGameplayEffectExecute processes:
      Shield absorbs first → remaining hits Health
  → Health <= 0 → Apply State.Downed tag
  → All teammates downed → Apply State.Dead tag → Eliminate
```

## EOS (Epic Online Services) Integration

```
┌─────────────┐     ┌──────────────────┐
│   Client     │────▶│   EOS Backend    │
│              │     │  • Auth (login)  │
│  Login ──────┼────▶│  • Sessions      │
│  Matchmake ──┼────▶│  • Matchmaking   │
│  Leaderboard─┼────▶│  • Stats/Ldbrd   │
│  Friends ────┼────▶│  • Friends       │
│  Voice ──────┼────▶│  • Voice (P2P)   │
└─────────────┘     └──────────────────┘
```

### Session Flow
1. Player logs in via EOS Auth (Epic/Device ID for mobile)
2. Player requests matchmaking via EOS Lobbies
3. EOS matches players by skill/region
4. Server creates session with `SBGameInstance::HostSession()`
5. Players travel to match map
6. Match begins when `MinPlayersToStart` reached

## Bandwidth Optimization

| Technique | Application |
|-----------|-------------|
| `COND_OwnerOnly` | Ammo count, inventory, ability cooldowns |
| `COND_SkipOwner` | Third-person animation pose, cosmetic effects |
| `COND_InitialOnly` | Team ID, player name, skin selection |
| NetCullDistance | Loot (5000), Vehicles (15000), Players (20000) |
| Relevancy | Loot actors not relevant beyond pickup range |
| Quantization | Rotators sent as compressed (short) |
| Delta compression | Default UE5 delta serialization |

## Scalability

- **100 players** per match target
- Server tick rate: **30 Hz** (configurable)
- Client send rate: **60 Hz** for owner, **30 Hz** observed
- Net priority: Nearby players > distant > dead
- Server runs headless (no rendering) for max capacity
