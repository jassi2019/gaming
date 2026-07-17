// Copyright Island Of Death Games. All Rights Reserved.

#include "UI/SBGameFlowHUD.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "FileMediaSource.h"
#include "Character/SBCharacterBase.h"
#include "Character/SBCharacterMovementComponent.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponBase.h"
#include "Weapon/SBWeaponDataAsset.h"
#include "Core/SBPlayerState.h"
#include "Core/SBAttributeSet.h"
#include "Core/SBBattleRoyaleGameState.h"
#include "Core/SBBattleRoyaleGameMode.h"
#include "Inventory/SBInventoryComponent.h"
#include "BattleRoyale/SBZoneManager.h"
#include "Backend/SBBackendSubsystem.h"
#include "StormBreaker.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

ASBGameFlowHUD::ASBGameFlowHUD()
{
    CurrentScreen = ESBFlowScreen::Login;
    ScreenTimer = 0.0f;
    CachedFPS = 60.0f;
    FPSTimer = 0.0f;
    bLoggedIn = false;
    SelectedBotCount = 5;
    bMatchStartRequested = false;
    bMouseWasPressed = false;
    bSplashVideoPlaying = false;

    // Transition state
    bTransitioning = false;
    TransitionTarget = ESBFlowScreen::Login;
    TransitionTimer = 0.0f;
    TransitionDuration = 0.8f;
    bTransitionFadingOut = true;
}

void ASBGameFlowHUD::SetupSplashVideo()
{
    // Create media player at runtime
    SplashMediaPlayer = NewObject<UMediaPlayer>(this);
    if (!SplashMediaPlayer) return;

    SplashMediaPlayer->SetLooping(false);
    SplashMediaPlayer->PlayOnOpen = true;

    // Create media texture to render the video
    SplashMediaTexture = NewObject<UMediaTexture>(this);
    if (SplashMediaTexture)
    {
        SplashMediaTexture->SetMediaPlayer(SplashMediaPlayer);
        SplashMediaTexture->UpdateResource();
    }

    // Try to open the splash video from disk
    FString VideoPath = FPaths::ProjectContentDir() / TEXT("Splash/SplashVideo.mp4");
    FString AbsPath = FPaths::ConvertRelativePathToFull(VideoPath);

    // Try multiple URL formats for maximum compatibility
    TArray<FString> UrlsToTry = {
        FString::Printf(TEXT("file://%s"), *AbsPath.Replace(TEXT("\\"), TEXT("/"))),
        AbsPath,
        VideoPath
    };

    for (const FString& Url : UrlsToTry)
    {
        if (SplashMediaPlayer->OpenUrl(Url))
        {
            bSplashVideoPlaying = true;
            UE_LOG(LogStormBreaker, Log, TEXT("Splash video opened: %s"), *Url);
            break;
        }
    }

    if (!bSplashVideoPlaying)
    {
        UE_LOG(LogStormBreaker, Warning, TEXT("Splash video failed to open. Tried: %s"), *AbsPath);
    }
}

void ASBGameFlowHUD::DrawSplashVideo()
{
    if (!SplashMediaTexture || !SplashMediaPlayer) return;

    // Check if video finished
    if (ScreenTimer > 1.0f && !SplashMediaPlayer->IsPlaying() && !SplashMediaPlayer->IsPreparing())
    {
        bSplashVideoPlaying = false;
        return;
    }

    // Draw video texture fullscreen
    if (SplashMediaPlayer->IsPlaying() || SplashMediaPlayer->IsPreparing())
    {
        Canvas->SetDrawColor(FColor::White);
        Canvas->DrawTile(SplashMediaTexture, 0, 0, Canvas->SizeX, Canvas->SizeY,
            0.0f, 0.0f, 1.0f, 1.0f);
    }
}

void ASBGameFlowHUD::BeginPlay()
{
    Super::BeginPlay();

    // Setup splash video
    SetupSplashVideo();

    // Start on Splash screen — hide the character
    APlayerController* PC = GetOwningPlayerController();
    if (PC)
    {
        PC->SetShowMouseCursor(true);
        PC->SetInputMode(FInputModeGameAndUI());

        APawn* Pawn = PC->GetPawn();
        if (Pawn)
        {
            Pawn->SetActorHiddenInGame(true);
            Pawn->SetActorEnableCollision(false);
            Pawn->DisableInput(PC);
        }
    }
}

void ASBGameFlowHUD::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ScreenTimer += DeltaTime;

    FPSTimer += DeltaTime;
    if (FPSTimer >= 0.5f)
    {
        FPSTimer = 0.0f;
        CachedFPS = (DeltaTime > 0.001f) ? 1.0f / DeltaTime : 60.0f;
    }

    // Auto-advance from Splash to Login
    if (CurrentScreen == ESBFlowScreen::Splash && ScreenTimer >= SplashDuration)
    {
        StartTransition(ESBFlowScreen::Login);
    }

    // Handle smooth transition
    if (bTransitioning)
    {
        TransitionTimer += DeltaTime;
        float HalfDur = TransitionDuration * 0.5f;

        if (bTransitionFadingOut && TransitionTimer >= HalfDur)
        {
            // Midpoint: switch screen
            bTransitionFadingOut = false;
            GoToScreen(TransitionTarget);
        }

        if (TransitionTimer >= TransitionDuration)
        {
            bTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }

    // Track mouse click state (block clicks during transition)
    APlayerController* PC = GetOwningPlayerController();
    if (PC)
    {
        bMouseWasPressed = !bTransitioning && PC->WasInputKeyJustPressed(EKeys::LeftMouseButton);
    }
}

void ASBGameFlowHUD::StartTransition(ESBFlowScreen Target, float Duration)
{
    if (bTransitioning) return;
    bTransitioning = true;
    TransitionTarget = Target;
    TransitionTimer = 0.0f;
    TransitionDuration = Duration;
    bTransitionFadingOut = true;
}

