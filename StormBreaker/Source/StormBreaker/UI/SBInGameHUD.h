// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Weapon/SBWeaponTypes.h"
#include "Inventory/SBInventoryTypes.h"
#include "SBInGameHUD.generated.h"

class ASBCharacterBase;
class ASBWeaponBase;
class ASBBattleRoyaleGameState;
class ASBZoneManager;

USTRUCT()
struct FSBKillFeedEntry
{
    GENERATED_BODY()

    FString KillerName;
    FString VictimName;
    FString WeaponName;
    float TimeStamp = 0.0f;
    bool bIsLocalKill = false;
};

USTRUCT()
struct FSBDamageIndicator
{
    GENERATED_BODY()

    FVector Direction = FVector::ZeroVector;
    float TimeRemaining = 0.0f;
};

/**
 * Full PUBG/BGMI-style in-game HUD drawn via Canvas.
 * No Widget Blueprint needed — fully self-contained.
 *
 * Shows: Health/Shield/Boost bars, ammo, weapon, crosshair, compass,
 * minimap, zone timer, alive count, team panel, kill feed, hit markers,
 * damage indicators, FPS, ping.
 */
UCLASS()
class STORMBREAKER_API ASBInGameHUD : public AHUD
{
    GENERATED_BODY()

public:
    ASBInGameHUD();

    virtual void DrawHUD() override;
    virtual void Tick(float DeltaTime) override;

    // --- Kill Feed ---
    void AddKillFeedEntry(const FString& Killer, const FString& Victim, const FString& Weapon, bool bLocalKill);

    // --- Damage Indicators ---
    void AddDamageIndicator(const FVector& DamageDirection);

    // --- Hit Marker ---
    void ShowHitMarker(bool bIsHeadshot);

    // --- Loot Popup ---
    void ShowLootPopup(const FString& ItemName, int32 Count);

private:
    // --- Draw Sections ---
    void DrawHealthShieldBoost();
    void DrawWeaponAmmo();
    void DrawCrosshair();
    void DrawCompass();
    void DrawMinimap();
    void DrawZoneTimer();
    void DrawAliveCount();
    void DrawTeamPanel();
    void DrawKillFeed();
    void DrawDamageIndicators();
    void DrawHitMarkerOverlay();
    void DrawFPSPing();
    void DrawLootPopups();

    // --- Helpers ---
    void DrawBar(float X, float Y, float W, float H, float Percent, FLinearColor FillColor, FLinearColor BgColor);
    void DrawTextCentered(UFont* Font, const FString& Text, float X, float Y, FColor Color);
    ASBCharacterBase* GetPlayerCharacter() const;

    // --- State ---
    TArray<FSBKillFeedEntry> KillFeed;
    TArray<FSBDamageIndicator> DamageIndicators;

    float HitMarkerTimer;
    bool bHitMarkerHeadshot;

    struct FLootPopup
    {
        FString ItemName;
        int32 Count;
        float Timer;
    };
    TArray<FLootPopup> LootPopups;

    float FPSUpdateTimer;
    float CachedFPS;
};
