// Copyright Island Of Death Games. All Rights Reserved.

#include "UI/SBGameFlowHUD.h"
#include "UI/IslandOfDeathLoginWidget.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "MediaSoundComponent.h"
#include "FileMediaSource.h"
#include "Blueprint/UserWidget.h"
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
    SelectedSquadSize = 4;
    LobbyMenuIndex = 0;
    bMatchStartRequested = false;
    bMouseWasPressed = false;
    bSplashVideoPlaying = false;

    // Transition
    bTransitioning = false;
    TransitionTarget = ESBFlowScreen::Login;
    TransitionTimer = 0.0f;
    TransitionDuration = 0.8f;
    bTransitionFadingOut = true;

    // Loading screen
    SelectedQuality = ESBGraphicsQuality::HD;
    DownloadProgress = 0.0f;
    bDownloadComplete = false;
    bDownloadStarted = false;
    DownloadSpeed = 0.0f;

    ChatMessage = TEXT("World [NoobMaster]: anyone for classic?");
}

// ============================================================================
// SPLASH VIDEO
// ============================================================================

void ASBGameFlowHUD::SetupSplashVideo()
{
    SplashMediaPlayer = NewObject<UMediaPlayer>(this);
    if (!SplashMediaPlayer) return;
    SplashMediaPlayer->SetLooping(false);
    SplashMediaPlayer->PlayOnOpen = true;

    // Media texture for video
    SplashMediaTexture = NewObject<UMediaTexture>(this);
    if (SplashMediaTexture)
    {
        SplashMediaTexture->SetMediaPlayer(SplashMediaPlayer);
        SplashMediaTexture->UpdateResource();
    }

    // Media sound component for audio playback
    SplashMediaSound = NewObject<UMediaSoundComponent>(this);
    if (SplashMediaSound)
    {
        SplashMediaSound->SetMediaPlayer(SplashMediaPlayer);
        SplashMediaSound->RegisterComponent();
        UE_LOG(LogStormBreaker, Log, TEXT("MediaSoundComponent created for splash video audio"));
    }

    FString VideoPath = FPaths::ProjectContentDir() / TEXT("Splash/SplashVideo.mp4");
    FString AbsPath = FPaths::ConvertRelativePathToFull(VideoPath);

    TArray<FString> UrlsToTry = {
        FString::Printf(TEXT("file://%s"), *AbsPath.Replace(TEXT("\\"), TEXT("/"))),
        AbsPath, VideoPath
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
}

void ASBGameFlowHUD::DrawSplashVideo()
{
    if (!SplashMediaTexture || !SplashMediaPlayer) return;
    if (ScreenTimer > 1.0f && !SplashMediaPlayer->IsPlaying() && !SplashMediaPlayer->IsPreparing())
    {
        bSplashVideoPlaying = false;
        return;
    }
    if (SplashMediaPlayer->IsPlaying() || SplashMediaPlayer->IsPreparing())
    {
        Canvas->SetDrawColor(FColor::White);
        Canvas->DrawTile(SplashMediaTexture, 0, 0, Canvas->SizeX, Canvas->SizeY, 0, 0, 1, 1);
    }
}

// ============================================================================
// BEGIN PLAY / TICK
// ============================================================================

void ASBGameFlowHUD::BeginPlay()
{
    Super::BeginPlay();
    SetupSplashVideo();

    APlayerController* PC = GetOwningPlayerController();
    if (PC)
    {
        PC->SetShowMouseCursor(true);
        PC->SetInputMode(FInputModeUIOnly());
        APawn* Pawn = PC->GetPawn();
        if (Pawn)
        {
            Pawn->SetActorHiddenInGame(true);
            Pawn->SetActorEnableCollision(false);
            Pawn->DisableInput(PC);
        }
    }

    // Create login widget immediately
    CreateLoginWidget();
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

    // Auto-advance splash
    if (CurrentScreen == ESBFlowScreen::Splash && ScreenTimer >= SplashDuration)
    {
        StartTransition(ESBFlowScreen::Login);
    }

    // Loading screen: simulate download
    if (CurrentScreen == ESBFlowScreen::Loading && bDownloadStarted && !bDownloadComplete)
    {
        // Speed varies for realism
        float SpeedTarget = 45.0f + 20.0f * FMath::Sin(ScreenTimer * 1.5f);
        DownloadSpeed = FMath::FInterpTo(DownloadSpeed, SpeedTarget, DeltaTime, 2.0f);
        DownloadProgress += (DownloadSpeed / 850.0f) * DeltaTime; // ~850MB total, takes ~18s
        if (DownloadProgress >= 1.0f)
        {
            DownloadProgress = 1.0f;
            bDownloadComplete = true;
        }
    }

    // Transition handling
    if (bTransitioning)
    {
        TransitionTimer += DeltaTime;
        float HalfDur = TransitionDuration * 0.5f;
        if (bTransitionFadingOut && TransitionTimer >= HalfDur)
        {
            bTransitionFadingOut = false;
            GoToScreen(TransitionTarget);
        }
        if (TransitionTimer >= TransitionDuration)
        {
            bTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }

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

void ASBGameFlowHUD::GoToScreen(ESBFlowScreen Screen)
{
    // Cleanup: remove login widget when leaving login screen
    if (CurrentScreen == ESBFlowScreen::Login && Screen != ESBFlowScreen::Login)
    {
        DestroyLoginWidget();
    }

    CurrentScreen = Screen;
    ScreenTimer = 0.0f;

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    if (Screen == ESBFlowScreen::Login)
    {
        // Show UMG login widget
        CreateLoginWidget();
        PC->SetShowMouseCursor(true);
        PC->SetInputMode(FInputModeUIOnly());
    }
    else if (Screen == ESBFlowScreen::InGame)
    {
        PC->SetShowMouseCursor(false);
        PC->SetInputMode(FInputModeGameOnly());
        APawn* Pawn = PC->GetPawn();
        if (Pawn)
        {
            Pawn->SetActorHiddenInGame(false);
            Pawn->SetActorEnableCollision(true);
            Pawn->EnableInput(PC);
        }
        ASBBattleRoyaleGameMode* GM = Cast<ASBBattleRoyaleGameMode>(UGameplayStatics::GetGameMode(this));
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

// --- Login Widget Management ---

void ASBGameFlowHUD::CreateLoginWidget()
{
    if (LoginWidget) return; // already exists

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    LoginWidget = CreateWidget<UIslandOfDeathLoginWidget>(PC, UIslandOfDeathLoginWidget::StaticClass());
    if (LoginWidget)
    {
        // Bind all login button delegates to our handler
        LoginWidget->OnGoogleLoginClicked.AddDynamic(this, &ASBGameFlowHUD::OnLoginButtonPressed);
        LoginWidget->OnFacebookLoginClicked.AddDynamic(this, &ASBGameFlowHUD::OnLoginButtonPressed);
        LoginWidget->OnAppleLoginClicked.AddDynamic(this, &ASBGameFlowHUD::OnLoginButtonPressed);
        LoginWidget->OnGuestLoginClicked.AddDynamic(this, &ASBGameFlowHUD::OnLoginButtonPressed);

        LoginWidget->AddToViewport(10);

        UE_LOG(LogStormBreaker, Log, TEXT("Login widget created and added to viewport"));
    }
}

void ASBGameFlowHUD::DestroyLoginWidget()
{
    if (LoginWidget)
    {
        LoginWidget->RemoveFromParent();
        LoginWidget = nullptr;
        UE_LOG(LogStormBreaker, Log, TEXT("Login widget removed"));
    }
}

void ASBGameFlowHUD::OnLoginButtonPressed()
{
    // Login via backend
    UGameInstance* GI = GetGameInstance();
    if (GI)
    {
        USBBackendSubsystem* Backend = GI->GetSubsystem<USBBackendSubsystem>();
        if (Backend)
        {
            Backend->Login(ESBAuthProvider::Guest);
        }
    }

    // Smooth transition to loading screen
    StartTransition(ESBFlowScreen::Loading, 0.8f);
}

void ASBGameFlowHUD::DrawTransitionOverlay()
{
    if (!bTransitioning) return;
    float HalfDur = TransitionDuration * 0.5f;
    float Alpha = bTransitionFadingOut
        ? FMath::Clamp(TransitionTimer / HalfDur, 0.0f, 1.0f)
        : FMath::Clamp(1.0f - (TransitionTimer - HalfDur) / HalfDur, 0.0f, 1.0f);
    DrawRect(FLinearColor(0, 0, 0, Alpha), 0, 0, Canvas->SizeX, Canvas->SizeY);
}

void ASBGameFlowHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    switch (CurrentScreen)
    {
    case ESBFlowScreen::Splash:  DrawSplashScreen();  break;
    case ESBFlowScreen::Login:
        // Login screen is now UMG widget-based — only draw black background
        DrawRect(FLinearColor(0, 0, 0), 0, 0, Canvas->SizeX, Canvas->SizeY);
        break;
    case ESBFlowScreen::Loading: DrawLoadingScreen(); break;
    case ESBFlowScreen::Lobby:   DrawLobbyScreen();   break;
    case ESBFlowScreen::InGame:  DrawInGameHUD();     break;
    }

    DrawTransitionOverlay();

    // Fade-in on screen entry
    if (!bTransitioning && ScreenTimer < 0.6f)
    {
        float FadeIn = 1.0f - FMath::Clamp(ScreenTimer / 0.6f, 0.0f, 1.0f);
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

    DrawRect(FLinearColor(0.01f, 0.015f, 0.03f), 0, 0, SW, SH);

    UFont* LargeFont = GEngine->GetLargeFont();
    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* TinyFont = GEngine->GetTinyFont();
    float TW, TH;

    // Lightning flashes
    bool bFlash = (T > 2.0f && T < 2.15f) || (T > 4.5f && T < 4.6f);
    if (bFlash) DrawRect(FLinearColor(0.4f, 0.5f, 0.8f, 0.2f), 0, 0, SW, SH);

    // Title fade-in
    float TitleAlpha = FMath::Clamp((T - 1.5f) / 2.0f, 0.0f, 1.0f);
    if (TitleAlpha > 0.0f)
    {
        FString Title = TEXT("ISLAND OF DEATH");
        Canvas->TextSize(LargeFont, Title, TW, TH);
        Canvas->SetDrawColor(FColor(220, 230, 245, (uint8)(255 * TitleAlpha)));
        Canvas->DrawText(LargeFont, Title, (SW - TW) * 0.5f, SH * 0.38f);
    }

    // Tagline
    float TagAlpha = FMath::Clamp((T - 4.0f) / 1.5f, 0.0f, 1.0f);
    if (TagAlpha > 0.0f)
    {
        FString Sub = TEXT("Some Spirits Don't Seek Revenge... They Seek Survival");
        Canvas->TextSize(SmallFont, Sub, TW, TH);
        Canvas->SetDrawColor(FColor(160, 170, 190, (uint8)(200 * TagAlpha)));
        Canvas->DrawText(SmallFont, Sub, (SW - TW) * 0.5f, SH * 0.52f);
    }

    // Loading bar
    float BarProgress = FMath::Clamp((T - 2.0f) / (SplashDuration - 3.0f), 0.0f, 1.0f);
    float BarW = SW * 0.25f;
    float BarX = (SW - BarW) * 0.5f;
    DrawBar(BarX, SH * 0.62f, BarW, 4, BarProgress,
        FLinearColor(0.2f, 0.5f, 1.0f, 0.8f), FLinearColor(0.1f, 0.1f, 0.15f, 0.5f));

    // Fade edges
    if (T < 1.0f) DrawRect(FLinearColor(0, 0, 0, 1.0f - T), 0, 0, SW, SH);
    if (T > SplashDuration - 1.5f)
    {
        float A = (T - (SplashDuration - 1.5f)) / 1.5f;
        DrawRect(FLinearColor(0, 0, 0, A), 0, 0, SW, SH);
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

    // Background
    DrawRect(FLinearColor(0.02f, 0.02f, 0.03f), 0, 0, SW, SH);
    for (int32 i = 0; i < 8; i++)
    {
        float Y = i * (SH * 0.05f);
        float A = 0.06f * (1.0f - (float)i / 8.0f);
        DrawRect(FLinearColor(0.3f, 0.05f, 0.02f, A), 0, Y, SW, SH * 0.05f);
    }

    DrawCornerAccents(SW, SH, 40.0f, FLinearColor(0.7f, 0.1f, 0.05f, 0.6f));

    // Title
    FString TitleTop = TEXT("ISLAND");
    Canvas->TextSize(LargeFont, TitleTop, TW, TH);
    float TitleCenterX = SW * 0.5f;
    float TitleY = SH * 0.12f;
    Canvas->SetDrawColor(FColor(230, 230, 240));
    Canvas->DrawText(LargeFont, TitleTop, TitleCenterX - TW * 0.5f, TitleY);

    FString TitleOf = TEXT("OF");
    Canvas->TextSize(MedFont, TitleOf, TW, TH);
    Canvas->SetDrawColor(FColor(200, 200, 210, 200));
    Canvas->DrawText(MedFont, TitleOf, TitleCenterX - TW * 0.5f, TitleY + 38);

    FString TitleDeath = TEXT("DEATH");
    Canvas->TextSize(LargeFont, TitleDeath, TW, TH);
    Canvas->SetDrawColor(FColor(200, 30, 20));
    Canvas->DrawText(LargeFont, TitleDeath, TitleCenterX - TW * 0.5f, TitleY + 55);

    // Tagline
    float TagY = TitleY + 115;
    Canvas->SetDrawColor(FColor(200, 200, 200, 220));
    Canvas->DrawText(TinyFont, TEXT("Some Spirits Don't seek revenge..."), TitleCenterX - 140, TagY);
    Canvas->SetDrawColor(FColor(220, 50, 30));
    Canvas->DrawText(TinyFont, TEXT("they seek survival"), TitleCenterX + 65, TagY);

    // Buttons
    float BtnW = FMath::Clamp(SW * 0.35f, 300.0f, 450.0f);
    float BtnH = 48.0f;
    float BtnX = (SW - BtnW) * 0.5f;
    float BtnGap = 12.0f;

    float GoogleY = SH * 0.45f;
    DrawLoginButton(BtnX, GoogleY, BtnW, BtnH, TEXT("G"), TEXT("CONTINUE WITH GOOGLE"),
        FLinearColor(0.92f, 0.92f, 0.90f), FLinearColor(0.7f, 0.7f, 0.68f), FLinearColor(0.15f, 0.15f, 0.15f));
    if (IsMouseInRect(BtnX, GoogleY, BtnW, BtnH) && WasMouseClicked())
    {
        UGameInstance* GI = GetGameInstance();
        if (GI) { USBBackendSubsystem* B = GI->GetSubsystem<USBBackendSubsystem>(); if (B) B->Login(ESBAuthProvider::EOS); }
        StartTransition(ESBFlowScreen::Loading);
    }

    float FbY = GoogleY + BtnH + BtnGap;
    DrawLoginButton(BtnX, FbY, BtnW, BtnH, TEXT("f"), TEXT("CONTINUE WITH FACEBOOK"),
        FLinearColor(0.15f, 0.35f, 0.70f), FLinearColor(0.2f, 0.45f, 0.85f), FLinearColor(1, 1, 1));
    if (IsMouseInRect(BtnX, FbY, BtnW, BtnH) && WasMouseClicked())
    {
        UGameInstance* GI = GetGameInstance();
        if (GI) { USBBackendSubsystem* B = GI->GetSubsystem<USBBackendSubsystem>(); if (B) B->Login(ESBAuthProvider::EOS); }
        StartTransition(ESBFlowScreen::Loading);
    }

    float AppleY = FbY + BtnH + BtnGap;
    DrawLoginButton(BtnX, AppleY, BtnW, BtnH, TEXT("A"), TEXT("CONTINUE WITH APPLE"),
        FLinearColor(0.12f, 0.12f, 0.14f), FLinearColor(0.35f, 0.35f, 0.38f), FLinearColor(1, 1, 1));
    if (IsMouseInRect(BtnX, AppleY, BtnW, BtnH) && WasMouseClicked())
    {
        UGameInstance* GI = GetGameInstance();
        if (GI) { USBBackendSubsystem* B = GI->GetSubsystem<USBBackendSubsystem>(); if (B) B->Login(ESBAuthProvider::EOS); }
        StartTransition(ESBFlowScreen::Loading);
    }

    // OR separator
    float OrY = AppleY + BtnH + 18;
    DrawSeparatorLine(BtnX, OrY, BtnW, TEXT("OR"));

    // Guest button
    float GuestY = OrY + 30;
    float GuestH = 55;
    bool bGuestHover = IsMouseInRect(BtnX, GuestY, BtnW, GuestH);
    DrawRect(bGuestHover ? FLinearColor(0.15f, 0.04f, 0.03f) : FLinearColor(0.08f, 0.03f, 0.02f),
        BtnX, GuestY, BtnW, GuestH);
    DrawRoundedBorder(BtnX, GuestY, BtnW, GuestH, FLinearColor(0.7f, 0.12f, 0.08f, bGuestHover ? 1.0f : 0.7f));

    Canvas->SetDrawColor(FColor(220, 50, 30));
    Canvas->DrawText(SmallFont, TEXT(">>"), BtnX + 18, GuestY + 8);
    Canvas->DrawText(SmallFont, TEXT("PLAY AS GUEST"), BtnX + 55, GuestY + 8);
    Canvas->SetDrawColor(FColor(140, 140, 140));
    Canvas->DrawText(TinyFont, TEXT("Jump into the island as a guest"), BtnX + 55, GuestY + 30);
    Canvas->SetDrawColor(FColor(200, 50, 30));
    Canvas->DrawText(SmallFont, TEXT(">"), BtnX + BtnW - 30, GuestY + 14);

    if (IsMouseInRect(BtnX, GuestY, BtnW, GuestH) && WasMouseClicked())
    {
        UGameInstance* GI = GetGameInstance();
        if (GI) { USBBackendSubsystem* B = GI->GetSubsystem<USBBackendSubsystem>(); if (B) B->Login(ESBAuthProvider::Guest); }
        StartTransition(ESBFlowScreen::Loading);
    }

    // Disclaimer
    float DisY = GuestY + GuestH + 20;
    FString D1 = TEXT("Your progress as a guest will be");
    Canvas->TextSize(TinyFont, D1, TW, TH);
    Canvas->SetDrawColor(FColor(90, 90, 100));
    Canvas->DrawText(TinyFont, D1, (SW - TW) * 0.5f, DisY);
    FString D2 = TEXT("stored on this device only.");
    Canvas->TextSize(TinyFont, D2, TW, TH);
    Canvas->DrawText(TinyFont, D2, (SW - TW) * 0.5f, DisY + 16);
}

// ============================================================================
// LOADING SCREEN — Graphics Quality + Resource Download
// ============================================================================

void ASBGameFlowHUD::DrawLoadingScreen()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;

    UFont* LargeFont = GEngine->GetLargeFont();
    UFont* MedFont = GEngine->GetMediumFont();
    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* TinyFont = GEngine->GetTinyFont();
    float TW, TH;

    // Dark background
    DrawRect(FLinearColor(0.02f, 0.025f, 0.04f), 0, 0, SW, SH);

    // Subtle animated particles (dots floating up)
    for (int32 i = 0; i < 15; i++)
    {
        float Seed = i * 137.5f;
        float PX = FMath::Fmod(Seed * 7.3f, SW);
        float PY = FMath::Fmod(SH - FMath::Fmod(ScreenTimer * (20 + i * 3) + Seed, SH + 50), SH);
        float PA = 0.15f + 0.1f * FMath::Sin(ScreenTimer + Seed);
        DrawRect(FLinearColor(0.3f, 0.5f, 0.8f, PA), PX, PY, 2, 2);
    }

    // Title
    FString Title = TEXT("ISLAND OF DEATH");
    Canvas->TextSize(LargeFont, Title, TW, TH);
    Canvas->SetDrawColor(FColor(220, 220, 230));
    Canvas->DrawText(LargeFont, Title, (SW - TW) * 0.5f, SH * 0.08f);

    // ====== GRAPHICS QUALITY SELECTOR ======
    float PanelW = FMath::Clamp(SW * 0.6f, 400.0f, 700.0f);
    float PanelH = 280.0f;
    float PanelX = (SW - PanelW) * 0.5f;
    float PanelY = SH * 0.18f;

    DrawPanel(PanelX, PanelY, PanelW, PanelH,
        FLinearColor(0.04f, 0.04f, 0.06f, 0.9f), FLinearColor(0.2f, 0.15f, 0.1f, 0.5f));

    // Section title
    FString QualTitle = TEXT("SELECT GRAPHICS QUALITY");
    Canvas->TextSize(SmallFont, QualTitle, TW, TH);
    Canvas->SetDrawColor(FColor(255, 220, 50));
    Canvas->DrawText(SmallFont, QualTitle, PanelX + (PanelW - TW) * 0.5f, PanelY + 15);

    // Quality options
    struct FQualityOption
    {
        ESBGraphicsQuality Quality;
        FString Name;
        FString Desc;
        FString Size;
        FLinearColor Color;
    };

    FQualityOption Options[] = {
        { ESBGraphicsQuality::Low,     TEXT("SMOOTH"),    TEXT("Best performance, low textures"),    TEXT("320 MB"), FLinearColor(0.4f, 0.7f, 0.3f) },
        { ESBGraphicsQuality::Medium,  TEXT("BALANCED"),  TEXT("Good visuals & performance"),        TEXT("520 MB"), FLinearColor(0.3f, 0.6f, 0.8f) },
        { ESBGraphicsQuality::HD,      TEXT("HD"),        TEXT("High quality textures"),             TEXT("680 MB"), FLinearColor(0.7f, 0.5f, 0.2f) },
        { ESBGraphicsQuality::HDR,     TEXT("HDR"),       TEXT("HDR lighting + high textures"),      TEXT("850 MB"), FLinearColor(0.8f, 0.3f, 0.2f) },
        { ESBGraphicsQuality::UltraHD, TEXT("ULTRA HD"),  TEXT("Maximum quality, 4K textures"),      TEXT("1.2 GB"), FLinearColor(0.9f, 0.2f, 0.6f) },
    };

    float OptStartY = PanelY + 50;
    float OptH = 40.0f;
    float OptGap = 5.0f;

    for (int32 i = 0; i < 5; i++)
    {
        float OY = OptStartY + i * (OptH + OptGap);
        float OX = PanelX + 15;
        float OW = PanelW - 30;

        bool bSelected = (SelectedQuality == Options[i].Quality);
        bool bHovered = IsMouseInRect(OX, OY, OW, OptH);

        // Background
        FLinearColor OptBg = bSelected
            ? FLinearColor(Options[i].Color.R * 0.2f, Options[i].Color.G * 0.2f, Options[i].Color.B * 0.2f, 0.8f)
            : (bHovered ? FLinearColor(0.08f, 0.08f, 0.1f) : FLinearColor(0.05f, 0.05f, 0.07f));
        DrawRect(OptBg, OX, OY, OW, OptH);

        // Selection border
        if (bSelected)
        {
            DrawRoundedBorder(OX, OY, OW, OptH, Options[i].Color, 2.0f);
        }

        // Radio circle
        float CircleX = OX + 20;
        float CircleY = OY + OptH * 0.5f - 5;
        DrawRect(FLinearColor(0.3f, 0.3f, 0.35f), CircleX - 5, CircleY - 5, 10, 10);
        if (bSelected)
        {
            DrawRect(Options[i].Color, CircleX - 3, CircleY - 3, 6, 6);
        }

        // Name
        Canvas->SetDrawColor(bSelected ? FColor::White : FColor(180, 180, 180));
        Canvas->DrawText(SmallFont, Options[i].Name, OX + 40, OY + 5);

        // Description
        Canvas->SetDrawColor(FColor(120, 120, 130));
        Canvas->DrawText(TinyFont, Options[i].Desc, OX + 40, OY + 23);

        // Size (right side)
        Canvas->TextSize(TinyFont, Options[i].Size, TW, TH);
        Canvas->SetDrawColor(FColor(150, 150, 160));
        Canvas->DrawText(TinyFont, Options[i].Size, OX + OW - TW - 15, OY + 12);

        // Click handler
        if (bHovered && WasMouseClicked())
        {
            SelectedQuality = Options[i].Quality;
        }
    }

    // ====== DOWNLOAD SECTION ======
    float DlY = PanelY + PanelH + 30;
    float DlW = PanelW;
    float DlX = PanelX;

    if (!bDownloadStarted)
    {
        // DOWNLOAD RESOURCES button
        float DlBtnH = 55;
        bool bDlHover = IsMouseInRect(DlX, DlY, DlW, DlBtnH);

        DrawRect(bDlHover ? FLinearColor(0.85f, 0.6f, 0.12f) : FLinearColor(0.75f, 0.52f, 0.08f),
            DlX, DlY, DlW, DlBtnH);
        DrawRect(FLinearColor(1, 1, 1, 0.1f), DlX, DlY, DlW, 2);

        FString DlText = TEXT("DOWNLOAD RESOURCES");
        Canvas->TextSize(MedFont, DlText, TW, TH);
        Canvas->SetDrawColor(FColor::White);
        Canvas->DrawText(MedFont, DlText, DlX + (DlW - TW) * 0.5f, DlY + (DlBtnH - TH) * 0.5f);

        // Size info
        FString SizeInfo;
        switch (SelectedQuality)
        {
        case ESBGraphicsQuality::Low:     SizeInfo = TEXT("Download: 320 MB"); break;
        case ESBGraphicsQuality::Medium:  SizeInfo = TEXT("Download: 520 MB"); break;
        case ESBGraphicsQuality::HD:      SizeInfo = TEXT("Download: 680 MB"); break;
        case ESBGraphicsQuality::HDR:     SizeInfo = TEXT("Download: 850 MB"); break;
        case ESBGraphicsQuality::UltraHD: SizeInfo = TEXT("Download: 1.2 GB"); break;
        }
        Canvas->TextSize(TinyFont, SizeInfo, TW, TH);
        Canvas->SetDrawColor(FColor(180, 180, 180));
        Canvas->DrawText(TinyFont, SizeInfo, DlX + (DlW - TW) * 0.5f, DlY + DlBtnH + 10);

        if (bDlHover && WasMouseClicked())
        {
            bDownloadStarted = true;
            DownloadProgress = 0.0f;
            DownloadSpeed = 10.0f;
        }
    }
    else if (!bDownloadComplete)
    {
        // Download progress
        DrawPanel(DlX, DlY, DlW, 90,
            FLinearColor(0.04f, 0.04f, 0.06f, 0.9f), FLinearColor(0.2f, 0.15f, 0.1f, 0.3f));

        // Progress bar
        float BarY = DlY + 15;
        DrawBar(DlX + 20, BarY, DlW - 40, 20, DownloadProgress,
            FLinearColor(0.2f, 0.7f, 0.3f, 0.9f), FLinearColor(0.1f, 0.1f, 0.12f));

        // Percentage
        FString PctStr = FString::Printf(TEXT("%.1f%%"), DownloadProgress * 100.0f);
        Canvas->TextSize(SmallFont, PctStr, TW, TH);
        Canvas->SetDrawColor(FColor::White);
        Canvas->DrawText(SmallFont, PctStr, DlX + (DlW - TW) * 0.5f, BarY + 1);

        // Speed + remaining
        float TotalMB;
        switch (SelectedQuality)
        {
        case ESBGraphicsQuality::Low:     TotalMB = 320; break;
        case ESBGraphicsQuality::Medium:  TotalMB = 520; break;
        case ESBGraphicsQuality::HD:      TotalMB = 680; break;
        case ESBGraphicsQuality::HDR:     TotalMB = 850; break;
        case ESBGraphicsQuality::UltraHD: TotalMB = 1200; break;
        default: TotalMB = 680; break;
        }

        float Downloaded = TotalMB * DownloadProgress;
        FString SpeedStr = FString::Printf(TEXT("%.1f MB/s  |  %.0f / %.0f MB"),
            DownloadSpeed, Downloaded, TotalMB);
        Canvas->TextSize(TinyFont, SpeedStr, TW, TH);
        Canvas->SetDrawColor(FColor(160, 160, 170));
        Canvas->DrawText(TinyFont, SpeedStr, DlX + (DlW - TW) * 0.5f, BarY + 40);

        // ETA
        float Remaining = (TotalMB - Downloaded);
        float ETA = (DownloadSpeed > 1.0f) ? Remaining / DownloadSpeed : 999.0f;
        FString ETAStr = FString::Printf(TEXT("ETA: %.0fs"), ETA);
        Canvas->TextSize(TinyFont, ETAStr, TW, TH);
        Canvas->SetDrawColor(FColor(120, 120, 130));
        Canvas->DrawText(TinyFont, ETAStr, DlX + (DlW - TW) * 0.5f, BarY + 58);
    }
    else
    {
        // Download complete — ENTER LOBBY button
        float EnterH = 55;
        bool bEnterHover = IsMouseInRect(DlX, DlY, DlW, EnterH);

        DrawRect(bEnterHover ? FLinearColor(0.2f, 0.75f, 0.3f) : FLinearColor(0.15f, 0.6f, 0.2f),
            DlX, DlY, DlW, EnterH);

        FString EnterText = TEXT("ENTER LOBBY");
        Canvas->TextSize(MedFont, EnterText, TW, TH);
        Canvas->SetDrawColor(FColor::White);
        Canvas->DrawText(MedFont, EnterText, DlX + (DlW - TW) * 0.5f, DlY + (EnterH - TH) * 0.5f);

        // Checkmark
        Canvas->SetDrawColor(FColor(100, 255, 100));
        Canvas->DrawText(SmallFont, TEXT("Download Complete!"), DlX + (DlW - 160) * 0.5f, DlY + EnterH + 10);

        if (bEnterHover && WasMouseClicked())
        {
            StartTransition(ESBFlowScreen::Lobby, 1.0f);
        }
    }

    // Skip button (bottom right) — skip download for testing
    float SkipX = SW - 150;
    float SkipY = SH - 45;
    bool bSkipHover = IsMouseInRect(SkipX, SkipY, 130, 30);
    Canvas->SetDrawColor(bSkipHover ? FColor(200, 200, 200) : FColor(100, 100, 110));
    Canvas->DrawText(TinyFont, TEXT("SKIP >>"), SkipX + 35, SkipY + 8);
    if (bSkipHover && WasMouseClicked())
    {
        StartTransition(ESBFlowScreen::Lobby, 0.6f);
    }

    // Footer
    Canvas->SetDrawColor(FColor(50, 50, 60));
    Canvas->DrawText(TinyFont, TEXT("Island Of Death Games | v1.0.0"), 20, SH - 25);
}

// ============================================================================
// LOBBY SCREEN — BGMI-style
// ============================================================================

void ASBGameFlowHUD::DrawLobbyScreen()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;

    // Dark cinematic background
    DrawRect(FLinearColor(0.03f, 0.035f, 0.05f), 0, 0, SW, SH);

    // Subtle gradient from center
    DrawRect(FLinearColor(0.06f, 0.04f, 0.03f, 0.3f), SW * 0.2f, 0, SW * 0.6f, SH);

    // Draw all lobby sections
    DrawLobbyTopBar();
    DrawLobbyGameLogo();
    DrawLobbyLeftMenu();
    DrawLobbyRightPanels();
    DrawLobbyChatBar();
    DrawLobbyBottomBar();
    DrawLobbyStartButton();
}

void ASBGameFlowHUD::DrawLobbyTopBar()
{
    const float SW = Canvas->SizeX;
    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* TinyFont = GEngine->GetTinyFont();

    // Top bar background
    DrawRect(FLinearColor(0.05f, 0.05f, 0.07f, 0.95f), 0, 0, SW, 55);
    DrawRect(FLinearColor(0.7f, 0.5f, 0.1f, 0.3f), 0, 53, SW, 2); // gold accent

    // Player info
    USBBackendSubsystem* Backend = nullptr;
    UGameInstance* GI = GetGameInstance();
    if (GI) Backend = GI->GetSubsystem<USBBackendSubsystem>();

    FString PlayerName = TEXT("PLAYER");
    int32 Level = 1;
    int32 Coins = 0;
    int32 UC = 0;

    if (Backend)
    {
        const FSBPlayerProfile& P = Backend->GetProfile();
        PlayerName = P.Username.ToUpper();
        Level = P.Level;
        Coins = P.Coins;
        UC = P.Gems;
    }

    // Avatar placeholder (square)
    DrawRect(FLinearColor(0.15f, 0.15f, 0.2f), 10, 5, 45, 45);
    DrawRoundedBorder(10, 5, 45, 45, FLinearColor(0.6f, 0.5f, 0.2f));

    // Player name + level
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(SmallFont, PlayerName, 65, 8);

    FString LvStr = FString::Printf(TEXT("Lv.%d"), Level);
    Canvas->SetDrawColor(FColor(200, 200, 200));
    Canvas->DrawText(TinyFont, LvStr, 65, 28);

    // XP bar
    DrawBar(65, 42, 100, 6, 0.65f, FLinearColor(0.8f, 0.6f, 0.1f), FLinearColor(0.15f, 0.15f, 0.2f));

    // VIP badge
    DrawBadge(175, 10, TEXT("VIP 3"), FLinearColor(0.7f, 0.5f, 0.1f), FLinearColor(0, 0, 0));

    // Currency (right side)
    float CurrX = SW - 350;

    // BP coins
    DrawRect(FLinearColor(0.1f, 0.1f, 0.12f, 0.8f), CurrX, 12, 100, 28);
    Canvas->SetDrawColor(FColor(255, 220, 50));
    Canvas->DrawText(TinyFont, TEXT("BP"), CurrX + 8, 17);
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(TinyFont, FString::Printf(TEXT("%d"), Coins), CurrX + 30, 17);

    // UC
    DrawRect(FLinearColor(0.1f, 0.1f, 0.12f, 0.8f), CurrX + 110, 12, 100, 28);
    Canvas->SetDrawColor(FColor(255, 200, 50));
    Canvas->DrawText(TinyFont, TEXT("UC"), CurrX + 118, 17);
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(TinyFont, FString::Printf(TEXT("%d"), UC), CurrX + 140, 17);

    // + button
    DrawRect(FLinearColor(0.6f, 0.5f, 0.1f), CurrX + 220, 14, 24, 24);
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(TinyFont, TEXT("+"), CurrX + 228, 17);

    // Icons (right side)
    float IconX = SW - 100;
    // Settings gear
    Canvas->SetDrawColor(FColor(180, 180, 190));
    Canvas->DrawText(TinyFont, TEXT("[S]"), IconX, 18);
    // Ping
    Canvas->SetDrawColor(CachedFPS >= 30 ? FColor(100, 255, 100) : FColor(255, 100, 100));
    FString PingStr = TEXT("32ms");
    APlayerController* PC = GetOwningPlayerController();
    if (PC && PC->PlayerState)
    {
        PingStr = FString::Printf(TEXT("%.0fms"), PC->PlayerState->GetPingInMilliseconds());
    }
    Canvas->DrawText(TinyFont, PingStr, IconX + 35, 18);
}

void ASBGameFlowHUD::DrawLobbyGameLogo()
{
    const float SW = Canvas->SizeX;
    UFont* LargeFont = GEngine->GetLargeFont();
    UFont* MedFont = GEngine->GetMediumFont();
    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* TinyFont = GEngine->GetTinyFont();

    // Logo panel (top left, below top bar)
    float LogoX = 10;
    float LogoY = 65;
    float LogoW = 200;
    float LogoH = 120;

    DrawPanel(LogoX, LogoY, LogoW, LogoH,
        FLinearColor(0.05f, 0.04f, 0.03f, 0.85f), FLinearColor(0.6f, 0.4f, 0.1f, 0.5f));

    // "ISLAND OF DEATH" in logo style
    Canvas->SetDrawColor(FColor(230, 230, 235));
    Canvas->DrawText(SmallFont, TEXT("ISLAND OF"), LogoX + 15, LogoY + 10);
    Canvas->SetDrawColor(FColor(200, 30, 20));
    Canvas->DrawText(MedFont, TEXT("DEATH"), LogoX + 15, LogoY + 30);

    // Tagline
    Canvas->SetDrawColor(FColor(180, 180, 180, 180));
    Canvas->DrawText(TinyFont, TEXT("Some Spirits Don't seek"), LogoX + 15, LogoY + 65);
    Canvas->DrawText(TinyFont, TEXT("revenge..."), LogoX + 15, LogoY + 80);
    Canvas->SetDrawColor(FColor(200, 50, 30));
    Canvas->DrawText(TinyFont, TEXT("they seek survival"), LogoX + 15, LogoY + 95);
}

void ASBGameFlowHUD::DrawLobbyLeftMenu()
{
    const float SH = Canvas->SizeY;
    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* TinyFont = GEngine->GetTinyFont();

    struct FMenuItem
    {
        FString Icon;
        FString Label;
        bool bHasNotif;
        FString NotifText;
    };

    FMenuItem Items[] = {
        { TEXT("[>]"),  TEXT("START"),     false, TEXT("") },
        { TEXT("[W]"),  TEXT("LOADOUT"),   false, TEXT("") },
        { TEXT("[C]"),  TEXT("CHARACTER"), false, TEXT("") },
        { TEXT("[I]"),  TEXT("INVENTORY"), false, TEXT("") },
        { TEXT("[M]"),  TEXT("MISSIONS"),  true,  TEXT("") },
        { TEXT("[RP]"), TEXT("SEASON"),    true,  TEXT("") },
        { TEXT("[CL]"), TEXT("CLAN"),      false, TEXT("") },
        { TEXT("[E]"),  TEXT("EVENTS"),    true,  TEXT("NEW") },
        { TEXT("[S]"),  TEXT("STORE"),     false, TEXT("") },
    };

    float MenuX = 10;
    float MenuY = 195;
    float ItemW = 200;
    float ItemH = 36;
    float ItemGap = 2;

    for (int32 i = 0; i < 9; i++)
    {
        float IY = MenuY + i * (ItemH + ItemGap);
        bool bSelected = (i == LobbyMenuIndex);
        bool bHovered = IsMouseInRect(MenuX, IY, ItemW, ItemH);

        // Background
        if (bSelected)
        {
            DrawRect(FLinearColor(0.75f, 0.55f, 0.1f, 0.9f), MenuX, IY, ItemW, ItemH);
        }
        else if (bHovered)
        {
            DrawRect(FLinearColor(0.1f, 0.1f, 0.12f, 0.8f), MenuX, IY, ItemW, ItemH);
        }
        else
        {
            DrawRect(FLinearColor(0.05f, 0.05f, 0.07f, 0.6f), MenuX, IY, ItemW, ItemH);
        }

        // Left accent for selected
        if (bSelected)
        {
            DrawRect(FLinearColor(0.9f, 0.7f, 0.1f), MenuX, IY, 3, ItemH);
        }

        // Icon
        Canvas->SetDrawColor(bSelected ? FColor(30, 30, 30) : FColor(180, 180, 180));
        Canvas->DrawText(TinyFont, Items[i].Icon, MenuX + 10, IY + 10);

        // Label
        Canvas->SetDrawColor(bSelected ? FColor(20, 20, 20) : FColor(220, 220, 220));
        Canvas->DrawText(SmallFont, Items[i].Label, MenuX + 45, IY + 7);

        // Notification badge
        if (Items[i].bHasNotif)
        {
            if (Items[i].NotifText.Len() > 0)
            {
                DrawBadge(MenuX + ItemW - 50, IY + 8, Items[i].NotifText, FLinearColor(0.8f, 0.15f, 0.1f), FLinearColor(1, 1, 1));
            }
            else
            {
                // Red dot
                DrawRect(FLinearColor(0.9f, 0.15f, 0.1f), MenuX + ItemW - 20, IY + 12, 8, 8);
            }
        }

        if (bHovered && WasMouseClicked())
        {
            LobbyMenuIndex = i;
        }
    }
}

void ASBGameFlowHUD::DrawLobbyRightPanels()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;
    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* MedFont = GEngine->GetMediumFont();
    UFont* TinyFont = GEngine->GetTinyFont();

    float PanelW = 250;
    float PanelX = SW - PanelW - 10;

    // ====== RANK PUSH PANEL ======
    float RpY = 65;
    float RpH = 80;
    DrawPanel(PanelX, RpY, PanelW, RpH,
        FLinearColor(0.06f, 0.04f, 0.03f, 0.9f), FLinearColor(0.5f, 0.3f, 0.1f, 0.4f));

    Canvas->SetDrawColor(FColor(255, 220, 50));
    Canvas->DrawText(SmallFont, TEXT("RANK PUSH"), PanelX + 15, RpY + 10);
    Canvas->SetDrawColor(FColor(200, 50, 30));
    Canvas->DrawText(TinyFont, TEXT("SEASON 12"), PanelX + 15, RpY + 30);

    // RP level
    Canvas->SetDrawColor(FColor(255, 220, 50));
    Canvas->DrawText(TinyFont, TEXT("RP"), PanelX + 15, RpY + 52);
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(MedFont, TEXT("50"), PanelX + 40, RpY + 45);
    DrawBar(PanelX + 80, RpY + 55, 100, 8, 0.8f,
        FLinearColor(0.8f, 0.6f, 0.1f), FLinearColor(0.15f, 0.15f, 0.2f));
    Canvas->SetDrawColor(FColor(150, 150, 150));
    Canvas->DrawText(TinyFont, TEXT("80/100"), PanelX + 185, RpY + 52);

    // ====== DAILY SPECIAL BUNDLE ======
    float DsY = RpY + RpH + 10;
    float DsH = 80;
    DrawPanel(PanelX, DsY, PanelW, DsH,
        FLinearColor(0.05f, 0.03f, 0.05f, 0.9f), FLinearColor(0.4f, 0.15f, 0.3f, 0.4f));

    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(SmallFont, TEXT("DAILY SPECIAL BUNDLE"), PanelX + 15, DsY + 10);
    DrawBadge(PanelX + PanelW - 55, DsY + 10, TEXT("-70%"), FLinearColor(0.8f, 0.15f, 0.1f), FLinearColor(1, 1, 1));

    // Character slots
    for (int32 i = 0; i < 3; i++)
    {
        float SlotX = PanelX + 15 + i * 55;
        DrawRect(FLinearColor(0.1f, 0.1f, 0.12f), SlotX, DsY + 38, 45, 35);
        DrawRoundedBorder(SlotX, DsY + 38, 45, 35, FLinearColor(0.3f, 0.3f, 0.35f));
    }

    // ====== EVENTS PANEL ======
    float EvY = DsY + DsH + 10;
    float EvH = 70;
    DrawPanel(PanelX, EvY, PanelW, EvH,
        FLinearColor(0.04f, 0.04f, 0.06f, 0.9f), FLinearColor(0.3f, 0.15f, 0.1f, 0.4f));

    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(SmallFont, TEXT("EVENTS"), PanelX + 15, EvY + 8);
    DrawBadge(PanelX + PanelW - 50, EvY + 8, TEXT("HOT"), FLinearColor(0.8f, 0.15f, 0.1f), FLinearColor(1, 1, 1));

    Canvas->SetDrawColor(FColor(200, 50, 30));
    Canvas->DrawText(SmallFont, TEXT("ZOMBIE SURVIVAL"), PanelX + 15, EvY + 32);
    Canvas->SetDrawColor(FColor(220, 50, 30));
    Canvas->DrawText(TinyFont, TEXT("LIVE NOW"), PanelX + 15, EvY + 50);

    // ====== BATTLE ROYALE MODE PANEL ======
    float BrY = EvY + EvH + 10;
    float BrH = 70;
    DrawPanel(PanelX, BrY, PanelW, BrH,
        FLinearColor(0.04f, 0.04f, 0.06f, 0.9f), FLinearColor(0.2f, 0.2f, 0.25f, 0.4f));

    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(SmallFont, TEXT("BATTLE ROYALE"), PanelX + 15, BrY + 10);
    Canvas->SetDrawColor(FColor(180, 180, 180));
    Canvas->DrawText(TinyFont, TEXT("CLASSIC"), PanelX + 15, BrY + 30);

    // TPP + Squad info
    Canvas->SetDrawColor(FColor(150, 150, 160));
    Canvas->DrawText(TinyFont, TEXT("TPP"), PanelX + 15, BrY + 48);

    // Squad size selector
    FString SquadStr = TEXT("Solo");
    if (SelectedSquadSize == 2) SquadStr = TEXT("Duo");
    else if (SelectedSquadSize == 4) SquadStr = TEXT("Squad");
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(TinyFont, SquadStr, PanelX + 60, BrY + 48);

    // Bot count
    Canvas->SetDrawColor(FColor(255, 220, 50));
    Canvas->DrawText(TinyFont, FString::Printf(TEXT("Bots: %d"), SelectedBotCount), PanelX + 130, BrY + 48);
}

void ASBGameFlowHUD::DrawLobbyChatBar()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;
    UFont* TinyFont = GEngine->GetTinyFont();

    float ChatY = SH - 80;
    float ChatW = SW * 0.55f;
    float ChatH = 28;

    DrawRect(FLinearColor(0.04f, 0.04f, 0.06f, 0.8f), 10, ChatY, ChatW, ChatH);
    DrawRoundedBorder(10, ChatY, ChatW, ChatH, FLinearColor(0.2f, 0.2f, 0.25f, 0.4f));

    Canvas->SetDrawColor(FColor(100, 100, 110));
    Canvas->DrawText(TinyFont, TEXT("[...]"), 18, ChatY + 7);

    Canvas->SetDrawColor(FColor(180, 180, 180));
    Canvas->DrawText(TinyFont, ChatMessage, 45, ChatY + 7);
}

void ASBGameFlowHUD::DrawLobbyBottomBar()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;
    UFont* TinyFont = GEngine->GetTinyFont();
    UFont* SmallFont = GEngine->GetSmallFont();

    // Bottom bar
    float BarY = SH - 45;
    DrawRect(FLinearColor(0.04f, 0.04f, 0.06f, 0.9f), 0, BarY, SW, 45);
    DrawRect(FLinearColor(0.3f, 0.25f, 0.1f, 0.3f), 0, BarY, SW, 1);

    // Tab buttons
    struct FTab { FString Icon; FString Label; };
    FTab Tabs[] = {
        { TEXT("[R]"), TEXT("RANK") },
        { TEXT("[T]"), TEXT("ACHIEVEMENTS") },
        { TEXT("[W]"), TEXT("WORKSHOP") },
        { TEXT("[P]"), TEXT("SOCIAL") },
    };

    float TabX = 20;
    for (int32 i = 0; i < 4; i++)
    {
        float TabW = 110;
        bool bHover = IsMouseInRect(TabX, BarY + 5, TabW, 35);

        Canvas->SetDrawColor(bHover ? FColor(255, 220, 50) : FColor(160, 160, 170));
        Canvas->DrawText(TinyFont, Tabs[i].Icon, TabX, BarY + 8);
        Canvas->DrawText(TinyFont, Tabs[i].Label, TabX + 25, BarY + 8);

        TabX += TabW + 10;
    }

    // FIRST TOP-UP
    Canvas->SetDrawColor(FColor(255, 220, 50));
    Canvas->DrawText(TinyFont, TEXT("FIRST TOP-UP"), SW * 0.5f, BarY + 15);
}

void ASBGameFlowHUD::DrawLobbyStartButton()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;
    UFont* LargeFont = GEngine->GetLargeFont();
    UFont* MedFont = GEngine->GetMediumFont();
    float TW, TH;

    // Big START button (bottom right)
    float BtnW = 200;
    float BtnH = 65;
    float BtnX = SW - BtnW - 15;
    float BtnY = SH - BtnH - 55;

    bool bHovered = IsMouseInRect(BtnX, BtnY, BtnW, BtnH);

    // Yellow/gold gradient button
    FLinearColor BtnColor = bHovered
        ? FLinearColor(0.95f, 0.75f, 0.15f)
        : FLinearColor(0.85f, 0.62f, 0.08f);

    DrawRect(BtnColor, BtnX, BtnY, BtnW, BtnH);

    // Darker bottom half for depth
    DrawRect(FLinearColor(0, 0, 0, 0.15f), BtnX, BtnY + BtnH * 0.5f, BtnW, BtnH * 0.5f);

    // Border
    DrawRoundedBorder(BtnX, BtnY, BtnW, BtnH,
        FLinearColor(0.9f, 0.7f, 0.2f), 2);

    // Highlight on top
    DrawRect(FLinearColor(1, 1, 1, 0.2f), BtnX + 2, BtnY + 2, BtnW - 4, 2);

    // "START" text
    FString StartStr = TEXT("START");
    Canvas->TextSize(MedFont, StartStr, TW, TH);
    Canvas->SetDrawColor(FColor(20, 15, 5));
    Canvas->DrawText(MedFont, StartStr, BtnX + (BtnW - TW) * 0.5f, BtnY + (BtnH - TH) * 0.5f);

    // Arrow
    Canvas->DrawText(MedFont, TEXT(">"), BtnX + BtnW - 35, BtnY + (BtnH - TH) * 0.5f);

    if (bHovered && WasMouseClicked())
    {
        StartTransition(ESBFlowScreen::InGame, 1.2f);
    }
}