void ASBGameFlowHUD::DrawTransitionOverlay()
{
    if (!bTransitioning) return;

    float HalfDur = TransitionDuration * 0.5f;
    float Alpha;

    if (bTransitionFadingOut)
    {
        // Fade to black
        Alpha = FMath::Clamp(TransitionTimer / HalfDur, 0.0f, 1.0f);
    }
    else
    {
        // Fade from black
        Alpha = FMath::Clamp(1.0f - (TransitionTimer - HalfDur) / HalfDur, 0.0f, 1.0f);
    }

    DrawRect(FLinearColor(0, 0, 0, Alpha), 0, 0, Canvas->SizeX, Canvas->SizeY);
}

void ASBGameFlowHUD::GoToScreen(ESBFlowScreen Screen)
{
    CurrentScreen = Screen;
    ScreenTimer = 0.0f;

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    if (Screen == ESBFlowScreen::InGame)
    {
        // Enable gameplay
        PC->SetShowMouseCursor(false);
        PC->SetInputMode(FInputModeGameOnly());

        APawn* Pawn = PC->GetPawn();
        if (Pawn)
        {
            Pawn->SetActorHiddenInGame(false);
            Pawn->SetActorEnableCollision(true);
            Pawn->EnableInput(PC);
        }

        // Start the match
        ASBBattleRoyaleGameMode* GM = Cast<ASBBattleRoyaleGameMode>(
            UGameplayStatics::GetGameMode(this));
        if (GM)
        {
            GM->NumberOfBots = SelectedBotCount;
            GM->StartMatch();
        }
    }
    else
    {
        PC->SetShowMouseCursor(true);
        PC->SetInputMode(FInputModeGameAndUI());
    }
}

void ASBGameFlowHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    switch (CurrentScreen)
    {
    case ESBFlowScreen::Splash: DrawSplashScreen(); break;
    case ESBFlowScreen::Login:  DrawLoginScreen();  break;
    case ESBFlowScreen::Lobby:  DrawLobbyScreen();  break;
    case ESBFlowScreen::InGame: DrawInGameHUD();    break;
    }

    // Smooth fade overlay on top
    DrawTransitionOverlay();

    // Fade-in when login screen first appears (from splash video)
    if (CurrentScreen == ESBFlowScreen::Login && !bTransitioning && ScreenTimer < 1.0f)
    {
        float FadeIn = 1.0f - FMath::Clamp(ScreenTimer / 1.0f, 0.0f, 1.0f);
        DrawRect(FLinearColor(0, 0, 0, FadeIn), 0, 0, Canvas->SizeX, Canvas->SizeY);
    }
}

// ============================================================================
// SPLASH SCREEN
// ============================================================================

