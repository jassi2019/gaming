// Copyright StormBreaker Games. All Rights Reserved.

#include "Subsystems/SBPerformanceManager.h"
#include "StormBreaker.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/PlatformMisc.h"

void USBPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentTier = ESBQualityTier::High;
    CurrentFPSTarget = ESBFPSTarget::FPS_60;
    CachedFPS = 60.0f;
    AdaptTimer = 0.0f;

#if PLATFORM_ANDROID || PLATFORM_IOS
    ApplyMobileDefaults();
#endif

    UE_LOG(LogStormBreaker, Log, TEXT("Performance Manager initialized. Tier: %d"), (int32)CurrentTier);
}

// ============================================================================
// Quality
// ============================================================================

void USBPerformanceManager::SetQualityTier(ESBQualityTier Tier)
{
    if (Tier == ESBQualityTier::Auto)
    {
        bAutoAdaptQuality = true;
        return;
    }

    bAutoAdaptQuality = false;
    CurrentTier = Tier;
    ApplyQualitySettings(Tier);
}

void USBPerformanceManager::ApplyQualitySettings(ESBQualityTier Tier)
{
    CurrentTier = Tier;

    UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
    if (!Settings) return;

    int32 ScalabilityLevel = 0;
    switch (Tier)
    {
    case ESBQualityTier::Low:    ScalabilityLevel = 0; break;
    case ESBQualityTier::Medium: ScalabilityLevel = 1; break;
    case ESBQualityTier::High:   ScalabilityLevel = 2; break;
    case ESBQualityTier::Ultra:  ScalabilityLevel = 3; break;
    default: break;
    }

    Settings->SetOverallScalabilityLevel(ScalabilityLevel);
    Settings->ApplySettings(true);

    // Distance culling
    IConsoleVariable* CVarDistCull = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale"));
    if (CVarDistCull)
    {
        float DistScale[] = { 0.6f, 0.8f, 1.0f, 1.2f };
        CVarDistCull->Set(DistScale[ScalabilityLevel]);
    }

    // Shadow quality
    IConsoleVariable* CVarShadows = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ShadowQuality"));
    if (CVarShadows)
    {
        int32 ShadowQuality[] = { 0, 2, 3, 5 };
        CVarShadows->Set(ShadowQuality[ScalabilityLevel]);
    }

    // Niagara particle scalability
    IConsoleVariable* CVarNiagara = IConsoleManager::Get().FindConsoleVariable(TEXT("fx.Niagara.QualityLevel"));
    if (CVarNiagara)
    {
        CVarNiagara->Set(ScalabilityLevel);
    }

    // Foliage density
    IConsoleVariable* CVarFoliage = IConsoleManager::Get().FindConsoleVariable(TEXT("foliage.DensityScale"));
    if (CVarFoliage)
    {
        float FoliageDensity[] = { 0.4f, 0.6f, 0.8f, 1.0f };
        CVarFoliage->Set(FoliageDensity[ScalabilityLevel]);
    }

    UE_LOG(LogStormBreaker, Log, TEXT("Quality tier set to %d (scalability %d)"), (int32)Tier, ScalabilityLevel);
}

void USBPerformanceManager::SetFPSTarget(ESBFPSTarget Target)
{
    CurrentFPSTarget = Target;

    float Limit = 0.0f;
    switch (Target)
    {
    case ESBFPSTarget::FPS_30:  Limit = 30.0f;  break;
    case ESBFPSTarget::FPS_60:  Limit = 60.0f;  break;
    case ESBFPSTarget::FPS_90:  Limit = 90.0f;  break;
    case ESBFPSTarget::FPS_120: Limit = 120.0f; break;
    case ESBFPSTarget::Unlimited: Limit = 0.0f; break;
    }

    IConsoleVariable* CVarMaxFPS = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
    if (CVarMaxFPS)
    {
        CVarMaxFPS->Set(Limit);
    }

    UE_LOG(LogStormBreaker, Log, TEXT("FPS target: %.0f"), Limit);
}

// ============================================================================
// Dynamic Resolution
// ============================================================================

void USBPerformanceManager::SetDynamicResolution(bool bEnabled, float MinScale, float MaxScale)
{
    IConsoleVariable* CVarDynRes = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicRes.OperationMode"));
    if (CVarDynRes)
    {
        CVarDynRes->Set(bEnabled ? 1 : 0);
    }

    IConsoleVariable* CVarMin = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicRes.MinScreenPercentage"));
    IConsoleVariable* CVarMax = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicRes.MaxScreenPercentage"));

    if (CVarMin) CVarMin->Set(MinScale * 100.0f);
    if (CVarMax) CVarMax->Set(MaxScale * 100.0f);
}

