# StormBreaker — Phase 2: Character System Setup

## What Was Built

### C++ Classes

| Class | File | Purpose |
|-------|------|---------|
| `USBCharacterMovementComponent` | Character/SBCharacterMovementComponent | Custom CMC: Sprint, Prone, Mantle, Vault, ADS speed, full network prediction |
| `ASBCharacterBase` | Character/SBCharacterBase | Main character: Enhanced Input, third-person camera with ADS, all movement states |
| `USBCharacterAnimInstance` | Character/SBCharacterAnimInstance | Anim instance: speed, direction, aim offset, lean, foot IK, all state flags |
| `USBMobileTouchWidget` | UI/SBMobileTouchWidget | Mobile touch: virtual joystick, look zone, action button delegates |

### Movement Capabilities
- **Walk** — 250 cm/s (base speed)
- **Run** — 500 cm/s (default movement)
- **Sprint** — 800 cm/s (hold shift, drains stamina)
- **Jump** — 500 Z velocity, auto-attempts vault/mantle before jumping
- **Crouch** — 200 cm/s, toggle via C key
- **Prone** — 100 cm/s, capsule reduced to 30 cm half-height, toggle via Z key
- **Vault** — obstacles up to 100 cm, 0.4s parabolic arc
- **Mantle** — ledges up to 250 cm, 0.8s climb (60% vertical, 40% horizontal)
- **Swimming** — 300 cm/s, uses built-in swimming mode
- **ADS** — 200 cm/s, character faces camera direction

### Network Prediction
All custom movement states are predicted via `FSBSavedMove`:
- `bWantsToSprint` → FLAG_Custom_0
- `bIsProning` → FLAG_Custom_1
- `bIsAiming` → FLAG_Custom_2

`bIsAiming` is also replicated on the character with `COND_SkipOwner` for third-person visual updates.

---

## Blueprint Setup Instructions

### 1. Create Input Assets (Content/Input/)

#### Input Actions
Create these Input Action assets in `Content/Input/`:

| Asset | Value Type | Description |
|-------|-----------|-------------|
| `IA_Move` | Axis2D (Vector2D) | WASD / Left joystick |
| `IA_Look` | Axis2D (Vector2D) | Mouse delta / Right touch zone |
| `IA_Jump` | Digital (Bool) | Space bar / Jump button |
| `IA_Sprint` | Digital (Bool) | Left Shift (hold) / Sprint button |
| `IA_Crouch` | Digital (Bool) | C (toggle) / Crouch button |
| `IA_Prone` | Digital (Bool) | Z (toggle) / Prone button |
| `IA_ADS` | Digital (Bool) | Right Mouse (hold) / ADS button |
| `IA_Interact` | Digital (Bool) | F / Interact button |
| `IA_Fire` | Digital (Bool) | Left Mouse / Fire button |
| `IA_Reload` | Digital (Bool) | R / Reload button |
| `IA_Inventory` | Digital (Bool) | Tab / Inventory button |
| `IA_Map` | Digital (Bool) | M / Map button |

#### Input Mapping Context
Create `IMC_Default` in `Content/Input/`:

**Keyboard/Mouse Bindings:**
| Action | Key | Modifiers |
|--------|-----|-----------|
| IA_Move | W/A/S/D | Negate (S, A), Swizzle YXZ |
| IA_Look | Mouse Delta XY | Negate Y for pitch |
| IA_Jump | Space | — |
| IA_Sprint | Left Shift | — |
| IA_Crouch | C | — |
| IA_Prone | Z | — |
| IA_ADS | Right Mouse Button | — |
| IA_Fire | Left Mouse Button | — |
| IA_Interact | F | — |
| IA_Reload | R | — |
| IA_Inventory | Tab | — |
| IA_Map | M | — |

**IA_Move Detailed Setup:**
1. Create IA_Move with Value Type = Axis2D
2. In IMC_Default, add 4 key mappings for IA_Move:
   - W → Modifiers: Swizzle Input Axis Values (YXZ)
   - S → Modifiers: Swizzle Input Axis Values (YXZ), Negate
   - D → (no modifiers needed)
   - A → Modifiers: Negate

### 2. Create Character Blueprint

1. Right-click → Blueprint Class → Parent: `SBCharacterBase`
2. Name: `BP_SBCharacter`
3. Save to: `Content/Characters/Player/Blueprints/`
4. Open BP_SBCharacter:
   - **Details panel:** Set all `IA_*` references to the Input Action assets
   - **Details panel:** Set `Default Mapping Context` to `IMC_Default`
   - **Mesh component:** Assign a skeletal mesh (placeholder or final)
   - **Camera settings:** Adjust `DefaultBoomLength`, `ADSBoomLength`, `ADSFOV` as desired

### 3. Assign Character to Game Mode

1. Open `BP_SBBattleRoyaleGameMode`
2. Set **Default Pawn Class** to `BP_SBCharacter`
3. Open `BP_SBGameModeBase` (lobby mode)
4. Set **Default Pawn Class** to `BP_SBCharacter`