void ASBGameFlowHUD::DrawSplashScreen()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;
    const float T = ScreenTimer;
    const float Duration = SplashDuration;

    UFont* LargeFont = GEngine->GetLargeFont();
    UFont* MedFont = GEngine->GetMediumFont();
    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* TinyFont = GEngine->GetTinyFont();
    float TW, TH;

    // === PHASE 1: Dark background with subtle blue gradient ===
    DrawRect(FLinearColor(0.01f, 0.015f, 0.03f), 0, 0, SW, SH);

    // Subtle diagonal light streaks (storm feel)
    float StreakAlpha = 0.03f;
    for (int32 i = 0; i < 5; i++)
    {
        float X = SW * (0.1f + i * 0.2f) + FMath::Sin(T * 0.5f + i) * 30.0f;
        DrawRect(FLinearColor(0.3f, 0.5f, 0.8f, StreakAlpha), X, 0, 3, SH);
    }

    // Vignette — dark edges
    float VigSize = SW * 0.15f;
    DrawRect(FLinearColor(0, 0, 0, 0.6f), 0, 0, VigSize, SH);
    DrawRect(FLinearColor(0, 0, 0, 0.6f), SW - VigSize, 0, VigSize, SH);
    DrawRect(FLinearColor(0, 0, 0, 0.5f), 0, 0, SW, VigSize * 0.5f);
    DrawRect(FLinearColor(0, 0, 0, 0.5f), 0, SH - VigSize * 0.5f, SW, VigSize * 0.5f);

    // === PHASE 2: Lightning flashes ===
    bool bFlash1 = (T > 2.0f && T < 2.15f);
    bool bFlash2 = (T > 4.5f && T < 4.6f);
    if (bFlash1 || bFlash2)
    {
        float FlashAlpha = bFlash1 ? (1.0f - (T - 2.0f) / 0.15f) : (1.0f - (T - 4.5f) / 0.1f);
        DrawRect(FLinearColor(0.4f, 0.5f, 0.8f, FlashAlpha * 0.3f), 0, 0, SW, SH);
    }

    // === PHASE 3: Title fade-in (starts at 1.5s, full at 3.5s) ===
    float TitleAlpha = FMath::Clamp((T - 1.5f) / 2.0f, 0.0f, 1.0f);
    if (TitleAlpha > 0.0f)
    {
        FString Title = TEXT("ISLAND OF DEATH");
        Canvas->TextSize(LargeFont, Title, TW, TH);
        float TitleX = (SW - TW) * 0.5f;
        float TitleY = SH * 0.38f;

        // Glow behind title
        float GlowPulse = 0.5f + 0.5f * FMath::Sin(T * 3.0f);
        float GlowAlpha = TitleAlpha * 0.15f * GlowPulse;
        DrawRect(FLinearColor(0.3f, 0.5f, 1.0f, GlowAlpha),
            TitleX - 30, TitleY - 10, TW + 60, TH + 20);

        // Title text — metallic silver-blue
        uint8 R = (uint8)(FMath::Lerp(180.0f, 220.0f, GlowPulse) * TitleAlpha);
        uint8 G = (uint8)(FMath::Lerp(190.0f, 230.0f, GlowPulse) * TitleAlpha);
        uint8 B = (uint8)(FMath::Lerp(210.0f, 255.0f, GlowPulse) * TitleAlpha);
        Canvas->SetDrawColor(FColor(R, G, B, (uint8)(255 * TitleAlpha)));
        Canvas->DrawText(LargeFont, Title, TitleX, TitleY);
    }

    // === PHASE 4: Accent line (draws from center at 3s) ===
    float LineProgress = FMath::Clamp((T - 3.0f) / 1.0f, 0.0f, 1.0f);
    if (LineProgress > 0.0f)
    {
        float LineW = SW * 0.2f * LineProgress;
        float LineY = SH * 0.48f;
        float LineX = (SW - LineW) * 0.5f;

        // Glow
        DrawRect(FLinearColor(0.1f, 0.3f, 0.8f, 0.2f * LineProgress),
            LineX - 5, LineY - 3, LineW + 10, 8);
        // Main line
        DrawRect(FLinearColor(0.2f, 0.5f, 1.0f, LineProgress),
            LineX, LineY, LineW, 2);
    }

    // === PHASE 5: Tagline fade-in (starts at 4s) ===
    float TagAlpha = FMath::Clamp((T - 4.0f) / 1.5f, 0.0f, 1.0f);
    if (TagAlpha > 0.0f)
    {
        FString Sub = TEXT("Some Spirits Don't Seek Revenge... They Seek Survival");
        Canvas->TextSize(SmallFont, Sub, TW, TH);
        Canvas->SetDrawColor(FColor(160, 170, 190, (uint8)(200 * TagAlpha)));
        Canvas->DrawText(SmallFont, Sub, (SW - TW) * 0.5f, SH * 0.52f);
    }

    // === PHASE 6: Bottom info (fades in at 5s) ===
    float BottomAlpha = FMath::Clamp((T - 5.0f) / 1.0f, 0.0f, 1.0f);
    if (BottomAlpha > 0.0f)
    {
        // Engine version
        Canvas->SetDrawColor(FColor(60, 70, 90, (uint8)(120 * BottomAlpha)));
        Canvas->DrawText(TinyFont, TEXT("Unreal Engine 5.8"), SW - 180, SH - 30);

        // Studio name
        Canvas->DrawText(TinyFont, TEXT("Island Of Death Games"), 20, SH - 30);
    }

    // === PHASE 7: Loading bar (starts at 2s) ===
    float BarProgress = FMath::Clamp((T - 2.0f) / (Duration - 3.0f), 0.0f, 1.0f);
    float BarW = SW * 0.25f;
    float BarH = 4.0f;
    float BarX = (SW - BarW) * 0.5f;
    float BarY = SH * 0.62f;

    DrawBar(BarX, BarY, BarW, BarH, BarProgress,
        FLinearColor(0.2f, 0.5f, 1.0f, 0.8f), FLinearColor(0.1f, 0.1f, 0.15f, 0.5f));

    // Loading text
    float LoadAlpha2 = FMath::Clamp((T - 2.0f) / 1.0f, 0.0f, 1.0f);
    FString LoadStr = TEXT("LOADING...");
    Canvas->TextSize(TinyFont, LoadStr, TW, TH);
    Canvas->SetDrawColor(FColor(120, 130, 150, (uint8)(180 * LoadAlpha2)));
    Canvas->DrawText(TinyFont, LoadStr, (SW - TW) * 0.5f, BarY + 12.0f);

    // === FADE: Black at start and end ===
    // Fade from black (0-1s)
    if (T < 1.0f)
    {
        float FadeAlpha = 1.0f - T;
        DrawRect(FLinearColor(0, 0, 0, FadeAlpha), 0, 0, SW, SH);
    }

    // Fade to black (last 1.5s)
    if (T > Duration - 1.5f)
    {
        float FadeAlpha = (T - (Duration - 1.5f)) / 1.5f;
        DrawRect(FLinearColor(0, 0, 0, FadeAlpha), 0, 0, SW, SH);
    }
}

// ============================================================================
// LOGIN SCREEN
// ============================================================================

