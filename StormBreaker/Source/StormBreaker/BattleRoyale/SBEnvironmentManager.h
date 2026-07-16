// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBEnvironmentManager.generated.h"

class UDirectionalLightComponent;
class USkyAtmosphereComponent;
class UExponentialHeightFogComponent;

UENUM(BlueprintType)
enum class ESBWeatherState : uint8
{
    Clear,
    Cloudy,
    Rain,
    HeavyRain,
    Fog
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, ESBWeatherState, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, float, NormalizedTime);

/**
 * Controls day/night cycle and dynamic weather.
 * Rotates sun directional light, adjusts fog, and transitions weather states.
 * Replicated to all clients.
 */
UCLASS()
class STORMBREAKER_API ASBEnvironmentManager : public AActor
{
    GENERATED_BODY()

public:
    ASBEnvironmentManager();

    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Time ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Environment")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Environment")
    float GetTimeOfDay() const { return CurrentTime; }

    UPROPERTY(EditAnywhere, Category = "StormBreaker|Environment")
    float DayLengthMinutes = 20.0f;

    UPROPERTY(EditAnywhere, Category = "StormBreaker|Environment")
    float StartTime = 0.35f;

    UPROPERTY(EditAnywhere, Category = "StormBreaker|Environment")
    bool bCycleEnabled = true;

    // --- Weather ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Environment")
    void SetWeather(ESBWeatherState NewWeather);

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Environment")
    ESBWeatherState GetWeather() const { return WeatherState; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Environment")
    float GetWindStrength() const;

    UPROPERTY(EditAnywhere, Category = "StormBreaker|Environment")
    float WeatherChangeIntervalMin = 300.0f;

    // --- Components ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UDirectionalLightComponent> SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UExponentialHeightFogComponent> FogComponent;

    // --- Delegates ---

    UPROPERTY(BlueprintAssignable)
    FOnWeatherChanged OnWeatherChanged;

    UPROPERTY(BlueprintAssignable)
    FOnTimeOfDayChanged OnTimeOfDayChanged;

protected:
    UPROPERTY(Replicated)
    float CurrentTime;

    UPROPERTY(ReplicatedUsing = OnRep_Weather)
    ESBWeatherState WeatherState;

    UFUNCTION()
    void OnRep_Weather();

private:
    void UpdateSun();
    void UpdateFog();
    void TickWeatherTimer(float DeltaTime);

    float WeatherTimer;
};
