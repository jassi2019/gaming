// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SBUIManager.generated.h"

class USBMainMenuWidget;
class USBLobbyWidget;
class USBSettingsWidget;

UENUM(BlueprintType)
enum class ESBUIScreen : uint8
{
    None,
    MainMenu,
    Lobby,
    InGame,
    Inventory,
    DeathScreen,
    Settings,
    MatchEnd
};

/**
 * Manages UI screen lifecycle and transitions.
 * Call from PlayerController or GameMode to show/hide screens.
 */
UCLASS()
class STORMBREAKER_API USBUIManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|UI")
    void ShowScreen(ESBUIScreen Screen);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|UI")
    void HideCurrentScreen();

    UFUNCTION(BlueprintPure, Category = "StormBreaker|UI")
    ESBUIScreen GetCurrentScreen() const { return CurrentScreen; }

    // Widget class overrides — set in Blueprint or C++ to customize
    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|UI")
    TSubclassOf<UUserWidget> LobbyWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|UI")
    TSubclassOf<UUserWidget> SettingsWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|UI")
    TSubclassOf<UUserWidget> DeathScreenWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|UI")
    TSubclassOf<UUserWidget> MatchEndWidgetClass;

private:
    ESBUIScreen CurrentScreen;

    UPROPERTY()
    TObjectPtr<UUserWidget> ActiveWidget;
};