void ASBGameFlowHUD::DrawLoginScreen()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;

    UFont* LargeFont = GEngine->GetLargeFont();
    UFont* MedFont = GEngine->GetMediumFont();
    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* TinyFont = GEngine->GetTinyFont();
    float TW, TH;

    // ====== BACKGROUND ======
    // Deep black base
    DrawRect(FLinearColor(0.02f, 0.02f, 0.03f), 0, 0, SW, SH);

    // Subtle dark red gradient at top (cinematic feel)
    for (int32 i = 0; i < 8; i++)
    {
        float Y = i * (SH * 0.05f);
        float A = 0.06f * (1.0f - (float)i / 8.0f);
        DrawRect(FLinearColor(0.3f, 0.05f, 0.02f, A), 0, Y, SW, SH * 0.05f);
    }

    // Subtle red glow at bottom edges
    DrawRect(FLinearColor(0.4f, 0.05f, 0.02f, 0.04f), 0, SH * 0.7f, SW, SH * 0.3f);

    // ====== CORNER ACCENTS (red frame like screenshot) ======
    DrawCornerAccents(SW, SH, 40.0f, FLinearColor(0.7f, 0.1f, 0.05f, 0.6f));

    // ====== TITLE: "ISLAND OF DEATH" ======
    // Main title — two parts with different styles

    // "ISLAND" — large, white/silver
    FString TitleTop = TEXT("ISLAND");
    Canvas->TextSize(LargeFont, TitleTop, TW, TH);
    float TitleCenterX = SW * 0.5f;
    float TitleY = SH * 0.12f;

    // Glow behind title
    float GlowPulse = 0.7f + 0.3f * FMath::Sin(ScreenTimer * 2.0f);
    DrawRect(FLinearColor(0.5f, 0.1f, 0.05f, 0.08f * GlowPulse),
        TitleCenterX - TW * 0.7f, TitleY - 10, TW * 1.4f, TH * 3.5f);

    Canvas->SetDrawColor(FColor(230, 230, 240));
    Canvas->DrawText(LargeFont, TitleTop, TitleCenterX - TW * 0.5f, TitleY);

    // "OF" — smaller, positioned between
    FString TitleOf = TEXT("OF");
    Canvas->TextSize(MedFont, TitleOf, TW, TH);
    Canvas->SetDrawColor(FColor(200, 200, 210, 200));
    Canvas->DrawText(MedFont, TitleOf, TitleCenterX - TW * 0.5f, TitleY + 38.0f);

    // "DEATH" — large, blood red
    FString TitleBottom = TEXT("DEATH");
    Canvas->TextSize(LargeFont, TitleBottom, TW, TH);
    Canvas->SetDrawColor(FColor(200, 30, 20));
    Canvas->DrawText(LargeFont, TitleBottom, TitleCenterX - TW * 0.5f, TitleY + 55.0f);

    // ====== TAGLINE with accent lines ======
    float TaglineY = TitleY + 115.0f;

    // Left accent line
    float LineW = SW * 0.12f;
    DrawRect(FLinearColor(0.5f, 0.1f, 0.05f, 0.5f), TitleCenterX - LineW - 10, TaglineY + 8, LineW, 1);
    // Right accent line
    DrawRect(FLinearColor(0.5f, 0.1f, 0.05f, 0.5f), TitleCenterX + 10, TaglineY + 8, LineW, 1);

    // Tagline text — "Some Spirits Don't seek revenge..." in italic white
    FString Tag1 = TEXT("Some Spirits Don't seek revenge...");
    Canvas->TextSize(TinyFont, Tag1, TW, TH);
    Canvas->SetDrawColor(FColor(200, 200, 200, 220));
    Canvas->DrawText(TinyFont, Tag1, TitleCenterX - TW * 0.5f - 40, TaglineY);

    // "they seek survival" in red
    FString Tag2 = TEXT("they seek survival");
    float T1W, T1H;
    Canvas->TextSize(TinyFont, Tag1, T1W, T1H);
    Canvas->SetDrawColor(FColor(220, 50, 30, 255));
    Canvas->DrawText(TinyFont, Tag2, TitleCenterX - TW * 0.5f + T1W - 38, TaglineY);

    // ====== LOGIN BUTTONS ======
    float BtnW = SW * 0.35f;
    if (BtnW < 300.0f) BtnW = 300.0f;
    if (BtnW > 450.0f) BtnW = 450.0f;
    float BtnH = 48.0f;
    float BtnX = (SW - BtnW) * 0.5f;
    float BtnStartY = SH * 0.45f;
    float BtnGap = 12.0f;

    // --- Continue with Google ---
    float GoogleY = BtnStartY;
    DrawLoginButton(BtnX, GoogleY, BtnW, BtnH,
        TEXT("G"), TEXT("CONTINUE WITH GOOGLE"),
        FLinearColor(0.92f, 0.92f, 0.90f),   // Light gray bg
        FLinearColor(0.7f, 0.7f, 0.68f),      // Gray border
        FLinearColor(0.15f, 0.15f, 0.15f));    // Dark text

    if (IsMouseInRect(BtnX, GoogleY, BtnW, BtnH) && WasMouseClicked())
    {
        UGameInstance* GI = GetGameInstance();
        if (GI)
        {
            USBBackendSubsystem* Backend = GI->GetSubsystem<USBBackendSubsystem>();
            if (Backend) Backend->Login(ESBAuthProvider::EOS);
        }
        StartTransition(ESBFlowScreen::Lobby);
    }

    // --- Continue with Facebook ---
    float FacebookY = GoogleY + BtnH + BtnGap;
    DrawLoginButton(BtnX, FacebookY, BtnW, BtnH,
        TEXT("f"), TEXT("CONTINUE WITH FACEBOOK"),
        FLinearColor(0.15f, 0.35f, 0.70f),    // Facebook blue
        FLinearColor(0.2f, 0.45f, 0.85f),     // Lighter blue border
        FLinearColor(1.0f, 1.0f, 1.0f));       // White text

    if (IsMouseInRect(BtnX, FacebookY, BtnW, BtnH) && WasMouseClicked())
    {
        UGameInstance* GI = GetGameInstance();
        if (GI)
        {
            USBBackendSubsystem* Backend = GI->GetSubsystem<USBBackendSubsystem>();
            if (Backend) Backend->Login(ESBAuthProvider::EOS);
        }
        StartTransition(ESBFlowScreen::Lobby);
    }

    // --- Continue with Apple ---
    float AppleY = FacebookY + BtnH + BtnGap;
    DrawLoginButton(BtnX, AppleY, BtnW, BtnH,
        TEXT("A"), TEXT("CONTINUE WITH APPLE"),
        FLinearColor(0.12f, 0.12f, 0.14f),    // Dark bg
        FLinearColor(0.35f, 0.35f, 0.38f),     // Gray border
        FLinearColor(1.0f, 1.0f, 1.0f));       // White text

    if (IsMouseInRect(BtnX, AppleY, BtnW, BtnH) && WasMouseClicked())
    {
        UGameInstance* GI = GetGameInstance();
        if (GI)
        {
            USBBackendSubsystem* Backend = GI->GetSubsystem<USBBackendSubsystem>();
            if (Backend) Backend->Login(ESBAuthProvider::EOS);
        }
        StartTransition(ESBFlowScreen::Lobby);
    }

    // ====== OR SEPARATOR ======
    float OrY = AppleY + BtnH + 18.0f;
    DrawSeparatorLine(BtnX, OrY, BtnW, TEXT("OR"));

    // ====== PLAY AS GUEST BUTTON (Red border, dark bg) ======
    float GuestY = OrY + 30.0f;
    float GuestH = 55.0f;

    // Dark background with red border
    bool bGuestHover = IsMouseInRect(BtnX, GuestY, BtnW, GuestH);
    FLinearColor GuestBg = bGuestHover
        ? FLinearColor(0.15f, 0.04f, 0.03f)
        : FLinearColor(0.08f, 0.03f, 0.02f);
    FLinearColor GuestBorder = FLinearColor(0.7f, 0.12f, 0.08f, bGuestHover ? 1.0f : 0.7f);

    DrawRect(GuestBg, BtnX, GuestY, BtnW, GuestH);
    DrawRoundedBorder(BtnX, GuestY, BtnW, GuestH, GuestBorder, 2.0f);

    // Red soldier icon (simple)
    Canvas->SetDrawColor(FColor(220, 50, 30));
    Canvas->DrawText(SmallFont, TEXT(">>"), BtnX + 18, GuestY + 8);

    // "PLAY AS GUEST" — bold red
    FString GuestTitle = TEXT("PLAY AS GUEST");
    Canvas->SetDrawColor(FColor(220, 50, 30));
    Canvas->DrawText(SmallFont, GuestTitle, BtnX + 55, GuestY + 8);

    // "Jump into the island as a guest" — subtitle
    FString GuestSub = TEXT("Jump into the island as a guest");
    Canvas->SetDrawColor(FColor(140, 140, 140));
    Canvas->DrawText(TinyFont, GuestSub, BtnX + 55, GuestY + 30);

    // Arrow on right side
    Canvas->SetDrawColor(FColor(200, 50, 30));
    Canvas->DrawText(SmallFont, TEXT(">"), BtnX + BtnW - 30, GuestY + 14);

    if (IsMouseInRect(BtnX, GuestY, BtnW, GuestH) && WasMouseClicked())
    {
        UGameInstance* GI = GetGameInstance();
        if (GI)
        {
            USBBackendSubsystem* Backend = GI->GetSubsystem<USBBackendSubsystem>();
            if (Backend) Backend->Login(ESBAuthProvider::Guest);
        }
        StartTransition(ESBFlowScreen::Lobby);
    }

    // ====== GUEST DISCLAIMER ======
    float DisclaimerY = GuestY + GuestH + 20.0f;
    FString Disclaimer = TEXT("Your progress as a guest will be");
    FString Disclaimer2 = TEXT("stored on this device only.");
    Canvas->TextSize(TinyFont, Disclaimer, TW, TH);
    Canvas->SetDrawColor(FColor(90, 90, 100));
    Canvas->DrawText(TinyFont, Disclaimer, (SW - TW) * 0.5f, DisclaimerY);
    Canvas->TextSize(TinyFont, Disclaimer2, TW, TH);
    Canvas->DrawText(TinyFont, Disclaimer2, (SW - TW) * 0.5f, DisclaimerY + 16);
}

