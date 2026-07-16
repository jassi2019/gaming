// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SBMultiplayerSubsystem.generated.h"

UENUM(BlueprintType)
enum class ESBLoginState : uint8
{
    NotLoggedIn,
    LoggingIn,
    LoggedIn,
    Failed
};

UENUM(BlueprintType)
enum class ESBSessionState : uint8
{
    Idle,
    Creating,
    Searching,
    Joining,
    InSession,
    Failed
};

USTRUCT(BlueprintType)
struct FSBSessionSearchResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SessionId;

    UPROPERTY(BlueprintReadOnly)
    FString HostName;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentPlayers = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 MaxPlayers = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 PingMs = 0;

    UPROPERTY(BlueprintReadOnly)
    FString MapName;

    int32 SearchResultIndex = -1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginStateChanged, ESBLoginState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionStateChanged, ESBSessionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchComplete, const TArray<FSBSessionSearchResult>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, bool, bSuccess);

/**
 * Manages EOS login, session creation, matchmaking, and lobby.
 * Wraps IOnlineSubsystem for the full multiplayer lifecycle.
 * Call from lobby UI to host, find, and join matches.
 */
UCLASS()
class STORMBREAKER_API USBMultiplayerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Login ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Multiplayer")
    void Login();

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Multiplayer")
    ESBLoginState GetLoginState() const { return LoginState; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Multiplayer")
    FString GetPlayerDisplayName() const;

    // --- Host ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Multiplayer")
    void HostSession(int32 MaxPlayers = 100, bool bIsDedicatedServer = false, const FString& MapName = TEXT("MainMap"));

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Multiplayer")
    void HostListenServer(int32 MaxPlayers = 4, const FString& MapName = TEXT("MainMap"));

    // --- Find & Join ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Multiplayer")
    void FindSessions(int32 MaxResults = 20);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Multiplayer")
    void JoinSession(int32 SearchResultIndex);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Multiplayer")
    void QuickMatch();

    // --- Leave ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Multiplayer")
    void LeaveSession();

    // --- State ---

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Multiplayer")
    ESBSessionState GetSessionState() const { return SessionState; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Multiplayer")
    bool IsInSession() const { return SessionState == ESBSessionState::InSession; }

    // --- Delegates ---

    UPROPERTY(BlueprintAssignable)
    FOnLoginStateChanged OnLoginStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnSessionStateChanged OnSessionStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnSessionSearchComplete OnSessionSearchComplete;

    UPROPERTY(BlueprintAssignable)
    FOnSessionJoined OnSessionJoined;

private:
    void SetLoginState(ESBLoginState NewState);
    void SetSessionState(ESBSessionState NewState);

    IOnlineSessionPtr GetSessionInterface() const;

    void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

    ESBLoginState LoginState;
    ESBSessionState SessionState;
    TSharedPtr<FOnlineSessionSearch> SessionSearch;
    TArray<FSBSessionSearchResult> CachedSearchResults;
};
