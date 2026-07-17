// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SBSettingsSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FSBGraphicsSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 QualityPreset = 2; // 0=Low, 1=Med, 2=High, 3=Ultra

    UPROPERTY(BlueprintReadWrite)
    int32 FrameRateLimit = 60;

    UPROPERTY(BlueprintReadWrite)
    float RenderScale = 100.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bEnableShadows = true;

    UPROPERTY(BlueprintReadWrite)
    bool bEnableAntiAliasing = true;

    UPROPERTY(BlueprintReadWrite)
    bool bEnablePostProcessing = true;
};

USTRUCT(BlueprintType)
struct FSBAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float MasterVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float SFXVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float MusicVolume = 0.7f;

    UPROPERTY(BlueprintReadWrite)
    float VoiceVolume = 1.0f;
};

USTRUCT(BlueprintType)
struct FSBControlSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float MouseSensitivity = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float ADSSensitivity = 0.6f;

    UPROPERTY(BlueprintReadWrite)
    bool bInvertY = false;

    UPROPERTY(BlueprintReadWrite)
    float GyroscopeSensitivity = 0.0f;
};

/**
 * Persists player settings across sessions via SaveGame.
 */
UCLASS()
class STORMBREAKER_API USBSettingsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Settings")
    void SaveSettings();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Settings")
    void LoadSettings();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Settings")
    void ApplyGraphicsSettings();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Settings")
    void ApplyAudioSettings();

    UPROPERTY(BlueprintReadWrite, Category = "IslandOfDeath|Settings")
    FSBGraphicsSettings Graphics;

    UPROPERTY(BlueprintReadWrite, Category = "IslandOfDeath|Settings")
    FSBAudioSettings Audio;

    UPROPERTY(BlueprintReadWrite, Category = "IslandOfDeath|Settings")
    FSBControlSettings Controls;

private:
    static const FString SaveSlotName;
};
