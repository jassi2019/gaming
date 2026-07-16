// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogStormBreaker, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSBWeapon, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSBCharacter, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSBInventory, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSBBattleRoyale, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSBMultiplayer, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSBAI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSBVehicle, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSBBackend, Log, All);

// Collision channels
#define ECC_Bullet          ECollisionChannel::ECC_GameTraceChannel1
#define ECC_Projectile      ECollisionChannel::ECC_GameTraceChannel2
#define ECC_Interaction     ECollisionChannel::ECC_GameTraceChannel3
#define ECC_Vehicle         ECollisionChannel::ECC_GameTraceChannel4

// Gameplay tags — defined once, used everywhere
namespace SBTags
{
    // Character states
    STORMBREAKER_API const FGameplayTag& State_Dead();
    STORMBREAKER_API const FGameplayTag& State_Downed();
    STORMBREAKER_API const FGameplayTag& State_Sprinting();
    STORMBREAKER_API const FGameplayTag& State_ADS();
    STORMBREAKER_API const FGameplayTag& State_Swimming();
    STORMBREAKER_API const FGameplayTag& State_Driving();
    STORMBREAKER_API const FGameplayTag& State_Parachuting();

    // Weapon slots
    STORMBREAKER_API const FGameplayTag& Slot_Primary();
    STORMBREAKER_API const FGameplayTag& Slot_Secondary();
    STORMBREAKER_API const FGameplayTag& Slot_Sidearm();
    STORMBREAKER_API const FGameplayTag& Slot_Melee();
    STORMBREAKER_API const FGameplayTag& Slot_Throwable();
}
