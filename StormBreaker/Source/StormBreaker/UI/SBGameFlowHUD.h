// Copyright StormBreaker Games. All Rights Reserved.

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

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Flow")
    float SplashDuration = 5.0f;

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

    // Login state
    bool bLoggedIn;

    // Lobby state
    int32 SelectedBotCount;
    bool bMatchStartRequested;

    // Track mouse
    bool bMouseWasPressed;
};
