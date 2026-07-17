// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MediaPlayer.h"
#include "SBGameFlowHUD.generated.h"

class UMediaPlayer;
class UMediaSource;
class UMediaTexture;

UENUM(BlueprintType)
enum class ESBFlowScreen : uint8
{
    Splash,
    Login,
    Lobby,
    InGame
};

/**
 * BGMI-style game flow HUD.
 * Renders: Splash (logo + loading) → Login (guest button) → Lobby (start match) → InGame HUD.
 * All drawn via Canvas — no Widget Blueprints needed.
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

    // Transition to next screen
    void GoToScreen(ESBFlowScreen Screen);

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Flow")
    float SplashDuration = 10.0f;

    // --- Splash Video ---

    UPROPERTY()
    TObjectPtr<UMediaPlayer> SplashMediaPlayer;

    UPROPERTY()
    TObjectPtr<UMediaTexture> SplashMediaTexture;

    void SetupSplashVideo();
    void DrawSplashVideo();
    bool bSplashVideoPlaying;

private:
    // --- Screen Draws ---
    void DrawSplashScreen();
    void DrawLoginScreen();
    void DrawLobbyScreen();
    void DrawInGameHUD();

    // --- InGame HUD elements (from SBInGameHUD) ---
    void DrawHealthShieldBoost();
    void DrawWeaponAmmo();
    void DrawCrosshair();
    void DrawCompass();
    void DrawMinimap();
    void DrawAliveCount();
    void DrawFPSPing();

    // --- Helpers ---
    void DrawBar(float X, float Y, float W, float H, float Pct, FLinearColor Fill, FLinearColor Bg);
    void DrawButton(float X, float Y, float W, float H, const FString& Text, FLinearColor Color);
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
    bool bTransitionFadingOut; // true = fading to black, false = fading in

    void StartTransition(ESBFlowScreen Target, float Duration = 0.8f);
    void DrawTransitionOverlay();

    // Login state
    bool bLoggedIn;

    // Lobby state
    int32 SelectedBotCount;
    bool bMatchStartRequested;

    // Track mouse
    bool bMouseWasPressed;

    // --- Login Screen Helpers ---
    void DrawLoginButton(float X, float Y, float W, float H,
        const FString& IconText, const FString& Label,
        FLinearColor BgColor, FLinearColor BorderColor, FLinearColor TextColor);
    void DrawRoundedBorder(float X, float Y, float W, float H, FLinearColor Color, float Thickness = 2.0f);
    void DrawCornerAccents(float SW, float SH, float Size, FLinearColor Color);
    void DrawSeparatorLine(float X, float Y, float W, const FString& Text);
};
