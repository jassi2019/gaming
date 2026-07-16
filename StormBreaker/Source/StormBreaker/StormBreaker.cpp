// Copyright StormBreaker Games. All Rights Reserved.

#include "StormBreaker.h"
#include "Modules/ModuleManager.h"
#include "GameplayTagsManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, StormBreaker, "StormBreaker");

DEFINE_LOG_CATEGORY(LogStormBreaker);
DEFINE_LOG_CATEGORY(LogSBWeapon);
DEFINE_LOG_CATEGORY(LogSBCharacter);
DEFINE_LOG_CATEGORY(LogSBInventory);
DEFINE_LOG_CATEGORY(LogSBBattleRoyale);
DEFINE_LOG_CATEGORY(LogSBMultiplayer);
DEFINE_LOG_CATEGORY(LogSBAI);
DEFINE_LOG_CATEGORY(LogSBVehicle);
DEFINE_LOG_CATEGORY(LogSBBackend);

namespace SBTags
{
    static FGameplayTag FindOrAdd(const FName& TagName)
    {
        return UGameplayTagsManager::Get().AddNativeGameplayTag(TagName, TEXT(""));
    }

    const FGameplayTag& State_Dead()
    {
        static FGameplayTag Tag = FindOrAdd(FName("State.Dead"));
        return Tag;
    }

    const FGameplayTag& State_Downed()
    {
        static FGameplayTag Tag = FindOrAdd(FName("State.Downed"));
        return Tag;
    }

    const FGameplayTag& State_Sprinting()
    {
        static FGameplayTag Tag = FindOrAdd(FName("State.Sprinting"));
        return Tag;
    }

    const FGameplayTag& State_ADS()
    {
        static FGameplayTag Tag = FindOrAdd(FName("State.ADS"));
        return Tag;
    }

    const FGameplayTag& State_Swimming()
    {
        static FGameplayTag Tag = FindOrAdd(FName("State.Swimming"));
        return Tag;
    }

    const FGameplayTag& State_Driving()
    {
        static FGameplayTag Tag = FindOrAdd(FName("State.Driving"));
        return Tag;
    }

    const FGameplayTag& State_Parachuting()
    {
        static FGameplayTag Tag = FindOrAdd(FName("State.Parachuting"));
        return Tag;
    }

    const FGameplayTag& Slot_Primary()
    {
        static FGameplayTag Tag = FindOrAdd(FName("Slot.Primary"));
        return Tag;
    }

    const FGameplayTag& Slot_Secondary()
    {
        static FGameplayTag Tag = FindOrAdd(FName("Slot.Secondary"));
        return Tag;
    }

    const FGameplayTag& Slot_Sidearm()
    {
        static FGameplayTag Tag = FindOrAdd(FName("Slot.Sidearm"));
        return Tag;
    }

    const FGameplayTag& Slot_Melee()
    {
        static FGameplayTag Tag = FindOrAdd(FName("Slot.Melee"));
        return Tag;
    }

    const FGameplayTag& Slot_Throwable()
    {
        static FGameplayTag Tag = FindOrAdd(FName("Slot.Throwable"));
        return Tag;
    }
}