// ============================================================================
// IN-GAME HUD
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
    const float BarH = 16;
    float Y = SH - 30;

    float Health = 100, MaxHealth = 100, Shield = 0, MaxShield = 150;
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
    FString WeaponName, FireMode;

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
    float X = SW - 20;
    float Y = SH - 30;

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
            float TW2, TH2;
            Canvas->TextSize(Font, D.Value, TW2, TH2);
            Canvas->SetDrawColor(FColor::White);
            Canvas->DrawText(Font, D.Value, SX - TW2 * 0.5f, 30);
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
// HELPERS
// ============================================================================

void ASBGameFlowHUD::DrawBar(float X, float Y, float W, float H, float Pct, FLinearColor Fill, FLinearColor Bg)
{
    DrawRect(Bg, X, Y, W, H);
    DrawRect(Fill, X, Y, W * FMath::Clamp(Pct, 0.0f, 1.0f), H);
}

void ASBGameFlowHUD::DrawButton(float X, float Y, float W, float H, const FString& Text, FLinearColor Color)
{
    bool bHovered = IsMouseInRect(X, Y, W, H);
    DrawRect(bHovered ? Color * 1.3f : Color, X, Y, W, H);
    DrawRect(FLinearColor(1, 1, 1, 0.3f), X, Y, W, 2);

    UFont* Font = GEngine->GetSmallFont();
    float TW, TH;
    Canvas->TextSize(Font, Text, TW, TH);
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, Text, X + (W - TW) * 0.5f, Y + (H - TH) * 0.5f);
}

