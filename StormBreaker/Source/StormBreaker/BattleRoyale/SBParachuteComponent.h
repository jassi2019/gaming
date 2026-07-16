// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SBParachuteComponent.generated.h"

class UStaticMeshComponent;
class ASBCharacterBase;

UENUM(BlueprintType)
enum class ESBParachuteState : uint8
{
    None,
    FreeFall,
    Deployed,
    Landed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParachuteStateChanged, ESBParachuteState, NewState);

/**
 * Handles free-fall and parachute deployment after jumping from aircraft.
 * Provides steering, auto-deploy at threshold altitude, landing detection,
 * and fall damage prevention.
 */
UCLASS(ClassGroup = "StormBreaker", meta = (BlueprintSpawnableComponent))
class STORMBREAKER_API USBParachuteComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USBParachuteComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Control ---

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Parachute")
    void BeginFreeFall();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Parachute")
    void DeployParachute();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Parachute")
    void SetSteeringInput(FVector2D Input);

    // --- Queries ---

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Parachute")
    ESBParachuteState GetState() const { return ParachuteState; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Parachute")
    bool IsParachuteActive() const { return ParachuteState == ESBParachuteState::FreeFall || ParachuteState == ESBParachuteState::Deployed; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Parachute")
    float GetAltitudeAboveGround() const;

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Parachute")
    float FreeFallSpeed = 5000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Parachute")
    float FreeFallForwardSpeed = 3000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Parachute")
    float ParachuteDescendSpeed = 800.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Parachute")
    float ParachuteForwardSpeed = 1500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Parachute")
    float SteeringSpeed = 120.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Parachute")
    float AutoDeployAltitude = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Parachute")
    float LandingAltitude = 50.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Parachute")
    float WindStrength = 200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|Parachute")
    TObjectPtr<UStaticMesh> ParachuteMesh;

    // --- Delegates ---

    UPROPERTY(BlueprintAssignable)
    FOnParachuteStateChanged OnParachuteStateChanged;

protected:
    UFUNCTION(Server, Reliable)
    void Server_DeployParachute();

private:
    void TickFreeFall(float DeltaTime);
    void TickDeployed(float DeltaTime);
    void Land();
    void SetState(ESBParachuteState NewState);
    float TraceAltitude() const;

    UPROPERTY(ReplicatedUsing = OnRep_ParachuteState)
    ESBParachuteState ParachuteState;

    UFUNCTION()
    void OnRep_ParachuteState();

    FVector2D CurrentSteeringInput;
    FVector WindOffset;

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> ParachuteMeshComp;
};