### 4. Create Animation Blueprint

1. Right-click → Animation → Animation Blueprint
2. Parent: `SBCharacterAnimInstance`
3. Skeleton: Select the character's skeleton
4. Name: `ABP_SBCharacter`
5. Save to: `Content/Characters/Player/Animations/`

#### Animation Graph Setup:

**State Machine: Locomotion**
```
Entry → Idle/Walk/Run → Sprint
  ↓           ↓
  Jump     Crouch
  ↓           ↓
  Fall     Prone
  ↓
  Land
```

States:
- **Idle**: Play when `Speed < 3`
- **Walk/Run**: Blend Space 1D by `Speed` (0-500)
- **Sprint**: Play when `bIsSprinting` && `Speed > 500`
- **Crouch Idle/Move**: Blend Space by `Speed` when `bIsCrouching`
- **Prone Idle/Move**: When `bIsProning`
- **Jump**: When `bIsInAir` && not falling
- **Fall**: When `bIsFalling`
- **Land**: Transition from Fall to ground
- **Mantle**: When `bIsMantling`
- **Vault**: When `bIsVaulting`
- **Swim**: When `bIsSwimming`

**Aim Offset:**
- Create AimOffset asset with `AimPitch` and `AimYaw`
- Layer this over the locomotion output
- Use when `bIsAiming` is true

**Lean:**
- Use `LeanAmount` to add additive lean in the upper body
- Apply via layered blend per bone

**Foot IK:**
- Use `LeftFootIKOffset`, `RightFootIKOffset`, `HipOffset`
- Apply Two Bone IK nodes for each foot
- Apply hip offset to pelvis bone

### 5. Assign Animation Blueprint to Character

1. Open `BP_SBCharacter`
2. Select the **Mesh** component
3. Set **Anim Class** to `ABP_SBCharacter`

### 6. Mobile Touch Widget (Android)

1. Create Widget Blueprint: `WBP_MobileTouch`
2. Parent: `SBMobileTouchWidget`
3. Save to: `Content/UI/Common/Widgets/`
4. Design layout:
   ```
   ┌────────────────────────────────────────────┐
   │  [Sprint]                    [ADS] [Fire]  │
   │                                            │
   │  ┌─────┐                         [Reload] │
   │  │Move │                                   │
   │  │Stick│         Look Zone        [Crouch] │
   │  └─────┘                          [Prone]  │
   │                                            │
   │  [Jump]  [Interact]                        │
   └────────────────────────────────────────────┘
   ```
5. Bind button events to the corresponding `On*Pressed` / `On*Released` functions
6. Bind joystick to `SetMoveInput()` with normalized Vector2D
7. Bind touch zone to `SetLookInput()` with delta position

8. Open `BP_SBPlayerController`
9. Set **Mobile Touch Widget Class** to `WBP_MobileTouch`

### 7. Test Map Setup

1. Open `Content/Maps/TestMap/TestMap`
2. Place the following:
   - **Floor:** Large plane (scale 100x100)
   - **Ramp:** Angled static mesh for testing sprint/slope
   - **Low Wall (80cm):** For vault testing
   - **Medium Wall (180cm):** For mantle testing
   - **Tall Wall (300cm):** Should NOT be mantleable
   - **Pool/Water Volume:** For swim testing
   - **Player Start:** At least 2 for multiplayer testing
   - **Nav Mesh Bounds Volume:** Cover the play area
3. World Settings → GameMode Override → `BP_SBBattleRoyaleGameMode`
4. Set lighting: Directional Light + Sky Atmosphere + Sky Light

### 8. Testing Checklist

- [ ] **Walk/Run:** WASD movement at 500 cm/s
- [ ] **Sprint:** Hold Shift → character moves at 800 cm/s
- [ ] **Jump:** Space bar → character jumps 500 units
- [ ] **Crouch:** C toggles crouch, reduced speed and capsule
- [ ] **Prone:** Z toggles prone, very low capsule (30 cm), slow movement
- [ ] **Prone exit blocked:** Cannot stand if ceiling is too low
- [ ] **Vault:** Jump near 80cm wall → character vaults over (0.4s)
- [ ] **Mantle:** Jump near 180cm wall → character climbs up (0.8s)
- [ ] **Tall wall:** Jump near 300cm wall → normal jump (no mantle)
- [ ] **Swimming:** Enter water volume → swim mode activates
- [ ] **ADS:** Right-click → camera zooms in, character faces aim direction
- [ ] **Camera:** Smooth boom + lag, collision detection works
- [ ] **Animation:** All states reflected in anim blueprint
- [ ] **Multiplayer:** Two PIE clients see each other's movement states
- [ ] **Sprint replicates:** Other client sees sprint animation
- [ ] **Prone replicates:** Other client sees prone pose
- [ ] **ADS replicates:** Other client sees aim pose (COND_SkipOwner)
- [ ] **Mobile (Android):** Touch widget appears, joystick works, buttons fire events
