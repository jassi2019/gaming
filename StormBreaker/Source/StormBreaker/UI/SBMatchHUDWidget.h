// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SBMatchHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;
class ASBCharacterBase;

/**
 * In-game match HUD — shows Health, Shield, Ammo, Alive players.
 * Auto-created by PlayerController, updates every frame.
 * Works without Blueprint setup — creates UI elements in C++.
 */
UCLASS()
class STORMBREAKER_API USBMatchHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    void UpdateHUD();

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> HealthText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ShieldText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> AmmoText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> AliveText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> ShieldBar;
};
