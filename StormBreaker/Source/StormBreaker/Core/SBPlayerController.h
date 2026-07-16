// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SBPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * Player controller — owns input mapping, HUD, and client-side RPC bridge.
 * Enhanced Input System is bound here.
 */
UCLASS()
class STORMBREAKER_API ASBPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ASBPlayerController();

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void SetupInputComponent() override;

    // ----- Input Contexts -----

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBreaker|Input")
    int32 DefaultMappingPriority = 0;

    // ----- HUD -----

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|UI")
    void ShowMatchHUD();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|UI")
    void ShowInventoryUI();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|UI")
    void HideInventoryUI();

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|UI")
    void ShowMapUI();

    // ----- Spectating -----

    UFUNCTION(BlueprintCallable, Category = "StormBreaker|Spectator")
    void StartSpectating();

    UFUNCTION(Server, Reliable)
    void Server_RequestRespawn();

protected:
    UFUNCTION(Client, Reliable)
    void Client_OnMatchEnd(bool bIsWinner);

    UFUNCTION(Client, Reliable)
    void Client_ShowKillFeed(const FString& KillerName, const FString& VictimName, const FString& WeaponName);

private:
    void BindInputMappingContext();
};
