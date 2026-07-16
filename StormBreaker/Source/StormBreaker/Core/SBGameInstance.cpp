// Copyright StormBreaker Games. All Rights Reserved.

#include "Core/SBGameInstance.h"
#include "StormBreaker.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Online/OnlineSessionNames.h"

USBGameInstance::USBGameInstance()
    : LocalPlayerDisplayName(TEXT("Player"))
{
}

void USBGameInstance::Init()
{
    Super::Init();
    UE_LOG(LogStormBreaker, Log, TEXT("StormBreaker GameInstance initialized."));
    InitOnlineSubsystem();
}

void USBGameInstance::OnStart()
{
    Super::OnStart();
}

void USBGameInstance::Shutdown()
{
    DestroyCurrentSession();
    Super::Shutdown();
}

// ----- Online Session -----

void USBGameInstance::InitOnlineSubsystem()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub)
    {
        UE_LOG(LogSBMultiplayer, Warning, TEXT("No OnlineSubsystem found. Multiplayer disabled."));
        return;
    }

    UE_LOG(LogSBMultiplayer, Log, TEXT("OnlineSubsystem: %s"), *OnlineSub->GetSubsystemName().ToString());
}

void USBGameInstance::HostSession(int32 MaxPlayers, bool bIsDedicatedServer)
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub) return;

    IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    FOnlineSessionSettings SessionSettings;
    SessionSettings.bIsLANMatch = (OnlineSub->GetSubsystemName() == "NULL");
    SessionSettings.NumPublicConnections = MaxPlayers;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bUsesPresence = true;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bIsDedicated = bIsDedicatedServer;
    SessionSettings.Set(SETTING_MAPNAME, FString(TEXT("MainMap")), EOnlineDataAdvertisementType::ViaOnlineService);

    SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
        this, &USBGameInstance::OnCreateSessionComplete);

    SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}

void USBGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccessful)
{
    if (bSuccessful)
    {
        UE_LOG(LogSBMultiplayer, Log, TEXT("Session '%s' created. Travelling to match map."), *SessionName.ToString());
        GetWorld()->ServerTravel(TEXT("/Game/Maps/MainMap/MainMap?listen"));
    }
    else
    {
        UE_LOG(LogSBMultiplayer, Error, TEXT("Failed to create session '%s'."), *SessionName.ToString());
    }
}

void USBGameInstance::FindAndJoinSession()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub) return;

    IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults = 20;
    SessionSearch->bIsLanQuery = (OnlineSub->GetSubsystemName() == "NULL");
    SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);

    SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
        this, &USBGameInstance::OnFindSessionsComplete);

    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void USBGameInstance::OnFindSessionsComplete(bool bSuccessful)
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub || !bSuccessful || !SessionSearch.IsValid()) return;

    IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    if (SessionSearch->SearchResults.Num() == 0)
    {
        UE_LOG(LogSBMultiplayer, Warning, TEXT("No sessions found."));
        return;
    }

    // Join first available session
    SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
        this, &USBGameInstance::OnJoinSessionComplete);

    SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
}

void USBGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogSBMultiplayer, Error, TEXT("Failed to join session '%s'. Result: %d"), *SessionName.ToString(), (int32)Result);
        return;
    }

    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub) return;

    IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    FString TravelURL;
    if (SessionInterface->GetResolvedConnectString(SessionName, TravelURL))
    {
        UE_LOG(LogSBMultiplayer, Log, TEXT("Joining session at: %s"), *TravelURL);
        APlayerController* PC = GetFirstLocalPlayerController();
        if (PC)
        {
            PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
        }
    }
}

void USBGameInstance::DestroyCurrentSession()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub) return;

    IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession)
    {
        SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
            this, &USBGameInstance::OnDestroySessionComplete);
        SessionInterface->DestroySession(NAME_GameSession);
    }
}

void USBGameInstance::OnDestroySessionComplete(FName SessionName, bool bSuccessful)
{
    UE_LOG(LogSBMultiplayer, Log, TEXT("Session '%s' destroyed. Success: %s"),
        *SessionName.ToString(), bSuccessful ? TEXT("true") : TEXT("false"));
}

// ----- Player Identity -----

FString USBGameInstance::GetLocalPlayerDisplayName() const
{
    return LocalPlayerDisplayName;
}

void USBGameInstance::SetLocalPlayerDisplayName(const FString& InName)
{
    LocalPlayerDisplayName = InName;
}

// ----- Quality Settings -----

void USBGameInstance::ApplyQualityPreset(int32 PresetIndex)
{
    UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
    if (!UserSettings) return;

    // 0=Low, 1=Medium, 2=High, 3=Ultra
    PresetIndex = FMath::Clamp(PresetIndex, 0, 3);
    UserSettings->SetOverallScalabilityLevel(PresetIndex);
    UserSettings->ApplySettings(true);

    UE_LOG(LogStormBreaker, Log, TEXT("Applied quality preset: %d"), PresetIndex);
}
