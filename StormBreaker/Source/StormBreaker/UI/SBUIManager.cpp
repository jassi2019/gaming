// Copyright StormBreaker Games. All Rights Reserved.

#include "UI/SBUIManager.h"
#include "StormBreaker.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void USBUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    CurrentScreen = ESBUIScreen::None;
}

void USBUIManager::ShowScreen(ESBUIScreen Screen)
{
    HideCurrentScreen();
    CurrentScreen = Screen;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetGameInstance()->GetWorld(), 0);
    if (!PC) return;

    TSubclassOf<UUserWidget> WidgetClass = nullptr;
    switch (Screen)
    {
    case ESBUIScreen::MainMenu:    WidgetClass = MainMenuWidgetClass; break;
    case ESBUIScreen::Lobby:       WidgetClass = LobbyWidgetClass; break;
    case ESBUIScreen::Settings:    WidgetClass = SettingsWidgetClass; break;
    case ESBUIScreen::DeathScreen: WidgetClass = DeathScreenWidgetClass; break;
    case ESBUIScreen::MatchEnd:    WidgetClass = MatchEndWidgetClass; break;
    default: break;
    }

    if (WidgetClass)
    {
        ActiveWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
        if (ActiveWidget)
        {
            ActiveWidget->AddToViewport(10);
        }
    }
}

void USBUIManager::HideCurrentScreen()
{
    if (ActiveWidget)
    {
        ActiveWidget->RemoveFromParent();
        ActiveWidget = nullptr;
    }
    CurrentScreen = ESBUIScreen::None;
}
