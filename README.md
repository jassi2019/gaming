# Island of Death — Battle Royale

A complete Battle Royale game built with **Unreal Engine 5.8**, targeting **Android**, **iOS**, and **Windows**.

All original assets, branding, characters, maps, UI, sounds, and mechanics. No copyrighted content.

> *"Some Spirits Don't seek revenge... they seek survival"*

---

## Screenshots

### Login Screen
Dark cinematic login with Google, Facebook, Apple, and Guest options.

### Loading Screen
Graphics quality selector (Smooth → Ultra HD) with resource download progress.

### Lobby (BGMI-Style)
Full BGMI-style main menu — player profile, VIP badge, currency, left navigation menu (Start/Loadout/Character/Inventory/Missions/Season/Clan/Events/Store), right panels (Rank Push, Daily Bundle, Events, Battle Royale mode), chat bar, bottom tabs, and golden START button.

### In-Game HUD
Health/Shield bars, weapon info, crosshair, compass, minimap, alive count, FPS/ping.

---

## Game Flow

```
Splash Video → Login Screen → Loading (Quality + Download) → Lobby → Match
```

All transitions use smooth fade-through-black (0.6–1.2s). Mouse clicks are blocked during transitions to prevent accidental presses.

---

## Tech Stack

| Technology | Usage |
|-----------|-------|
| Unreal Engine 5.8 | Game engine |
| C++ | Core gameplay systems (~15,000+ lines) |
| Blueprints | Visual scripting layer |
| Enhanced Input System | Input handling (mobile + KB/M) |
| GAS (Gameplay Ability System) | Abilities, damage, health, effects |
| EOS (Epic Online Services) | Multiplayer sessions, matchmaking |
| Niagara | VFX (muzzle flash, explosions, impacts) |
| Chaos Vehicles | Vehicle physics |
| Canvas HUD | Full UI system (no Widget Blueprints needed) |

---

## Development Phases

| Phase | Module | Status |
|-------|--------|--------|
| 1 | Project Architecture | ✅ Complete |
| 2 | Third Person Character | ✅ Complete |
| 3 | Weapon & Combat System | ✅ Complete |
| 4 | Inventory, Loot & BR Core | ✅ Complete |
| 5 | Battle Royale Match Flow | ✅ Complete |
| 6 | Multiplayer & Networking | ✅ Complete |
| 7 | AI Bots (99-bot BR) | ✅ Complete |
| 8 | Production UI (BGMI-style) | ✅ Complete |
| 9 | Mobile Optimization | ✅ Complete |
| 10 | Backend & Live Service | ✅ Complete |
| 11 | World Building (Procedural Island) | ✅ Complete |
| 12 | QA & Bug Fixes | ✅ Complete (74/74 tests passed) |
| 13 | Game Flow & UI Screens | ✅ Complete |

---

## UI Screens (Phase 13)

### 1. Splash Screen
- Cinematic dark background with lightning flashes
- "ISLAND OF DEATH" title fade-in with glow
- Tagline + loading bar
- Fade-from-black and fade-to-black transitions

### 2. Login Screen
- Dark red-tinted cinematic background
- Red corner accent frame
- "ISLAND / OF / DEATH" styled title (white + blood red)
- Tagline with red highlight
- **Continue with Google** (white button)
- **Continue with Facebook** (blue button)
- **Continue with Apple** (dark button)
- **OR** separator
- **PLAY AS GUEST** (red bordered, with arrow)
- Guest disclaimer text

### 3. Loading Screen (Graphics Quality + Resource Download)
- Animated floating particles
- **5 Quality Tiers:**
  - SMOOTH (320 MB) — Best performance
  - BALANCED (520 MB) — Good visuals & performance
  - HD (680 MB) — High quality textures
  - HDR (850 MB) — HDR lighting + high textures
  - ULTRA HD (1.2 GB) — Maximum quality, 4K textures
- Radio selector with color-coded options
- **DOWNLOAD RESOURCES** button
- Live download progress bar with speed (MB/s) and ETA
- **ENTER LOBBY** button after download completes
- **SKIP >>** button for testing

### 4. Lobby Screen (BGMI-Style)
- **Top Bar:** Avatar, player name, level, XP bar, VIP badge, BP/UC currency, settings, ping
- **Game Logo Panel:** "ISLAND OF DEATH" with tagline (top-left)
- **Left Navigation Menu:**
  - START (gold highlight, selected)
  - LOADOUT
  - CHARACTER
  - INVENTORY
  - MISSIONS (red notification dot)
  - SEASON (red notification dot)
  - CLAN
  - EVENTS (NEW badge)
  - STORE
- **Right Panels:**
  - Rank Push — Season 12, RP level 50, progress bar
  - Daily Special Bundle — -70% badge, character slots
  - Events — Zombie Survival LIVE NOW
  - Battle Royale — Classic, TPP, Solo/Duo/Squad, Bot count