void ASBGameFlowHUD::DrawLoginButton(float X, float Y, float W, float H,
    const FString& IconText, const FString& Label,
    FLinearColor BgColor, FLinearColor BorderColor, FLinearColor TextColor)
{
    bool bHovered = IsMouseInRect(X, Y, W, H);
    DrawRect(bHovered ? BgColor * 1.15f : BgColor, X, Y, W, H);
    DrawRoundedBorder(X, Y, W, H, BorderColor, bHovered ? 2.0f : 1.0f);
    DrawRect(FLinearColor(1, 1, 1, 0.08f), X + 1, Y + 1, W - 2, 1);

    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* MedFont = GEngine->GetMediumFont();
    float TW, TH;

    uint8 TR = (uint8)(TextColor.R * 255);
    uint8 TG = (uint8)(TextColor.G * 255);
    uint8 TB = (uint8)(TextColor.B * 255);

    Canvas->SetDrawColor(FColor(TR, TG, TB));
    Canvas->DrawText(MedFont, IconText, X + 20, Y + (H - 20) * 0.5f);
    DrawRect(FLinearColor(TextColor.R, TextColor.G, TextColor.B, 0.2f), X + 55, Y + 8, 1, H - 16);

    Canvas->TextSize(SmallFont, Label, TW, TH);
    Canvas->SetDrawColor(FColor(TR, TG, TB));
    Canvas->DrawText(SmallFont, Label, X + 60 + (W - 60 - TW) * 0.5f, Y + (H - TH) * 0.5f);
}

