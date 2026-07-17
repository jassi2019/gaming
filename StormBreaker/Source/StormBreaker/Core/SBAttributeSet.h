// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "SBAttributeSet.generated.h"

// Macros for attribute accessors (standard GAS pattern)
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Core attribute set for all characters.
 * Health, Shield, Stamina, Speed modifiers.
 */
UCLASS()
class STORMBREAKER_API USBAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    USBAttributeSet();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    // ----- Health -----

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "IslandOfDeath|Attributes")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(USBAttributeSet, Health)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "IslandOfDeath|Attributes")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(USBAttributeSet, MaxHealth)

    // ----- Shield (Armor) -----

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Shield, Category = "IslandOfDeath|Attributes")
    FGameplayAttributeData Shield;
    ATTRIBUTE_ACCESSORS(USBAttributeSet, Shield)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxShield, Category = "IslandOfDeath|Attributes")
    FGameplayAttributeData MaxShield;
    ATTRIBUTE_ACCESSORS(USBAttributeSet, MaxShield)

    // ----- Stamina -----

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "IslandOfDeath|Attributes")
    FGameplayAttributeData Stamina;
    ATTRIBUTE_ACCESSORS(USBAttributeSet, Stamina)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "IslandOfDeath|Attributes")
    FGameplayAttributeData MaxStamina;
    ATTRIBUTE_ACCESSORS(USBAttributeSet, MaxStamina)

    // ----- Meta Attribute (damage pipeline) -----

    UPROPERTY(BlueprintReadOnly, Category = "IslandOfDeath|Attributes")
    FGameplayAttributeData IncomingDamage;
    ATTRIBUTE_ACCESSORS(USBAttributeSet, IncomingDamage)

    UPROPERTY(BlueprintReadOnly, Category = "IslandOfDeath|Attributes")
    FGameplayAttributeData IncomingHealing;
    ATTRIBUTE_ACCESSORS(USBAttributeSet, IncomingHealing)

protected:
    UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldHealth);

    UFUNCTION()
    void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

    UFUNCTION()
    void OnRep_Shield(const FGameplayAttributeData& OldShield);

    UFUNCTION()
    void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield);

    UFUNCTION()
    void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

    UFUNCTION()
    void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
};