// --- Login Screen Helper: Styled Button ---
void ASBGameFlowHUD::DrawLoginButton(float X, float Y, float W, float H,
    const FString& IconText, const FString& Label,
    FLinearColor BgColor, FLinearColor BorderColor, FLinearColor TextColor)
{
    bool bHovered = IsMouseInRect(X, Y, W, H);
    FLinearColor Bg = bHovered ? BgColor * 1.15f : BgColor;

    // Button background
    DrawRect(Bg, X, Y, W, H);

    // Border
    DrawRoundedBorder(X, Y, W, H, BorderColor, bHovered ? 2.0f : 1.0f);

    // Highlight on top edge
    DrawRect(FLinearColor(1, 1, 1, 0.08f), X + 1, Y + 1, W - 2, 1);

    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* MedFont = GEngine->GetMediumFont();
    float TW, TH;

    // Icon text (left side)
    Canvas->SetDrawColor(FColor(
        (uint8)(TextColor.R * 255),
        (uint8)(TextColor.G * 255),
        (uint8)(TextColor.B * 255)));
    Canvas->DrawText(MedFont, IconText, X + 20, Y + (H - 20) * 0.5f);

    // Separator line
    DrawRect(FLinearColor(TextColor.R, TextColor.G, TextColor.B, 0.2f),
        X + 55, Y + 8, 1, H - 16);

    // Label text (centered in remaining space)
    Canvas->TextSize(SmallFont, Label, TW, TH);
    float LabelX = X + 60 + (W - 60 - TW) * 0.5f;
    Canvas->SetDrawColor(FColor(
        (uint8)(TextColor.R * 255),
        (uint8)(TextColor.G * 255),
        (uint8)(TextColor.B * 255)));
    Canvas->DrawText(SmallFont, Label, LabelX, Y + (H - TH) * 0.5f);
}

void ASBGameFlowHUD::DrawRoundedBorder(float X, float Y, float W, float H, FLinearColor Color, float Thickness)
{
    // Top
    DrawRect(Color, X, Y, W, Thickness);
    // Bottom
    DrawRect(Color, X, Y + H - Thickness, W, Thickness);
    // Left
    DrawRect(Color, X, Y, Thickness, H);
    // Right
    DrawRect(Color, X + W - Thickness, Y, Thickness, H);
}