// ============================================================================
// Tick Budgeting
// ============================================================================

void USBPerformanceManager::SetAITickBudgetMs(float BudgetMs)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ai.TickBudgetMs"));
    if (CVar) CVar->Set(BudgetMs);
}

void USBPerformanceManager::SetAnimationBudgetMs(float BudgetMs)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("a.AnimBudget.BudgetMs"));
    if (CVar) CVar->Set(BudgetMs);
}

// ============================================================================
// Memory
// ============================================================================

void USBPerformanceManager::SetTexturePoolSizeMB(int32 SizeMB)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize"));
    if (CVar) CVar->Set(SizeMB);
}

void USBPerformanceManager::TriggerStreamingFlush()
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.FullyLoadUsedTextures"));
    if (CVar) CVar->Set(0);
}

float USBPerformanceManager::GetCurrentResolutionScale() const
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
    return CVar ? CVar->GetFloat() : 100.0f;
}

int32 USBPerformanceManager::GetUsedMemoryMB() const
{
    FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
    return static_cast<int32>(Stats.UsedPhysical / (1024 * 1024));
}

// ============================================================================
// Auto Adapt
// ============================================================================

void USBPerformanceManager::TickAutoAdapt(float DeltaTime)
{
    if (!bAutoAdaptQuality) return;

    CachedFPS = (DeltaTime > 0.0f) ? 1.0f / DeltaTime : 60.0f;
    AdaptTimer += DeltaTime;

    if (AdaptTimer < AdaptCheckInterval) return;
    AdaptTimer = 0.0f;

    if (CachedFPS < DowngradeThresholdFPS && CurrentTier > ESBQualityTier::Low)
    {
        ESBQualityTier NewTier = static_cast<ESBQualityTier>(static_cast<uint8>(CurrentTier) - 1);
        ApplyQualitySettings(NewTier);
        UE_LOG(LogStormBreaker, Log, TEXT("Auto-downgraded quality to %d (FPS: %.0f)"), (int32)NewTier, CachedFPS);
    }
    else if (CachedFPS > UpgradeThresholdFPS && CurrentTier < ESBQualityTier::Ultra)
    {
        ESBQualityTier NewTier = static_cast<ESBQualityTier>(static_cast<uint8>(CurrentTier) + 1);
        ApplyQualitySettings(NewTier);
        UE_LOG(LogStormBreaker, Log, TEXT("Auto-upgraded quality to %d (FPS: %.0f)"), (int32)NewTier, CachedFPS);
    }
}

// ============================================================================
// Mobile Defaults
// ============================================================================

void USBPerformanceManager::ApplyMobileDefaults()
{
    SetFPSTarget(ESBFPSTarget::FPS_60);
    SetDynamicResolution(true, 0.6f, 1.0f);
    SetTexturePoolSizeMB(512);
    SetAITickBudgetMs(2.0f);
    SetAnimationBudgetMs(3.0f);
    bAutoAdaptQuality = true;

    // Mobile-specific CVars
    auto SetCVar = [](const TCHAR* Name, float Value)
    {
        IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(Name);
        if (CVar) CVar->Set(Value);
    };

    SetCVar(TEXT("r.MobileContentScaleFactor"), 1.0f);
    SetCVar(TEXT("r.Mobile.DisableVertexFog"), 1.0f);
    SetCVar(TEXT("r.Mobile.AllowDitheredLODTransition"), 0.0f);
    SetCVar(TEXT("r.Streaming.Boost"), 0.5f);
    SetCVar(TEXT("r.Streaming.HLODStrategy"), 2.0f);
    SetCVar(TEXT("gc.MaxObjectsNotConsideredByGC"), 200000.0f);
    SetCVar(TEXT("gc.TimeBetweenPurgingPendingKillObjects"), 60.0f);
    SetCVar(TEXT("r.ParticleLightComplexity"), 0.0f);
    SetCVar(TEXT("r.DefaultFeature.Bloom"), 0.0f);
    SetCVar(TEXT("r.DefaultFeature.AmbientOcclusion"), 0.0f);
    SetCVar(TEXT("r.DefaultFeature.MotionBlur"), 0.0f);
    SetCVar(TEXT("r.DefaultFeature.LensFlare"), 0.0f);

    ApplyQualitySettings(ESBQualityTier::Medium);

    UE_LOG(LogStormBreaker, Log, TEXT("Mobile defaults applied."));
}