void ASBGameFlowHUD::DrawRoundedBorder(float X, float Y, float W, float H, FLinearColor Color, float Thickness)
{
    DrawRect(Color, X, Y, W, Thickness);
    DrawRect(Color, X, Y + H - Thickness, W, Thickness);
    DrawRect(Color, X, Y, Thickness, H);
    DrawRect(Color, X + W - Thickness, Y, Thickness, H);
}

void ASBGameFlowHUD::DrawCornerAccents(float SW, float SH, float Size, FLinearColor Color)
{
    float T = 2.0f;
    float Ins = 15.0f;
    DrawRect(Color, Ins, Ins, Size, T);
    DrawRect(Color, Ins, Ins, T, Size);
    DrawRect(Color, SW - Ins - Size, Ins, Size, T);
    DrawRect(Color, SW - Ins - T, Ins, T, Size);
    DrawRect(Color, Ins, SH - Ins - T, Size, T);
    DrawRect(Color, Ins, SH - Ins - Size, T, Size);
    DrawRect(Color, SW - Ins - Size, SH - Ins - T, Size, T);
    DrawRect(Color, SW - Ins - T, SH - Ins - Size, T, Size);
}

void ASBGameFlowHUD::DrawSeparatorLine(float X, float Y, float W, const FString& Text)
{
    UFont* TinyFont = GEngine->GetTinyFont();
    float TW, TH;
    Canvas->TextSize(TinyFont, Text, TW, TH);
    float TextX = X + (W - TW) * 0.5f;
    float LineY2 = Y + TH * 0.5f;
    float Gap = 10;
    DrawRect(FLinearColor(0.25f, 0.15f, 0.1f, 0.5f), X, LineY2, TextX - X - Gap, 1);
    DrawRect(FLinearColor(0.25f, 0.15f, 0.1f, 0.5f), TextX + TW + Gap, LineY2, X + W - TextX - TW - Gap, 1);
    Canvas->SetDrawColor(FColor(120, 100, 80));
    Canvas->DrawText(TinyFont, Text, TextX, Y);
}

void ASBGameFlowHUD::DrawPanel(float X, float Y, float W, float H, FLinearColor Bg, FLinearColor Border)
{
    DrawRect(Bg, X, Y, W, H);
    DrawRoundedBorder(X, Y, W, H, Border);
}

void ASBGameFlowHUD::DrawBadge(float X, float Y, const FString& Text, FLinearColor Bg, FLinearColor TextCol)
{
    UFont* TinyFont = GEngine->GetTinyFont();
    float TW, TH;
    Canvas->TextSize(TinyFont, Text, TW, TH);
    float Pad = 6;
    DrawRect(Bg, X, Y, TW + Pad * 2, TH + 4);
    Canvas->SetDrawColor(FColor(
        (uint8)(TextCol.R * 255),
        (uint8)(TextCol.G * 255),
        (uint8)(TextCol.B * 255)));
    Canvas->DrawText(TinyFont, Text, X + Pad, Y + 2);
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
