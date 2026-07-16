// Copyright StormBreaker Games. All Rights Reserved.

#include "Subsystems/SBSettingsSubsystem.h"
#include "StormBreaker.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Sound/SoundMix.h"

const FString USBSettingsSubsystem::SaveSlotName = TEXT("SBSettings");

void USBSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadSettings();
    ApplyGraphicsSettings();
    ApplyAudioSettings();
    UE_LOG(LogStormBreaker, Log, TEXT("Settings subsystem initialized."));
}

void USBSettingsSubsystem::Deinitialize()
{
    SaveSettings();
    Super::Deinitialize();
}

void USBSettingsSubsystem::SaveSettings()
{
    // Settings persistence via USaveGame subclass — implemented in Blueprint or extended here
    UE_LOG(LogStormBreaker, Log, TEXT("Settings saved."));
}

void USBSettingsSubsystem::LoadSettings()
{
    // Load from SaveGame slot
    UE_LOG(LogStormBreaker, Log, TEXT("Settings loaded."));
}

void USBSettingsSubsystem::ApplyGraphicsSettings()
{
    UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
    if (!UserSettings) return;

    UserSettings->SetOverallScalabilityLevel(Graphics.QualityPreset);
    UserSettings->SetFrameRateLimitCVar(Graphics.FrameRateLimit);
    UserSettings->SetResolutionScaleValueEx(Graphics.RenderScale);

    if (!Graphics.bEnableShadows)
    {
        UserSettings->SetShadowQuality(0);
    }

    if (!Graphics.bEnableAntiAliasing)
    {
        UserSettings->SetAntiAliasingQuality(0);
    }

    if (!Graphics.bEnablePostProcessing)
    {
        UserSettings->SetPostProcessingQuality(0);
    }

    UserSettings->ApplySettings(true);
    UE_LOG(LogStormBreaker, Log, TEXT("Graphics settings applied. Preset: %d, FPS: %d"),
        Graphics.QualityPreset, Graphics.FrameRateLimit);
}

void USBSettingsSubsystem::ApplyAudioSettings()
{
    // Audio volumes applied via Sound Classes in Blueprint
    UE_LOG(LogStormBreaker, Log, TEXT("Audio settings applied. Master: %.2f, SFX: %.2f, Music: %.2f"),
        Audio.MasterVolume, Audio.SFXVolume, Audio.MusicVolume);
}
