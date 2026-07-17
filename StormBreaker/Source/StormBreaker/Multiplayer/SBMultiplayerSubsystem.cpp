// Copyright Island Of Death Games. All Rights Reserved.

#include "Multiplayer/SBMultiplayerSubsystem.h"
#include "StormBreaker.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"

void USBMultiplayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoginState = ESBLoginState::NotLoggedIn;
    SessionState = ESBSessionState::Idle;

    UE_LOG(LogSBMultiplayer, Log, TEXT("Multiplayer subsystem initialized."));
}

void USBMultiplayerSubsystem::Deinitialize()
{
    LeaveSession();
    Super::Deinitialize();
}

// ============================================================================
// Login
// ============================================================================

void USBMultiplayerSubsystem::Login()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub)
    {
        UE_LOG(LogSBMultiplayer, Warning, TEXT("No OnlineSubsystem found."));
        SetLoginState(ESBLoginState::Failed);
        return;
    }

    IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
    if (!Identity.IsValid())
    {
        SetLoginState(ESBLoginState::Failed);
        return;
    }

    SetLoginState(ESBLoginState::LoggingIn);

    Identity->OnLoginCompleteDelegates->AddUObject(this, &USBMultiplayerSubsystem::OnLoginComplete);

    // Use device ID for testing, EOS account for production
    if (OnlineSub->GetSubsystemName() == FName("NULL"))
    {
        // NULL subsystem — auto-login
        SetLoginState(ESBLoginState::LoggedIn);
        return;
    }

    FOnlineAccountCredentials Credentials;
    Credentials.Type = TEXT("deviceid");
    Credentials.Id = TEXT("");
    Credentials.Token = TEXT("");

    Identity->Login(0, Credentials);
}

void USBMultiplayerSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
    if (bWasSuccessful)
    {
        SetLoginState(ESBLoginState::LoggedIn);
        UE_LOG(LogSBMultiplayer, Log, TEXT("Login successful. User: %s"), *GetPlayerDisplayName());
    }
    else
    {
        SetLoginState(ESBLoginState::Failed);
        UE_LOG(LogSBMultiplayer, Error, TEXT("Login failed: %s"), *Error);
    }

    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
        if (Identity.IsValid())
        {
            Identity->ClearOnLoginCompleteDelegates(0, this);
        }
    }
}

FString USBMultiplayerSubsystem::GetPlayerDisplayName() const
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub) return TEXT("Player");

    IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
    if (!Identity.IsValid()) return TEXT("Player");

    return Identity->GetPlayerNickname(0);
}

// ============================================================================
// Host
// ============================================================================

void USBMultiplayerSubsystem::HostSession(int32 MaxPlayers, bool bIsDedicatedServer, const FString& MapName)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (!Sessions.IsValid())
    {
        SetSessionState(ESBSessionState::Failed);
        return;
    }

    // Destroy existing session first
    FNamedOnlineSession* Existing = Sessions->GetNamedSession(NAME_GameSession);
    if (Existing)
    {
        Sessions->DestroySession(NAME_GameSession);
    }

    SetSessionState(ESBSessionState::Creating);

    FOnlineSessionSettings Settings;
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    Settings.bIsLANMatch = (OnlineSub && OnlineSub->GetSubsystemName() == FName("NULL"));
    Settings.NumPublicConnections = MaxPlayers;
    Settings.bShouldAdvertise = true;
    Settings.bUsesPresence = !bIsDedicatedServer;
    Settings.bAllowJoinInProgress = true;
    Settings.bAllowJoinViaPresence = true;
    Settings.bIsDedicated = bIsDedicatedServer;
    Settings.Set(FName("MAPNAME"), MapName, EOnlineDataAdvertisementType::ViaOnlineService);
    Settings.Set(FName("BUILDID"), FString::FromInt(GetBuildUniqueId()), EOnlineDataAdvertisementType::ViaOnlineService);

    Sessions->OnCreateSessionCompleteDelegates.AddUObject(this, &USBMultiplayerSubsystem::OnCreateSessionComplete);
    Sessions->CreateSession(0, NAME_GameSession, Settings);
}

void USBMultiplayerSubsystem::HostListenServer(int32 MaxPlayers, const FString& MapName)
{
    HostSession(MaxPlayers, false, MapName);
}

void USBMultiplayerSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (Sessions.IsValid())
    {
        Sessions->ClearOnCreateSessionCompleteDelegates(this);
    }

    if (bWasSuccessful)
    {
        SetSessionState(ESBSessionState::InSession);

        UGameInstance* GI = GetGameInstance();
        if (GI && GI->GetWorld())
        {
            GI->GetWorld()->ServerTravel(TEXT("/Game/Maps/TestMap/TestMap?listen"));
        }

        UE_LOG(LogSBMultiplayer, Log, TEXT("Session '%s' created."), *SessionName.ToString());
    }
    else
    {
        SetSessionState(ESBSessionState::Failed);
        UE_LOG(LogSBMultiplayer, Error, TEXT("Failed to create session."));
    }
}

// ============================================================================
// Find & Join
// ============================================================================

