// Copyright Island Of Death Games. All Rights Reserved.

#include "Core/SBAttributeSet.h"
#include "StormBreaker.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"

USBAttributeSet::USBAttributeSet()
{
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
    InitShield(0.0f);
    InitMaxShield(150.0f);
    InitStamina(100.0f);
    InitMaxStamina(100.0f);
    InitIncomingDamage(0.0f);
    InitIncomingHealing(0.0f);
}

void USBAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(USBAttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(USBAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(USBAttributeSet, Shield, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(USBAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(USBAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(USBAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
}

void USBAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // Clamp current values to their max
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetShieldAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
    }
    else if (Attribute == GetStaminaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
    }
}

void USBAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // Process incoming damage through shield first, then health
    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        float Damage = GetIncomingDamage();
        SetIncomingDamage(0.0f);

        if (Damage > 0.0f)
        {
            // Absorb with shield first
            float CurrentShield = GetShield();
            if (CurrentShield > 0.0f)
            {
                float ShieldAbsorb = FMath::Min(CurrentShield, Damage);
                SetShield(CurrentShield - ShieldAbsorb);
                Damage -= ShieldAbsorb;
            }

            // Remaining damage hits health
            if (Damage > 0.0f)
            {
                float NewHealth = FMath::Max(0.0f, GetHealth() - Damage);
                SetHealth(NewHealth);

                if (NewHealth <= 0.0f)
                {
                    UE_LOG(LogSBCharacter, Log, TEXT("Character health reached zero."));
                    // Death/downed handling triggered via GAS tag
                }
            }
        }
    }

    // Process incoming healing
    if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
    {
        float HealAmount = GetIncomingHealing();
        SetIncomingHealing(0.0f);

        if (HealAmount > 0.0f)
        {
            float NewHealth = FMath::Min(GetHealth() + HealAmount, GetMaxHealth());
            SetHealth(NewHealth);
        }
    }
}

// ----- RepNotify -----

void USBAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(USBAttributeSet, Health, OldHealth);
}

void USBAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(USBAttributeSet, MaxHealth, OldMaxHealth);
}

void USBAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(USBAttributeSet, Shield, OldShield);
}

void USBAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(USBAttributeSet, MaxShield, OldMaxShield);
}

void USBAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(USBAttributeSet, Stamina, OldStamina);
}

void USBAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(USBAttributeSet, MaxStamina, OldMaxStamina);
}
