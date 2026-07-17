// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SBPlayerController.generated.h"

class UInputMappingContext;
class USBMobileTouchWidget;

/**
 * Player controller — owns input mapping, HUD, mobile touch widget, and client-side RPC bridge.
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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IslandOfDeath|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IslandOfDeath|Input")
    int32 DefaultMappingPriority = 0;

    // ----- Mobile -----

    UPROPERTY(EditDefaultsOnly, Category = "IslandOfDeath|Mobile")
    TSubclassOf<USBMobileTouchWidget> MobileTouchWidgetClass;

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Mobile")
    USBMobileTouchWidget* GetMobileTouchWidget() const { return MobileTouchWidget; }

    // ----- HUD -----

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|UI")
    void ShowMatchHUD();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|UI")
    void ShowInventoryUI();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|UI")
    void HideInventoryUI();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|UI")
    void ShowMapUI();

    // ----- Spectating -----

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Spectator")
    void StartSpectating();

    UFUNCTION(Server, Reliable)
    void Server_RequestRespawn();

public:
    UFUNCTION(Client, Reliable)
    void Client_OnMatchEnd(bool bIsWinner);

    UFUNCTION(Client, Reliable)
    void Client_ShowKillFeed(const FString& KillerName, const FString& VictimName, const FString& WeaponName);

private:
    void BindInputMappingContext();
    void CreateMobileTouchWidget();

    UPROPERTY()
    TObjectPtr<USBMobileTouchWidget> MobileTouchWidget;
};
