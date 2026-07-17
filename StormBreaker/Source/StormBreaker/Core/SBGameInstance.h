// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SBGameInstance.generated.h"

/**
 * Root game instance — persists across map travel.
 * Owns online session lifecycle, player profile data, and global settings.
 */
UCLASS()
class STORMBREAKER_API USBGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    USBGameInstance();

    virtual void Init() override;
    virtual void Shutdown() override;

    // ----- Online Session -----

    /** Create or find a match and travel to it. */
    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Online")
    void HostSession(int32 MaxPlayers, bool bIsDedicatedServer);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Online")
    void FindAndJoinSession();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Online")
    void DestroyCurrentSession();

    // ----- Player Identity -----

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Player")
    FString GetLocalPlayerDisplayName() const;

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Player")
    void SetLocalPlayerDisplayName(const FString& InName);

    // ----- Quality Settings -----

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Settings")
    void ApplyQualityPreset(int32 PresetIndex);

protected:
    virtual void OnStart() override;

private:
    void InitOnlineSubsystem();

    void OnCreateSessionComplete(FName SessionName, bool bSuccessful);
    void OnFindSessionsComplete(bool bSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnDestroySessionComplete(FName SessionName, bool bSuccessful);

    UPROPERTY()
    FString LocalPlayerDisplayName;

    TSharedPtr<class FOnlineSessionSearch> SessionSearch;
};
