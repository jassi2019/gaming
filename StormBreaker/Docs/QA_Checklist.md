# StormBreaker — QA Checklist

Complete testing checklist for every gameplay system. Test each item in PIE.

## Pre-Test Setup
- [ ] Open any level (even empty) in UE5.8 Editor
- [ ] Press Play — arena auto-spawns (ground + cover + PlayerStarts + ZoneManager)
- [ ] Character spawns at a PlayerStart
- [ ] HUD displays (health, shield, ammo, compass, minimap, alive count)

---

## 1. Character Movement
- [ ] WASD forward/back/strafe at 500 cm/s
- [ ] Sprint (Left Shift hold) at 800 cm/s
- [ ] Jump (Space) — 500 Z velocity
- [ ] Crouch (C toggle) — capsule shrinks, speed 200 cm/s
- [ ] Prone (Z toggle) — capsule 30cm, speed 100 cm/s
- [ ] Cannot crouch while prone, cannot prone while in air
- [ ] Jump exits prone state
- [ ] Camera follows character smoothly with lag

## 2. Vault & Mantle
- [ ] Jump near low wall (~80cm) → vault over (0.4s arc)
- [ ] Jump near medium wall (~180cm) → mantle up (0.8s climb)
- [ ] Jump near tall wall (>250cm) → normal jump (no mantle)
- [ ] Cannot vault/mantle while prone

## 3. ADS (Aim Down Sights)
- [ ] Right-click → camera zooms in (300→100 boom, 90→65 FOV)
- [ ] Character rotates to face camera direction
- [ ] Movement speed drops to 200 cm/s
- [ ] Sprint cancels on ADS
- [ ] Release right-click → camera returns to default

## 4. Weapon System
- [ ] AK-47 auto-equipped on spawn
- [ ] Left-click fires (auto mode by default)
- [ ] Recoil kicks camera up per shot
- [ ] Spread increases during sustained fire
- [ ] Spread recovers when not firing
- [ ] R reloads (consumes reserve ammo)
- [ ] Dry fire sound when magazine empty
- [ ] Auto-reload on empty fire attempt
- [ ] Ammo display: "30 / 120" format
- [ ] Fire mode displayed: "AUTO"

## 5. HUD Elements
- [ ] Health bar (green, bottom-left) — shows 100/100
- [ ] Shield bar (blue, bottom-left) — shows 0/150
- [ ] Boost bar (segmented, bottom-left)
- [ ] Weapon name (bottom-right) — "AK-47"
- [ ] Fire mode (bottom-right) — "AUTO"
- [ ] Ammo count (bottom-right) — "30 / 120"
- [ ] Crosshair (center) — 4 lines + dot
- [ ] Compass (top-center) — N/S/E/W + bearing
- [ ] Minimap (top-right) — player dot + zone circles
- [ ] Alive count (top-right) — shows total alive
- [ ] FPS counter (bottom-left)
- [ ] Ping display (bottom-left)

## 6. Zone System
- [ ] Zone manager spawns automatically
- [ ] Zone phases start after 10s delay
- [ ] Target circle (white) visible on minimap
- [ ] Current circle (blue) visible on minimap
- [ ] Zone shrinks smoothly
- [ ] Zone timer shows countdown
- [ ] Damage applied outside safe zone
- [ ] Damage increases per phase

## 7. AI Bots
- [ ] Bots spawn (default: 5)
- [ ] Bots have weapons equipped
- [ ] Bots move/patrol when idle
- [ ] Bots engage player on sight
- [ ] Bots fire in bursts
- [ ] Bots flee when low HP
- [ ] Bots move toward safe zone
- [ ] Bot AI LOD — distant bots tick slower
- [ ] Alive count decreases as bots die

## 8. Inventory & Equipment
- [ ] InventoryComponent exists on character
- [ ] Weight/capacity system functional
- [ ] Equipment slots (helmet/vest/backpack) exist

## 9. Knock/Revive/Death
- [ ] KnockReviveComponent exists on character
- [ ] Health reaching 0 → knocked state (team mode)
- [ ] Death crate spawns with items
- [ ] Spectator mode after death

## 10. Match Flow
- [ ] Press Play → character spawns → match starts immediately
- [ ] Alive count updates as bots die
- [ ] Zone phases progress automatically
- [ ] Last player standing → match ends
- [ ] "WINNER WINNER CHICKEN DINNER" logged
- [ ] Match results submitted to backend
- [ ] XP + coins + rank points calculated

## 11. Backend
- [ ] Guest login auto-creates profile
- [ ] Match results persist to local JSON save
- [ ] Profile level/XP update after match
- [ ] Rank points update after match
- [ ] Save file at: Saved/StormBreaker_SaveData.json

## 12. Performance
- [ ] Stable 60 FPS in editor on PC
- [ ] No visible hitching during combat
- [ ] Memory usage reasonable
- [ ] Object pool subsystem initialized

## 13. Multiplayer (2-Player PIE)
- [ ] Set Number of Players: 2, Net Mode: Listen Server
- [ ] Both players spawn
- [ ] Both players see each other
- [ ] Movement replicates smoothly
- [ ] Weapons fire replicates (muzzle flash, sound)
- [ ] Damage syncs correctly
- [ ] Alive count syncs
- [ ] Zone visible for both clients

## 14. Map Configurator
- [ ] Ground plane (600m x 600m) spawns
- [ ] 40 cover structures spawn (mix of low/medium/tall)
- [ ] 4 PlayerStart actors spawn in circle
- [ ] ZoneManager spawns with 30km radius
- [ ] All cover has collision

## 15. Mobile (Android Build)
- [ ] Touch controls component exists
- [ ] Gyroscope sensitivity configurable
- [ ] Peek left/right functional
- [ ] Healing wheel opens/closes
- [ ] Dynamic resolution enabled
- [ ] Quality auto-adapts
