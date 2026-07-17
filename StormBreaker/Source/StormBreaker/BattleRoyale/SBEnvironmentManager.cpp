// Copyright Island Of Death Games. All Rights Reserved.

#include "BattleRoyale/SBEnvironmentManager.h"
#include "StormBreaker.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Net/UnrealNetwork.h"

ASBEnvironmentManager::ASBEnvironmentManager()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;

    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    RootComponent = SunLight;
    SunLight->SetIntensity(5.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.85f));

    FogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("Fog"));
    FogComponent->SetupAttachment(RootComponent);
    FogComponent->SetFogDensity(0.005f);
    FogComponent->SetFogMaxOpacity(0.5f);

    CurrentTime = 0.35f;
    WeatherState = ESBWeatherState::Clear;
    WeatherTimer = 0.0f;
}

void ASBEnvironmentManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASBEnvironmentManager, CurrentTime);
    DOREPLIFETIME(ASBEnvironmentManager, WeatherState);
}

void ASBEnvironmentManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HasAuthority())
    {
        // Advance time
        if (bCycleEnabled && DayLengthMinutes > 0.0f)
        {
            float TimeAdvance = DeltaTime / (DayLengthMinutes * 60.0f);
            CurrentTime = FMath::Fmod(CurrentTime + TimeAdvance, 1.0f);
            OnTimeOfDayChanged.Broadcast(CurrentTime);
        }

        TickWeatherTimer(DeltaTime);
    }

    UpdateSun();
    UpdateFog();
}

// ============================================================================
// Time
// ============================================================================

void ASBEnvironmentManager::SetTimeOfDay(float NormalizedTime)
{
    CurrentTime = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    UpdateSun();
}

void ASBEnvironmentManager::UpdateSun()
{
    // 0.0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset
    float SunPitch = (CurrentTime - 0.25f) * 360.0f;
    SunLight->SetWorldRotation(FRotator(SunPitch, -45.0f, 0.0f));

    // Adjust intensity based on time
    float DotUp = FMath::Max(0.0f, FMath::Cos(FMath::DegreesToRadians(SunPitch)));
    SunLight->SetIntensity(FMath::Lerp(0.2f, 8.0f, DotUp));

    // Color shift: warm at sunrise/sunset, white at noon
    float WarmFactor = 1.0f - DotUp;
    FLinearColor SunColor = FMath::Lerp(
        FLinearColor(1.0f, 0.98f, 0.92f),
        FLinearColor(1.0f, 0.6f, 0.3f),
        WarmFactor * 0.5f);
    SunLight->SetLightColor(SunColor);
}

// ============================================================================
// Weather
// ============================================================================

void ASBEnvironmentManager::SetWeather(ESBWeatherState NewWeather)
{
    if (WeatherState == NewWeather) return;
    WeatherState = NewWeather;
    OnWeatherChanged.Broadcast(NewWeather);
    UpdateFog();
}

void ASBEnvironmentManager::OnRep_Weather()
{
    OnWeatherChanged.Broadcast(WeatherState);
    UpdateFog();
}

void ASBEnvironmentManager::UpdateFog()
{
    if (!FogComponent) return;

    switch (WeatherState)
    {
    case ESBWeatherState::Clear:
        FogComponent->SetFogDensity(0.002f);
        FogComponent->SetFogMaxOpacity(0.3f);
        break;
    case ESBWeatherState::Cloudy:
        FogComponent->SetFogDensity(0.005f);
        FogComponent->SetFogMaxOpacity(0.5f);
        break;
    case ESBWeatherState::Rain:
        FogComponent->SetFogDensity(0.01f);
        FogComponent->SetFogMaxOpacity(0.6f);
        break;
    case ESBWeatherState::HeavyRain:
        FogComponent->SetFogDensity(0.02f);
        FogComponent->SetFogMaxOpacity(0.75f);
        break;
    case ESBWeatherState::Fog:
        FogComponent->SetFogDensity(0.05f);
        FogComponent->SetFogMaxOpacity(0.9f);
        break;
    }
}

float ASBEnvironmentManager::GetWindStrength() const
{
    switch (WeatherState)
    {
    case ESBWeatherState::Clear:     return 0.0f;
    case ESBWeatherState::Cloudy:    return 100.0f;
    case ESBWeatherState::Rain:      return 250.0f;
    case ESBWeatherState::HeavyRain: return 500.0f;
    case ESBWeatherState::Fog:       return 50.0f;
    default: return 0.0f;
    }
}

void ASBEnvironmentManager::TickWeatherTimer(float DeltaTime)
{
    WeatherTimer += DeltaTime;

    if (WeatherTimer >= WeatherChangeIntervalMin)
    {
        WeatherTimer = 0.0f;

        // Random weather transition
        int32 Roll = FMath::RandRange(0, 100);
        if (Roll < 50)      SetWeather(ESBWeatherState::Clear);
        else if (Roll < 70) SetWeather(ESBWeatherState::Cloudy);
        else if (Roll < 85) SetWeather(ESBWeatherState::Rain);
        else if (Roll < 95) SetWeather(ESBWeatherState::Fog);
        else                SetWeather(ESBWeatherState::HeavyRain);
    }
}
