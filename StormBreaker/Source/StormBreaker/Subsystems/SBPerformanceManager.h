// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SBPerformanceManager.generated.h"

UENUM(BlueprintType)
enum class ESBQualityTier : uint8
{
    Low,
    Medium,
    High,
    Ultra,
    Auto
};

UENUM(BlueprintType)
enum class ESBFPSTarget : uint8
{
    FPS_30,
    FPS_60,
    FPS_90,
    FPS_120,
    Unlimited
};

/**
 * Runtime performance manager.
 * Handles dynamic resolution, quality tier switching, tick budgeting,
 * animation LOD, AI update throttling, and GC tuning.
 * Auto-adapts on mobile to maintain target FPS.
 */
UCLASS()
class STORMBREAKER_API USBPerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // --- Quality ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Performance")
    void SetQualityTier(ESBQualityTier Tier);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Performance")
    void SetFPSTarget(ESBFPSTarget Target);

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Performance")
    ESBQualityTier GetCurrentTier() const { return CurrentTier; }

    // --- Dynamic Resolution ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Performance")
    void SetDynamicResolution(bool bEnabled, float MinScale = 0.6f, float MaxScale = 1.0f);

    // --- Tick Budgeting ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Performance")
    void SetAITickBudgetMs(float BudgetMs);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Performance")
    void SetAnimationBudgetMs(float BudgetMs);

    // --- Memory ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Performance")
    void SetTexturePoolSizeMB(int32 SizeMB);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Performance")
    void TriggerStreamingFlush();

    // --- Stats ---

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Performance")
    float GetCurrentFPS() const { return CachedFPS; }

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Performance")
    float GetCurrentResolutionScale() const;

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Performance")
    int32 GetUsedMemoryMB() const;

    // --- Auto Adapt ---

    void TickAutoAdapt(float DeltaTime);

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Performance")
    bool bAutoAdaptQuality = true;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Performance")
    float AdaptCheckInterval = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Performance")
    float DowngradeThresholdFPS = 28.0f;

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Performance")
    float UpgradeThresholdFPS = 55.0f;

private:
    void ApplyQualitySettings(ESBQualityTier Tier);
    void ApplyMobileDefaults();

    ESBQualityTier CurrentTier;
    ESBFPSTarget CurrentFPSTarget;
    float CachedFPS;
    float AdaptTimer;
};
