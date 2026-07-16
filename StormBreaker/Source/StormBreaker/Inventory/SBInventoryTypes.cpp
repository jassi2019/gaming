// Copyright StormBreaker Games. All Rights Reserved.

#include "Inventory/SBInventoryTypes.h"

float FSBEquipmentState::GetDamageReduction(ESBArmorSlot Slot) const
{
    switch (Slot)
    {
    case ESBArmorSlot::Helmet:
    {
        switch (HelmetLevel)
        {
        case ESBArmorLevel::Level1: return 0.30f;
        case ESBArmorLevel::Level2: return 0.40f;
        case ESBArmorLevel::Level3: return 0.55f;
        default: return 0.0f;
        }
    }
    case ESBArmorSlot::Vest:
    {
        switch (VestLevel)
        {
        case ESBArmorLevel::Level1: return 0.30f;
        case ESBArmorLevel::Level2: return 0.40f;
        case ESBArmorLevel::Level3: return 0.55f;
        default: return 0.0f;
        }
    }
    default: return 0.0f;
    }
}

float FSBEquipmentState::AbsorbDamage(float IncomingDamage, ESBArmorSlot Slot)
{
    float Reduction = GetDamageReduction(Slot);
    if (Reduction <= 0.0f) return IncomingDamage;

    float Absorbed = IncomingDamage * Reduction;
    float Remaining = IncomingDamage - Absorbed;

    // Reduce durability
    float* Durability = nullptr;
    switch (Slot)
    {
    case ESBArmorSlot::Helmet: Durability = &HelmetDurability; break;
    case ESBArmorSlot::Vest: Durability = &VestDurability; break;
    default: break;
    }

    if (Durability)
    {
        *Durability = FMath::Max(0.0f, *Durability - Absorbed);
        if (*Durability <= 0.0f)
        {
            // Armor broken
            switch (Slot)
            {
            case ESBArmorSlot::Helmet:
                HelmetLevel = ESBArmorLevel::None;
                HelmetItemID = NAME_None;
                break;
            case ESBArmorSlot::Vest:
                VestLevel = ESBArmorLevel::None;
                VestItemID = NAME_None;
                break;
            default: break;
            }
        }
    }

    return Remaining;
}
