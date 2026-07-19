// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MediaPlayer.h"
#include "SBGameFlowHUD.generated.h"

class UMediaPlayer;
class UMediaSource;
class UMediaTexture;
class UMediaSoundComponent;
class UIslandOfDeathLoginWidget;

UENUM(BlueprintType)
enum class ESBFlowScreen : uint8
{
    Splash,
    Login,
    Loading,   // Graphics quality + resource download
    Lobby,     // BGMI-style main menu
    InGame
};

UENUM(BlueprintType)
enum class ESBGraphicsQuality : uint8
{
    Low,
    Medium,
    HD,
    HDR,
    UltraHD
};

/**
 * Full game flow HUD — Island of Death.
 * Login → Loading (quality select + resource download) → Lobby (BGMI-style) → InGame.
 */
UCLASS()
class STORMBREAKER_API ASBGameFlowHUD : public AHUD
{
    GENERATED_BODY()

public:
    ASBGameFlowHUD();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void DrawHUD() override;

    void GoToScreen(ESBFlowScreen Screen);

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Flow")
    float SplashDuration = 10.0f;

    // --- Splash Video ---
    UPROPERTY()
    TObjectPtr<UMediaPlayer> SplashMediaPlayer;
    UPROPERTY()
    TObjectPtr<UMediaTexture> SplashMediaTexture;
    UPROPERTY()
    TObjectPtr<UMediaSoundComponent> SplashMediaSound;
    void SetupSplashVideo();
    void DrawSplashVideo();
    bool bSplashVideoPlaying;

    // --- Login Widget (UMG-based) ---
    UPROPERTY()
    TObjectPtr<UIslandOfDeathLoginWidget> LoginWidget;
    void CreateLoginWidget();
    void DestroyLoginWidget();
    UFUNCTION()
    void OnLoginButtonPressed();

private:
    // --- Screen Draws ---
    void DrawSplashScreen();
    void DrawLoginScreen();
    void DrawLoadingScreen();
    void DrawLobbyScreen();
    void DrawInGameHUD();

    // --- InGame HUD ---
    void DrawHealthShieldBoost();
    void DrawWeaponAmmo();
    void DrawCrosshair();
    void DrawCompass();
    void DrawMinimap();
    void DrawAliveCount();
    void DrawFPSPing();

    // --- Lobby sub-draws ---
    void DrawLobbyBackground();
    void DrawLobbyCenterScene();
    void DrawLobbyTopBar();
    void DrawLobbyLeftMenu();
    void DrawLobbyRightPanels();
    void DrawLobbyBottomBar();
    void DrawLobbyStartButton();
    void DrawLobbyGameLogo();
    void DrawLobbyChatBar();

    // --- Helpers ---
    void DrawBar(float X, float Y, float W, float H, float Pct, FLinearColor Fill, FLinearColor Bg);
    void DrawButton(float X, float Y, float W, float H, const FString& Text, FLinearColor Color);
    void DrawLoginButton(float X, float Y, float W, float H,
        const FString& IconText, const FString& Label,
        FLinearColor BgColor, FLinearColor BorderColor, FLinearColor TextColor);
    void DrawRoundedBorder(float X, float Y, float W, float H, FLinearColor Color, float Thickness = 2.0f);
    void DrawCornerAccents(float SW, float SH, float Size, FLinearColor Color);
    void DrawSeparatorLine(float X, float Y, float W, const FString& Text);
    void DrawPanel(float X, float Y, float W, float H, FLinearColor Bg, FLinearColor Border);
    void DrawBadge(float X, float Y, const FString& Text, FLinearColor Bg, FLinearColor TextCol);
    bool IsMouseInRect(float X, float Y, float W, float H) const;
    bool WasMouseClicked() const;
    class ASBCharacterBase* GetPlayerCharacter() const;

    ESBFlowScreen CurrentScreen;
    float ScreenTimer;
    float CachedFPS;
    float FPSTimer;

    // --- Smooth Transitions ---
    bool bTransitioning;
    ESBFlowScreen TransitionTarget;
    float TransitionTimer;
    float TransitionDuration;
    bool bTransitionFadingOut;
    void StartTransition(ESBFlowScreen Target, float Duration = 0.8f);
    void DrawTransitionOverlay();

    // --- Login state ---
    bool bLoggedIn;

    // --- Loading screen state ---
    ESBGraphicsQuality SelectedQuality;
    float DownloadProgress;
    bool bDownloadComplete;
    bool bDownloadStarted;
    float DownloadSpeed; // simulated

    // --- Lobby state ---
    int32 SelectedBotCount;
    int32 SelectedSquadSize; // 1=solo, 2=duo, 4=squad
    int32 LobbyMenuIndex;   // 0=Start, 1=Loadout, etc.
    bool bMatchStartRequested;
    FString ChatMessage;

    // --- Track mouse ---
    bool bMouseWasPressed;
};
