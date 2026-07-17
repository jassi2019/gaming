// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SBDebugHUD.generated.h"

/**
 * Debug HUD drawn directly to canvas — no Widget Blueprint needed.
 * Shows: Health, Shield, Ammo, Alive Players, Weapon Name, Stance.
 * Assigned as HUDClass on the GameMode for instant play.
 */
UCLASS()
class STORMBREAKER_API ASBHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    void DrawHealthBar(float X, float Y, float Width, float Height, float Percent, FLinearColor Color, const FString& Label);
};