void ASBGameFlowHUD::DrawCornerAccents(float SW, float SH, float Size, FLinearColor Color)
{
    float T = 2.0f; // thickness
    float Inset = 15.0f;

    // Top-left corner
    DrawRect(Color, Inset, Inset, Size, T);
    DrawRect(Color, Inset, Inset, T, Size);

    // Top-right corner
    DrawRect(Color, SW - Inset - Size, Inset, Size, T);
    DrawRect(Color, SW - Inset - T, Inset, T, Size);

    // Bottom-left corner
    DrawRect(Color, Inset, SH - Inset - T, Size, T);
    DrawRect(Color, Inset, SH - Inset - Size, T, Size);

    // Bottom-right corner
    DrawRect(Color, SW - Inset - Size, SH - Inset - T, Size, T);
    DrawRect(Color, SW - Inset - T, SH - Inset - Size, T, Size);
}

void ASBGameFlowHUD::DrawSeparatorLine(float X, float Y, float W, const FString& Text)
{
    UFont* TinyFont = GEngine->GetTinyFont();
    float TW, TH;
    Canvas->TextSize(TinyFont, Text, TW, TH);

    float TextX = X + (W - TW) * 0.5f;
    float LineY = Y + TH * 0.5f;
    float Gap = 10.0f;

    // Left line
    DrawRect(FLinearColor(0.25f, 0.15f, 0.1f, 0.5f), X, LineY, TextX - X - Gap, 1);
    // Right line
    DrawRect(FLinearColor(0.25f, 0.15f, 0.1f, 0.5f), TextX + TW + Gap, LineY, X + W - TextX - TW - Gap, 1);

    // Text
    Canvas->SetDrawColor(FColor(120, 100, 80));
    Canvas->DrawText(TinyFont, Text, TextX, Y);
}

// ============================================================================
// LOBBY SCREEN
// ============================================================================

void ASBGameFlowHUD::DrawLobbyScreen()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;

    // Dark blue gradient-ish background
    DrawRect(FLinearColor(0.03f, 0.04f, 0.08f), 0, 0, SW, SH);

    UFont* LargeFont = GEngine->GetLargeFont();
    UFont* MedFont = GEngine->GetMediumFont();
    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* TinyFont = GEngine->GetTinyFont();
    float TW, TH;

    // Top bar
    DrawRect(FLinearColor(0.08f, 0.08f, 0.12f, 0.9f), 0, 0, SW, 60.0f);

    // Player info (top left)
    USBBackendSubsystem* Backend = nullptr;
    UGameInstance* GI = GetGameInstance();
    if (GI) Backend = GI->GetSubsystem<USBBackendSubsystem>();

    FString PlayerName = TEXT("Player");
    int32 Level = 1;
    int32 Coins = 0;

    if (Backend)
    {
        const FSBPlayerProfile& P = Backend->GetProfile();
        PlayerName = P.Username;
        Level = P.Level;
        Coins = P.Coins;
    }

    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(SmallFont, FString::Printf(TEXT("Lv.%d  %s"), Level, *PlayerName), 20.0f, 20.0f);

    // Coins (top right)
    FString CoinStr = FString::Printf(TEXT("Coins: %d"), Coins);
    Canvas->TextSize(SmallFont, CoinStr, TW, TH);
    Canvas->SetDrawColor(FColor(255, 220, 50));
    Canvas->DrawText(SmallFont, CoinStr, SW - TW - 20.0f, 20.0f);

    // Center: Game Mode
    FString ModeText = TEXT("ISLAND OF DEATH - TPP");
    Canvas->TextSize(MedFont, ModeText, TW, TH);
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(MedFont, ModeText, (SW - TW) * 0.5f, SH * 0.15f);

    // Map name
    FString MapText = TEXT("Map: Island of Death");
    Canvas->TextSize(SmallFont, MapText, TW, TH);
    Canvas->SetDrawColor(FColor(180, 180, 180));
    Canvas->DrawText(SmallFont, MapText, (SW - TW) * 0.5f, SH * 0.15f + 35.0f);

    // Bot count selector
    float SelectorY = SH * 0.35f;
    Canvas->SetDrawColor(FColor(200, 200, 200));
    Canvas->DrawText(SmallFont, TEXT("AI Bots:"), (SW - 200.0f) * 0.5f, SelectorY);

    // Minus button
    float MinusBtnX = (SW - 200.0f) * 0.5f + 100.0f;
    DrawButton(MinusBtnX, SelectorY - 5.0f, 40.0f, 30.0f, TEXT("-"), FLinearColor(0.3f, 0.3f, 0.3f));
    if (IsMouseInRect(MinusBtnX, SelectorY - 5.0f, 40.0f, 30.0f) && WasMouseClicked())
    {
        SelectedBotCount = FMath::Max(0, SelectedBotCount - 5);
    }

    // Count display
    Canvas->SetDrawColor(FColor(255, 220, 50));
    FString BotStr = FString::Printf(TEXT("%d"), SelectedBotCount);
    Canvas->DrawText(MedFont, BotStr, MinusBtnX + 55.0f, SelectorY - 5.0f);

    // Plus button
    float PlusBtnX = MinusBtnX + 100.0f;
    DrawButton(PlusBtnX, SelectorY - 5.0f, 40.0f, 30.0f, TEXT("+"), FLinearColor(0.3f, 0.3f, 0.3f));
    if (IsMouseInRect(PlusBtnX, SelectorY - 5.0f, 40.0f, 30.0f) && WasMouseClicked())
    {
        SelectedBotCount = FMath::Min(99, SelectedBotCount + 5);
    }

    // Squad slots (bottom center)
    float SlotY = SH * 0.55f;
    float SlotSize = 70.0f;
    float SlotGap = 15.0f;
    float SlotsStartX = (SW - (SlotSize * 4 + SlotGap * 3)) * 0.5f;

    for (int32 i = 0; i < 4; i++)
    {
        float SX = SlotsStartX + i * (SlotSize + SlotGap);
        FLinearColor SlotColor = (i == 0) ? FLinearColor(0.2f, 0.6f, 0.3f, 0.8f) : FLinearColor(0.15f, 0.15f, 0.2f, 0.6f);
        DrawRect(SlotColor, SX, SlotY, SlotSize, SlotSize);

        // Slot border
        DrawRect(FLinearColor(0.4f, 0.4f, 0.5f), SX, SlotY, SlotSize, 2);
        DrawRect(FLinearColor(0.4f, 0.4f, 0.5f), SX, SlotY + SlotSize - 2, SlotSize, 2);
        DrawRect(FLinearColor(0.4f, 0.4f, 0.5f), SX, SlotY, 2, SlotSize);
        DrawRect(FLinearColor(0.4f, 0.4f, 0.5f), SX + SlotSize - 2, SlotY, 2, SlotSize);

        FString SlotLabel = (i == 0) ? TEXT("YOU") : FString::Printf(TEXT("Slot %d"), i + 1);
        Canvas->TextSize(TinyFont, SlotLabel, TW, TH);
        Canvas->SetDrawColor((i == 0) ? FColor::White : FColor(100, 100, 100));
        Canvas->DrawText(TinyFont, SlotLabel, SX + (SlotSize - TW) * 0.5f, SlotY + SlotSize + 5.0f);
    }

    // START MATCH button (big, bottom)
    float StartBtnW = 350.0f;
    float StartBtnH = 60.0f;
    float StartBtnX = (SW - StartBtnW) * 0.5f;
    float StartBtnY = SH * 0.80f;

    DrawButton(StartBtnX, StartBtnY, StartBtnW, StartBtnH, TEXT("START MATCH"), FLinearColor(0.9f, 0.6f, 0.1f));

    if (IsMouseInRect(StartBtnX, StartBtnY, StartBtnW, StartBtnH) && WasMouseClicked())
    {
        StartTransition(ESBFlowScreen::InGame, 1.2f);
    }

    // Footer
    Canvas->SetDrawColor(FColor(60, 60, 60));
    Canvas->DrawText(TinyFont, TEXT("Solo | TPP | Asia"), 20.0f, SH - 25.0f);

    Canvas->TextSize(TinyFont, TEXT("v1.0.0"), TW, TH);
    Canvas->DrawText(TinyFont, TEXT("v1.0.0"), SW - TW - 20.0f, SH - 25.0f);
}