- **Chat Bar:** World chat at bottom
- **Bottom Tabs:** RANK, ACHIEVEMENTS, WORKSHOP, SOCIAL, FIRST TOP-UP
- **START Button:** Large golden button (bottom-right) with hover glow

### 5. In-Game HUD
- Health bar (green) + Shield bar (blue)
- Weapon name + fire mode + ammo count
- Crosshair (4-line + dot)
- Compass with cardinal directions + degree heading
- Minimap (top-right)
- Alive player count
- FPS + Ping display

---

## Core Systems

### Character Movement
| Action | Speed | Key | Network |
|--------|-------|-----|---------|
| Walk | 250 cm/s | WASD (slow) | CMC built-in |
| Run | 500 cm/s | WASD | CMC built-in |
| Sprint | 800 cm/s | Left Shift | FLAG_Custom_0 |
| Jump | 500 Z vel | Space | CMC built-in |
| Crouch | 200 cm/s | C (toggle) | CMC built-in |
| Prone | 100 cm/s | Z (toggle) | FLAG_Custom_1 |
| Vault | Auto | Space (near obstacle) | Server authority |
| Mantle | Auto | Space (near ledge) | Server authority |
| ADS | 200 cm/s | Right Mouse | FLAG_Custom_2 |

### Weapons
| Weapon | Type | RPM | Damage | Range | Mag | Special |
|--------|------|-----|--------|-------|-----|---------|
| AK47 | AR | 600 | 36 | 4000cm | 30 | 1 pen, Auto/Single |
| MP5 | SMG | 800 | 24 | 2500cm | 25 | Low recoil |
| AWM | Sniper | 45 | 120 | 10000cm | 5 | 3x headshot |
| S12 | Shotgun | 120 | 18×8 | 1000cm | 8 | 8 pellets |
| M9 | Pistol | 400 | 30 | 3000cm | 15 | Single only |
| Frag | Grenade | — | 150 | 500r | 1+4 | Radial damage |
| Knife | Melee | 120 | 50 | 200cm | — | 60° arc |

### Battle Royale Zone (8 Phases)
- Phase 1–3: 0.4–1.0 DPS, 300–150s wait, 50% radius reduction
- Phase 4–6: 2.0–5.0 DPS, 120–60s wait, increasing intensity
- Phase 7–8: 8.0–14.0 DPS, 30s wait, final collapse to 0

### Backend & Live Service
- Guest + EOS authentication
- Player profiles (level, XP, rank, coins, gems)
- Match history, leaderboards
- Season pass with free/premium tracks
- Daily shop rotation
- Push notifications, friend system

---

## QA Results

| Category | Result |
|----------|--------|
| Tests Passed | 74 |
| Tests Failed | 0 |
| Bugs Fixed | 12 (10 code + 2 config) |
| Crashes Found | 0 |
| Build Targets | 3/3 pass (Editor, Game, Shipping) |
| Runtime Errors | 0 |
| Verdict | **PRODUCTION READY** |

---

## Folder Structure

```
StormBreaker/
├── Config/
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   ├── DefaultInput.ini
│   └── DefaultScalability.ini
├── Content/
│   ├── Characters/
│   ├── Weapons/
│   ├── Maps/
│   ├── Environment/
│   ├── Vehicles/
│   ├── UI/
│   ├── Effects/
│   ├── Audio/
│   ├── Loot/
│   ├── Splash/          — Splash screens (BMP/PNG, all sizes)
│   └── DataTables/
├── Source/
│   ├── StormBreaker/
│   │   ├── Core/        — GameMode, GameState, PlayerState, Controller
│   │   ├── Character/   — Movement, animation, mobile touch
│   │   ├── Weapon/      — Weapons, projectiles, pickups
│   │   ├── Inventory/   — Backpack, equipment, consumables
│   │   ├── BattleRoyale/ — Zone, aircraft, parachute, airdrops
│   │   ├── Multiplayer/ — Sessions, matchmaking, voice
│   │   ├── AI/          — Bot controller, behavior tree
│   │   ├── UI/          — Game flow HUD (all screens)
│   │   ├── Vehicle/     — Cars, boats, motorcycles
│   │   ├── Backend/     — Auth, profiles, leaderboards, shop
│   │   └── Subsystems/  — Settings, services
│   └── StormBreakerServer/
├── Docs/
└── StormBreaker.uproject
```

---

## How to Build

1. Install **Unreal Engine 5.8** via Epic Games Launcher
2. Clone this repository: `git clone https://github.com/jassi2019/gaming.git`
3. Open `StormBreaker/StormBreaker.uproject`
4. Click **Play** — game starts with Login → Loading → Lobby → Match flow
5. For packaging: File → Package Project → Windows/Android

## Performance Targets

| Metric | Android (Mid) | Android (High) | Windows |
|--------|--------------|----------------|---------|
| FPS | 30 stable | 60 stable | 60–120 |
| Draw Calls | < 500 | < 800 | < 2000 |
| RAM | < 2 GB | < 3 GB | < 6 GB |

---

## License

All rights reserved. Island of Death Games.
