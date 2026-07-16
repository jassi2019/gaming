# StormBreaker — Phase 4: Inventory, Loot & Battle Royale Core

## Architecture

```
FSBItemDefinition (DataTable row)     USBInventoryComponent
├── ItemID, Category, Rarity          ├── Items (TArray<FSBInventoryItem>)
├── WeaponData (if weapon)            ├── Equipment (Helmet/Vest/Backpack)
├── ConsumableType + heal/boost       ├── Consumable use (timed, cancellable)
├── ArmorSlot + level + durability    ├── Boost system (passive regen)
├── AttachmentSlot                    ├── Weight/capacity system
└── Weight, StackSize, Icon           └── Auto-loot settings

USBLootManager (WorldSubsystem)       ASBDeathCrate
├── Weighted random from DataTable    ├── Spawned on player death
├── GenerateBuildingLoot (2-5 items)  ├── Contains all player items
├── GenerateAirDropLoot (4 items)     ├── Loot interface for other players
└── RollSingleItem                   └── Self-destructs when empty

USBKnockReviveComponent
├── Alive → Knocked → Dead → Spectating
├── Bleed-out timer (90s)
├── Revive (10s, 300 unit range)
├── Death crate spawning
└── Team-mode aware
```

## Item Categories

| Category | Examples | Stack | Weight |
|----------|----------|-------|--------|
| Weapon | AK47, AWM, MP5 | 1 | 5.0 |
| Ammo | 5.56, 7.62, 9mm | 60 | 0.5 |
| Attachment | 4x Scope, Comp, ExtMag | 1 | 1.0 |
| Consumable | Bandage, MedKit, Energy | 5-10 | 1.0 |
| Armor | Helmet L1/2/3, Vest L1/2/3 | 1 | 3.0 |
| Backpack | Backpack L1/2/3 | 1 | 0.0 |

## Consumable Specs

| Item | Heal | Boost | Time | HP Cap | Stack |
|------|------|-------|------|--------|-------|
| Bandage | 10 | 0 | 4s | 75 | 15 |
| First Aid Kit | 75 | 0 | 6s | 75 | 5 |
| Med Kit | 100 | 0 | 8s | 100 | 3 |
| Energy Drink | 0 | 40 | 4s | — | 5 |
| Painkiller | 0 | 60 | 6s | — | 5 |
| Adrenaline | 0 | 100 | 8s | — | 2 |

## Boost System

| Boost Level | HP Regen/s | Speed Bonus |
|-------------|-----------|-------------|
| 0-19 | 0 | 0% |
| 20-39 | 2 | 0% |
| 40-59 | 4 | 0% |
| 60-100 | 6 | 6.5% |

Boost decays at 3/s.

## Armor Specs

| Level | Damage Reduction | Durability (Helmet/Vest) |
|-------|-----------------|-------------------------|
| L1 | 30% | 80 / 200 |
| L2 | 40% | 150 / 220 |
| L3 | 55% | 230 / 250 |

Damage pipeline: Incoming → Helmet absorbs (if headshot) → Vest absorbs → Remaining hits Health.

## Backpack Capacity

| Level | Bonus Capacity | Total |
|-------|---------------|-------|
| None | 0 | 200 |
| L1 | +150 | 350 |
| L2 | +200 | 400 |
| L3 | +250 | 450 |

## Knock/Revive System

| State | Duration | Rules |
|-------|----------|-------|
| Knocked | 90s bleed-out | Prone only, can crawl at 50 cm/s |
| Revive | 10s | Reviver must stay within 300 units |
| Death | Immediate | Spawns death crate, enters spectator after 3s |

- Solo mode: knock system disabled, instant death
- Team mode: knocked teammates can be revived
- Cancellation: revive cancels if reviver moves >300u away or takes damage

## Blueprint Setup

### 1. Item DataTable

Create DataTable `DT_Items` with row struct `FSBItemDefinition` in `Content/Loot/DataTables/`:

Add rows for all items (weapons, ammo, consumables, armor, backpacks, attachments).

### 2. Loot DataTable

Create DataTable `DT_DefaultLoot` with row struct `FSBLootTableEntry`:
- Assign `SpawnWeight` per item (higher = more common)
- Set `MinCount`/`MaxCount` ranges

Create `DT_AirDropLoot` with higher-rarity items.

### 3. Assign to Components

In `BP_SBCharacter`:
- InventoryComponent → Set `Item Database` to `DT_Items`
- KnockReviveComponent → Set `Death Crate Class` to BP_DeathCrate
- KnockReviveComponent → Set `bEnableKnockSystem` = true (team) / false (solo)

### 4. Testing Checklist

- [ ] Add item to inventory, verify weight/capacity
- [ ] Exceed capacity → AddItem returns false
- [ ] Equip L1 helmet, take headshot → damage reduced 30%
- [ ] Armor durability depletes, breaks at 0
- [ ] Use bandage → 4s timer, heals 10 HP, caps at 75
- [ ] Use energy drink → adds 40 boost, passive regen starts
- [ ] Sprint during consumable → cancels
- [ ] Health reaches 0 (team mode) → enters knocked state
- [ ] Teammate revives in 10s → restored to 30 HP
- [ ] Bleed-out timer expires → player dies
- [ ] Death crate spawns with all items
- [ ] Loot death crate → items transfer to looter's inventory
- [ ] Death crate disappears after 5 minutes or when empty