// ============================================================================
// IN-GAME HUD (simplified from SBInGameHUD)
// ============================================================================

void ASBGameFlowHUD::DrawInGameHUD()
{
    DrawHealthShieldBoost();
    DrawWeaponAmmo();
    DrawCrosshair();
    DrawCompass();
    DrawMinimap();
    DrawAliveCount();
    DrawFPSPing();
}

void ASBGameFlowHUD::DrawHealthShieldBoost()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;
    const float BarW = SW * 0.22f;
    const float BarH = 16.0f;
    float Y = SH - 30.0f;

    float Health = 100.0f, MaxHealth = 100.0f, Shield = 0.0f, MaxShield = 150.0f;

    ASBCharacterBase* Char = GetPlayerCharacter();
    if (Char)
    {
        ASBPlayerState* PS = Char->GetPlayerState<ASBPlayerState>();
        if (PS && PS->AttributeSet)
        {
            Health = PS->AttributeSet->GetHealth();
            MaxHealth = PS->AttributeSet->GetMaxHealth();
            Shield = PS->AttributeSet->GetShield();
            MaxShield = PS->AttributeSet->GetMaxShield();
        }
    }

    UFont* Font = GEngine->GetTinyFont();

    DrawBar(20, Y, BarW, BarH, MaxHealth > 0 ? Health / MaxHealth : 0,
        FLinearColor(0.15f, 0.85f, 0.25f, 0.9f), FLinearColor(0.1f, 0.1f, 0.1f, 0.7f));
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), Health), 24, Y + 1);

    Y -= BarH + 4;

    DrawBar(20, Y, BarW, BarH, MaxShield > 0 ? Shield / MaxShield : 0,
        FLinearColor(0.2f, 0.5f, 1.0f, 0.9f), FLinearColor(0.1f, 0.1f, 0.1f, 0.7f));
    Canvas->SetDrawColor(FColor(100, 180, 255));
    Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), Shield), 24, Y + 1);
}

void ASBGameFlowHUD::DrawWeaponAmmo()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;
    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* TinyFont = GEngine->GetTinyFont();

    FString AmmoStr = TEXT("--/--");
    FString WeaponName = TEXT("");
    FString FireMode = TEXT("");

    ASBCharacterBase* Char = GetPlayerCharacter();
    if (Char && Char->WeaponComponent)
    {
        ASBWeaponBase* W = Char->WeaponComponent->GetActiveWeapon();
        if (W)
        {
            AmmoStr = FString::Printf(TEXT("%d / %d"), W->GetCurrentMagazine(), W->GetCurrentReserve());
            if (W->WeaponData) WeaponName = W->WeaponData->DisplayName.ToString();
            switch (W->GetCurrentFireMode())
            {
            case ESBFireMode::Single: FireMode = TEXT("SINGLE"); break;
            case ESBFireMode::Burst:  FireMode = TEXT("BURST");  break;
            case ESBFireMode::Auto:   FireMode = TEXT("AUTO");   break;
            }
        }
    }

    float TW, TH;
    float X = SW - 20.0f;
    float Y = SH - 30.0f;

    Canvas->SetDrawColor(FColor::White);
    Canvas->TextSize(SmallFont, AmmoStr, TW, TH);
    Canvas->DrawText(SmallFont, AmmoStr, X - TW, Y - TH);
    Y -= TH + 4;

    Canvas->SetDrawColor(FColor(200, 200, 100));
    Canvas->TextSize(TinyFont, FireMode, TW, TH);
    Canvas->DrawText(TinyFont, FireMode, X - TW, Y - TH);
    Y -= TH + 2;

    Canvas->SetDrawColor(FColor(200, 200, 200));
    Canvas->TextSize(TinyFont, WeaponName, TW, TH);
    Canvas->DrawText(TinyFont, WeaponName, X - TW, Y - TH);
}