void USBMultiplayerSubsystem::FindSessions(int32 MaxResults)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (!Sessions.IsValid())
    {
        SetSessionState(ESBSessionState::Failed);
        return;
    }

    SetSessionState(ESBSessionState::Searching);
    CachedSearchResults.Empty();

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults = MaxResults;

    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    SessionSearch->bIsLanQuery = (OnlineSub && OnlineSub->GetSubsystemName() == FName("NULL"));

    Sessions->OnFindSessionsCompleteDelegates.AddUObject(this, &USBMultiplayerSubsystem::OnFindSessionsComplete);
    Sessions->FindSessions(0, SessionSearch.ToSharedRef());
}

void USBMultiplayerSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (Sessions.IsValid())
    {
        Sessions->ClearOnFindSessionsCompleteDelegates(this);
    }

    CachedSearchResults.Empty();

    if (bWasSuccessful && SessionSearch.IsValid())
    {
        for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
        {
            const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[i];

            FSBSessionSearchResult Entry;
            Entry.SearchResultIndex = i;
            Entry.SessionId = Result.GetSessionIdStr();
            Entry.CurrentPlayers = Result.Session.SessionSettings.NumPublicConnections -
                                   Result.Session.NumOpenPublicConnections;
            Entry.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
            Entry.PingMs = Result.PingInMs;

            FString MapName;
            Result.Session.SessionSettings.Get(FName("MAPNAME"), MapName);
            Entry.MapName = MapName;

            CachedSearchResults.Add(Entry);
        }

        UE_LOG(LogSBMultiplayer, Log, TEXT("Found %d sessions."), CachedSearchResults.Num());
    }

    SetSessionState(ESBSessionState::Idle);
    OnSessionSearchComplete.Broadcast(CachedSearchResults);
}

void USBMultiplayerSubsystem::JoinSession(int32 SearchResultIndex)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (!Sessions.IsValid() || !SessionSearch.IsValid())
    {
        OnSessionJoined.Broadcast(false);
        return;
    }

    if (!SessionSearch->SearchResults.IsValidIndex(SearchResultIndex))
    {
        OnSessionJoined.Broadcast(false);
        return;
    }

    SetSessionState(ESBSessionState::Joining);

    Sessions->OnJoinSessionCompleteDelegates.AddUObject(this, &USBMultiplayerSubsystem::OnJoinSessionComplete);
    Sessions->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[SearchResultIndex]);
}

void USBMultiplayerSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (Sessions.IsValid())
    {
        Sessions->ClearOnJoinSessionCompleteDelegates(this);
    }

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        SetSessionState(ESBSessionState::Failed);
        OnSessionJoined.Broadcast(false);
        UE_LOG(LogSBMultiplayer, Error, TEXT("Join failed. Result: %d"), (int32)Result);
        return;
    }

    SetSessionState(ESBSessionState::InSession);

    FString TravelURL;
    if (Sessions.IsValid() && Sessions->GetResolvedConnectString(SessionName, TravelURL))
    {
        UGameInstance* GI = GetGameInstance();
        if (GI)
        {
            APlayerController* PC = GI->GetFirstLocalPlayerController();
            if (PC)
            {
                PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
            }
        }
    }

    OnSessionJoined.Broadcast(true);
    UE_LOG(LogSBMultiplayer, Log, TEXT("Joined session '%s'."), *SessionName.ToString());
}

void USBMultiplayerSubsystem::QuickMatch()
{
    // Find sessions, then the user should call JoinSession(0) when OnSessionSearchComplete fires
    // Full auto-join logic is better handled in Blueprint for UI feedback
    FindSessions(10);
}

// ============================================================================
// Leave
// ============================================================================

void USBMultiplayerSubsystem::LeaveSession()
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (!Sessions.IsValid()) return;

    FNamedOnlineSession* Existing = Sessions->GetNamedSession(NAME_GameSession);
    if (Existing)
    {
        Sessions->OnDestroySessionCompleteDelegates.AddUObject(this, &USBMultiplayerSubsystem::OnDestroySessionComplete);
        Sessions->DestroySession(NAME_GameSession);
    }
}

void USBMultiplayerSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (Sessions.IsValid())
    {
        Sessions->ClearOnDestroySessionCompleteDelegates(this);
    }

    SetSessionState(ESBSessionState::Idle);
    UE_LOG(LogSBMultiplayer, Log, TEXT("Session destroyed. Success: %s"), bWasSuccessful ? TEXT("true") : TEXT("false"));
}

// ============================================================================
// Internal
// ============================================================================

IOnlineSessionPtr USBMultiplayerSubsystem::GetSessionInterface() const
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub) return nullptr;
    return OnlineSub->GetSessionInterface();
}

void USBMultiplayerSubsystem::SetLoginState(ESBLoginState NewState)
{
    LoginState = NewState;
    OnLoginStateChanged.Broadcast(NewState);
}

void USBMultiplayerSubsystem::SetSessionState(ESBSessionState NewState)
{
    SessionState = NewState;
    OnSessionStateChanged.Broadcast(NewState);
}
