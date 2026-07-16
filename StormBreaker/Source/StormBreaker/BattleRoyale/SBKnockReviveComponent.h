// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/SBInventoryTypes.h"
#include "SBKnockReviveComponent.generated.h"

class ASBCharacterBase;
class ASBDeathCrate;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLifeStateChanged, ESBPlayerLifeState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReviveProgress, float, Progress, AActor*, Reviver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathCrateSpawned, ASBDeathCrate*, Crate);

/**
 * Manages the knock/bleed-out/revive/death flow for battle royale.
 *
 * Alive → Knocked → (revive timer / bleed out) → Dead → Spectator
 * When knocked: movement severely restricted, can crawl.
 * Teammates can revive within the bleed-out window.
 * On death: spawns death crate with all inventory.
 */
UCLASS(ClassGroup = "StormBreaker", meta = (BlueprintSpawnableComponent))
class STORMBREAKER_API USBKnockReviveComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USBKnockReviveComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- State ---

    UFUNCTION(BlueprintPure, Category = "StormBreaker|LifeState")
    ESBPlayerLifeState GetLifeState() const { return LifeState; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|LifeState")
    bool IsAlive() const { return LifeState == ESBPlayerLifeState::Alive; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|LifeState")
    bool IsKnocked() const { return LifeState == ESBPlayerLifeState::Knocked; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|LifeState")
    bool IsDead() const { return LifeState == ESBPlayerLifeState::Dead; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|LifeState")
    float GetBleedOutTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category = "StormBreaker|LifeState")
    float GetReviveProgress() const;

    // --- Actions ---

    void HandleHealthReachedZero(AController* Killer);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|LifeState")
    void StartRevive(ASBCharacterBase* Reviver);

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|LifeState")
    void CancelRevive();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|LifeState")
    void ForceKill(AController* Killer);

    void EnterSpectatorMode();

    // --- Config ---

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|LifeState")
    float BleedOutDuration = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|LifeState")
    float ReviveDuration = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|LifeState")
    float KnockedMoveSpeed = 50.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|LifeState")
    float HealthAfterRevive = 30.0f;

    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|LifeState")
    TSubclassOf<ASBDeathCrate> DeathCrateClass;

    // If false, knocked players die instantly (solo mode)
    UPROPERTY(EditDefaultsOnly, Category = "StormBreaker|LifeState")
    bool bEnableKnockSystem = true;

    // --- Delegates ---

    UPROPERTY(BlueprintAssignable)
    FOnLifeStateChanged OnLifeStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnReviveProgress OnReviveProgress;

    UPROPERTY(BlueprintAssignable)
    FOnDeathCrateSpawned OnDeathCrateSpawned;

protected:
    UFUNCTION(Server, Reliable)
    void Server_StartRevive(ASBCharacterBase* Reviver);

    UFUNCTION(Server, Reliable)
    void Server_CancelRevive();

private:
    void SetLifeState(ESBPlayerLifeState NewState);
    void EnterKnockedState();
    void Die(AController* Killer);
    void FinishRevive();
    void SpawnDeathCrate();
    void TickBleedOut(float DeltaTime);
    void TickRevive(float DeltaTime);

    UPROPERTY(ReplicatedUsing = OnRep_LifeState)
    ESBPlayerLifeState LifeState;

    UFUNCTION()
    void OnRep_LifeState();

    UPROPERTY(Replicated)
    float BleedOutElapsed;

    UPROPERTY(Replicated)
    float ReviveElapsed;

    UPROPERTY()
    TObjectPtr<ASBCharacterBase> ActiveReviver;

    UPROPERTY()
    TObjectPtr<AController> LastKiller;
};