void ASBGameFlowHUD::DrawCrosshair()
{
    float CX = Canvas->SizeX * 0.5f;
    float CY = Canvas->SizeY * 0.5f;
    FLinearColor C(1, 1, 1, 0.8f);
    DrawRect(C, CX - 12, CY - 1, 8, 2);
    DrawRect(C, CX + 4, CY - 1, 8, 2);
    DrawRect(C, CX - 1, CY - 12, 2, 8);
    DrawRect(C, CX - 1, CY + 4, 2, 8);
    DrawRect(C, CX - 1, CY - 1, 2, 2);
}

void ASBGameFlowHUD::DrawCompass()
{
    float CX = Canvas->SizeX * 0.5f;
    float HalfW = Canvas->SizeX * 0.175f;
    DrawRect(FLinearColor(0, 0, 0, 0.4f), CX - HalfW, 28, HalfW * 2, 20);

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;
    float Yaw = PC->GetControlRotation().Yaw;
    if (Yaw < 0) Yaw += 360.0f;

    UFont* Font = GEngine->GetTinyFont();
    static const TPair<float, FString> Dirs[] = {
        {0, TEXT("N")}, {90, TEXT("E")}, {180, TEXT("S")}, {270, TEXT("W")}
    };

    for (const auto& D : Dirs)
    {
        float Delta = FMath::FindDeltaAngleDegrees(Yaw, D.Key);
        float SX = CX + (Delta / 180.0f) * HalfW;
        if (SX > CX - HalfW && SX < CX + HalfW)
        {
            float TW, TH;
            Canvas->TextSize(Font, D.Value, TW, TH);
            Canvas->SetDrawColor(FColor::White);
            Canvas->DrawText(Font, D.Value, SX - TW * 0.5f, 30);
        }
    }

    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), Yaw), CX - 8, 14);
    DrawRect(FLinearColor::White, CX - 1, 26, 2, 4);
}

void ASBGameFlowHUD::DrawMinimap()
{
    float MapSize = FMath::Min(Canvas->SizeX, Canvas->SizeY) * 0.15f;
    float X = Canvas->SizeX - MapSize - 15;
    float Y = 55;
    DrawRect(FLinearColor(0, 0, 0, 0.5f), X, Y, MapSize, MapSize);
    DrawRect(FLinearColor(0.2f, 1.0f, 0.3f), X + MapSize * 0.5f - 3, Y + MapSize * 0.5f - 3, 6, 6);
}

void ASBGameFlowHUD::DrawAliveCount()
{
    int32 Alive = 1;
    ASBBattleRoyaleGameState* GS = Cast<ASBBattleRoyaleGameState>(UGameplayStatics::GetGameState(this));
    if (GS) Alive = GS->GetAlivePlayerCount();

    UFont* Font = GEngine->GetSmallFont();
    float TW, TH;
    FString Str = FString::Printf(TEXT("%d"), Alive);
    Canvas->TextSize(Font, Str, TW, TH);
    DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.7f), Canvas->SizeX - 55, 30, TW + 25, TH + 4);
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, Str, Canvas->SizeX - 35, 32);
}

void ASBGameFlowHUD::DrawFPSPing()
{
    UFont* Font = GEngine->GetTinyFont();
    Canvas->SetDrawColor(CachedFPS >= 30 ? FColor(100, 255, 100) : FColor(255, 100, 100));
    Canvas->DrawText(Font, FString::Printf(TEXT("FPS: %.0f"), CachedFPS), 20, Canvas->SizeY - 90);

    APlayerController* PC = GetOwningPlayerController();
    if (PC && PC->PlayerState)
    {
        float Ping = PC->PlayerState->GetPingInMilliseconds();
        Canvas->SetDrawColor(FColor(100, 255, 100));
        Canvas->DrawText(Font, FString::Printf(TEXT("Ping: %.0fms"), Ping), 20, Canvas->SizeY - 76);
    }
}

// ============================================================================
// Helpers
// ============================================================================

void ASBGameFlowHUD::DrawBar(float X, float Y, float W, float H, float Pct, FLinearColor Fill, FLinearColor Bg)
{
    DrawRect(Bg, X, Y, W, H);
    DrawRect(Fill, X, Y, W * FMath::Clamp(Pct, 0.0f, 1.0f), H);
}

void ASBGameFlowHUD::DrawButton(float X, float Y, float W, float H, const FString& Text, FLinearColor Color)
{
    bool bHovered = IsMouseInRect(X, Y, W, H);
    FLinearColor BtnColor = bHovered ? Color * 1.3f : Color;

    DrawRect(BtnColor, X, Y, W, H);
    DrawRect(FLinearColor(1, 1, 1, 0.3f), X, Y, W, 2);

    UFont* Font = GEngine->GetSmallFont();
    float TW, TH;
    Canvas->TextSize(Font, Text, TW, TH);
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, Text, X + (W - TW) * 0.5f, Y + (H - TH) * 0.5f);
}

bool ASBGameFlowHUD::IsMouseInRect(float X, float Y, float W, float H) const
{
    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return false;

    float MX, MY;
    PC->GetMousePosition(MX, MY);
    return (MX >= X && MX <= X + W && MY >= Y && MY <= Y + H);
}

bool ASBGameFlowHUD::WasMouseClicked() const
{
    return bMouseWasPressed;
}

ASBCharacterBase* ASBGameFlowHUD::GetPlayerCharacter() const
{
    APlayerController* PC = GetOwningPlayerController();
    return PC ? Cast<ASBCharacterBase>(PC->GetPawn()) : nullptr;
}
