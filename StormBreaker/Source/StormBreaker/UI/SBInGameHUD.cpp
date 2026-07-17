// Copyright Island Of Death Games. All Rights Reserved.

#include "UI/SBInGameHUD.h"
#include "Character/SBCharacterBase.h"
#include "Character/SBCharacterMovementComponent.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponBase.h"
#include "Weapon/SBWeaponDataAsset.h"
#include "Core/SBPlayerState.h"
#include "Core/SBAttributeSet.h"
#include "Core/SBBattleRoyaleGameState.h"
#include "Inventory/SBInventoryComponent.h"
#include "BattleRoyale/SBZoneManager.h"
#include "StormBreaker.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

ASBInGameHUD::ASBInGameHUD()
{
    HitMarkerTimer = 0.0f;
    bHitMarkerHeadshot = false;
    FPSUpdateTimer = 0.0f;
    CachedFPS = 60.0f;
}

void ASBInGameHUD::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Decay hit marker
    if (HitMarkerTimer > 0.0f)
    {
        HitMarkerTimer -= DeltaTime;
    }

    // Decay damage indicators
    for (int32 i = DamageIndicators.Num() - 1; i >= 0; i--)
    {
        DamageIndicators[i].TimeRemaining -= DeltaTime;
        if (DamageIndicators[i].TimeRemaining <= 0.0f)
        {
            DamageIndicators.RemoveAt(i);
        }
    }

    // Decay kill feed
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (int32 i = KillFeed.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - KillFeed[i].TimeStamp > 5.0f)
        {
            KillFeed.RemoveAt(i);
        }
    }

    // Decay loot popups
    for (int32 i = LootPopups.Num() - 1; i >= 0; i--)
    {
        LootPopups[i].Timer -= DeltaTime;
        if (LootPopups[i].Timer <= 0.0f)
        {
            LootPopups.RemoveAt(i);
        }
    }

    // FPS counter
    FPSUpdateTimer += DeltaTime;
    if (FPSUpdateTimer >= 0.5f)
    {
        FPSUpdateTimer = 0.0f;
        CachedFPS = 1.0f / FMath::Max(DeltaTime, 0.001f);
    }
}

void ASBInGameHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    DrawHealthShieldBoost();
    DrawWeaponAmmo();
    DrawCrosshair();
    DrawCompass();
    DrawMinimap();
    DrawZoneTimer();
    DrawAliveCount();
    DrawKillFeed();
    DrawDamageIndicators();
    DrawHitMarkerOverlay();
    DrawFPSPing();
    DrawLootPopups();
}

// ============================================================================
// Health / Shield / Boost
// ============================================================================

void ASBInGameHUD::DrawHealthShieldBoost()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;
    const float BarW = SW * 0.22f;
    const float BarH = 16.0f;
    const float X = 20.0f;
    float Y = SH - 30.0f;

    float Health = 100.0f, MaxHealth = 100.0f;
    float Shield = 0.0f, MaxShield = 150.0f;
    float Boost = 0.0f;

    ASBCharacterBase* Char = GetPlayerCharacter();
    if (Char)
    {
        ASBPlayerState* PS = Char->GetPlayerState<ASBPlayerState>();
        if (PS && PS->AttributeSet)
        {
            Health = PS->AttributeSet->GetHealth();
            MaxHealth = PS->AttributeSet->GetMaxHealth();
            Shield = PS->AttributeSet->GetShield();
            MaxShield = PS->AttributeSet->GetMaxShield();
        }

        USBInventoryComponent* Inv = Char->InventoryComponent;
        if (Inv)
        {
            Boost = Inv->GetBoostState().BoostLevel;
        }
    }

    // Health bar
    DrawBar(X, Y, BarW, BarH, MaxHealth > 0 ? Health / MaxHealth : 0,
        FLinearColor(0.15f, 0.85f, 0.25f, 0.9f), FLinearColor(0.1f, 0.1f, 0.1f, 0.7f));
    UFont* Font = GEngine->GetTinyFont();
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), Health), X + 4, Y + 1);

    Y -= BarH + 4;

    // Shield bar
    DrawBar(X, Y, BarW, BarH, MaxShield > 0 ? Shield / MaxShield : 0,
        FLinearColor(0.2f, 0.5f, 1.0f, 0.9f), FLinearColor(0.1f, 0.1f, 0.1f, 0.7f));
    Canvas->SetDrawColor(FColor(100, 180, 255));
    Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), Shield), X + 4, Y + 1);

    Y -= BarH + 4;

    // Boost bar (segmented: 4 segments at 25% each)
    float BoostPercent = Boost / 100.0f;
    float SegW = BarW / 4.0f;
    for (int32 i = 0; i < 4; i++)
    {
        float SegPercent = FMath::Clamp((BoostPercent - i * 0.25f) / 0.25f, 0.0f, 1.0f);
        FLinearColor BoostColor = (i < 2) ? FLinearColor(1.0f, 0.6f, 0.1f, 0.9f) : FLinearColor(1.0f, 0.3f, 0.1f, 0.9f);
        DrawBar(X + i * (SegW + 2), Y, SegW - 2, BarH * 0.6f, SegPercent, BoostColor,
            FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));
    }
}

// ============================================================================
// Weapon / Ammo
// ============================================================================

void ASBInGameHUD::DrawWeaponAmmo()
{
    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;
    const float X = SW - 20.0f;
    float Y = SH - 30.0f;

    UFont* SmallFont = GEngine->GetSmallFont();
    UFont* TinyFont = GEngine->GetTinyFont();

    FString AmmoStr = TEXT("--/--");
    FString WeaponName = TEXT("");
    FString FireMode = TEXT("");

    ASBCharacterBase* Char = GetPlayerCharacter();
    if (Char && Char->WeaponComponent)
    {
        ASBWeaponBase* Weapon = Char->WeaponComponent->GetActiveWeapon();
        if (Weapon)
        {
            AmmoStr = FString::Printf(TEXT("%d / %d"), Weapon->GetCurrentMagazine(), Weapon->GetCurrentReserve());
            if (Weapon->WeaponData) WeaponName = Weapon->WeaponData->DisplayName.ToString();
            switch (Weapon->GetCurrentFireMode())
            {
            case ESBFireMode::Single: FireMode = TEXT("SINGLE"); break;
            case ESBFireMode::Burst:  FireMode = TEXT("BURST");  break;
            case ESBFireMode::Auto:   FireMode = TEXT("AUTO");   break;
            }
        }
    }

    float TW, TH;

    // Ammo (large)
    Canvas->SetDrawColor(FColor::White);
    Canvas->TextSize(SmallFont, AmmoStr, TW, TH);
    Canvas->DrawText(SmallFont, AmmoStr, X - TW, Y - TH);
    Y -= TH + 4;

    // Fire mode
    Canvas->SetDrawColor(FColor(200, 200, 100));
    Canvas->TextSize(TinyFont, FireMode, TW, TH);
    Canvas->DrawText(TinyFont, FireMode, X - TW, Y - TH);
    Y -= TH + 2;

    // Weapon name
    Canvas->SetDrawColor(FColor(200, 200, 200));
    Canvas->TextSize(TinyFont, WeaponName, TW, TH);
    Canvas->DrawText(TinyFont, WeaponName, X - TW, Y - TH);
}

// ============================================================================
// Crosshair
// ============================================================================

void ASBInGameHUD::DrawCrosshair()
{
    const float CX = Canvas->SizeX * 0.5f;
    const float CY = Canvas->SizeY * 0.5f;
    const float Size = 8.0f;
    const float Gap = 4.0f;
    const float Thick = 2.0f;

    FLinearColor CrosshairColor(1.0f, 1.0f, 1.0f, 0.8f);

    // Four lines with gap
    DrawRect(CrosshairColor, CX - Size - Gap, CY - Thick * 0.5f, Size, Thick);
    DrawRect(CrosshairColor, CX + Gap, CY - Thick * 0.5f, Size, Thick);
    DrawRect(CrosshairColor, CX - Thick * 0.5f, CY - Size - Gap, Thick, Size);
    DrawRect(CrosshairColor, CX - Thick * 0.5f, CY + Gap, Thick, Size);

    // Center dot
    DrawRect(CrosshairColor, CX - 1, CY - 1, 2, 2);
}

// ============================================================================
// Compass
// ============================================================================

void ASBInGameHUD::DrawCompass()
{
    const float CX = Canvas->SizeX * 0.5f;
    const float Y = 30.0f;
    const float Width = Canvas->SizeX * 0.35f;
    const float HalfW = Width * 0.5f;

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    float Yaw = PC->GetControlRotation().Yaw;
    if (Yaw < 0) Yaw += 360.0f;

    // Background strip
    DrawRect(FLinearColor(0, 0, 0, 0.4f), CX - HalfW, Y - 2, Width, 20);

    UFont* Font = GEngine->GetTinyFont();

    // Draw cardinal directions
    static const TArray<TPair<float, FString>> Cardinals = {
        {0.0f, TEXT("N")}, {45.0f, TEXT("NE")}, {90.0f, TEXT("E")}, {135.0f, TEXT("SE")},
        {180.0f, TEXT("S")}, {225.0f, TEXT("SW")}, {270.0f, TEXT("W")}, {315.0f, TEXT("NW")}
    };

    for (const auto& Dir : Cardinals)
    {
        float DeltaAngle = FMath::FindDeltaAngleDegrees(Yaw, Dir.Key);
        float ScreenX = CX + (DeltaAngle / 180.0f) * HalfW;

        if (ScreenX > CX - HalfW && ScreenX < CX + HalfW)
        {
            bool bCardinal = (Dir.Value.Len() == 1);
            Canvas->SetDrawColor(bCardinal ? FColor::White : FColor(150, 150, 150));
            float TW, TH;
            Canvas->TextSize(Font, Dir.Value, TW, TH);
            Canvas->DrawText(Font, Dir.Value, ScreenX - TW * 0.5f, Y);
        }
    }

    // Bearing number
    Canvas->SetDrawColor(FColor::White);
    FString BearingStr = FString::Printf(TEXT("%.0f"), Yaw);
    float TW, TH;
    Canvas->TextSize(Font, BearingStr, TW, TH);
    Canvas->DrawText(Font, BearingStr, CX - TW * 0.5f, Y + 14);

    // Center marker
    DrawRect(FLinearColor::White, CX - 1, Y - 4, 2, 6);
}

// ============================================================================
// Minimap (simple dot representation)
// ============================================================================

void ASBInGameHUD::DrawMinimap()
{
    const float MapSize = FMath::Min(Canvas->SizeX, Canvas->SizeY) * 0.18f;
    const float X = Canvas->SizeX - MapSize - 15.0f;
    const float Y = 55.0f;
    const float CX = X + MapSize * 0.5f;
    const float CY = Y + MapSize * 0.5f;
    const float MapScale = 0.01f;

    // Background circle
    DrawRect(FLinearColor(0, 0, 0, 0.5f), X, Y, MapSize, MapSize);

    ASBCharacterBase* Char = GetPlayerCharacter();
    if (!Char) return;

    FVector PlayerLoc = Char->GetActorLocation();

    // Player dot (center, green)
    DrawRect(FLinearColor(0.2f, 1.0f, 0.3f), CX - 3, CY - 3, 6, 6);

    // Zone circles
    for (TActorIterator<ASBZoneManager> It(GetWorld()); It; ++It)
    {
        ASBZoneManager* Zone = *It;
        if (!Zone) continue;

        // Current zone (blue circle)
        FVector ZoneDelta = (Zone->GetCurrentCenter() - PlayerLoc) * MapScale;
        float ZoneR = Zone->GetCurrentRadius() * MapScale;
        float ZX = CX + ZoneDelta.X;
        float ZY = CY - ZoneDelta.Y;
        // Draw as a box approximation
        DrawRect(FLinearColor(0.3f, 0.5f, 1.0f, 0.3f), ZX - ZoneR, ZY - ZoneR, ZoneR * 2, 1);
        DrawRect(FLinearColor(0.3f, 0.5f, 1.0f, 0.3f), ZX - ZoneR, ZY + ZoneR, ZoneR * 2, 1);
        DrawRect(FLinearColor(0.3f, 0.5f, 1.0f, 0.3f), ZX - ZoneR, ZY - ZoneR, 1, ZoneR * 2);
        DrawRect(FLinearColor(0.3f, 0.5f, 1.0f, 0.3f), ZX + ZoneR, ZY - ZoneR, 1, ZoneR * 2);

        // Target zone (white circle)
        FVector TargetDelta = (Zone->GetTargetCenter() - PlayerLoc) * MapScale;
        float TR = Zone->GetTargetRadius() * MapScale;
        float TX = CX + TargetDelta.X;
        float TY = CY - TargetDelta.Y;
        DrawRect(FLinearColor(1, 1, 1, 0.5f), TX - TR, TY - TR, TR * 2, 1);
        DrawRect(FLinearColor(1, 1, 1, 0.5f), TX - TR, TY + TR, TR * 2, 1);
        DrawRect(FLinearColor(1, 1, 1, 0.5f), TX - TR, TY - TR, 1, TR * 2);
        DrawRect(FLinearColor(1, 1, 1, 0.5f), TX + TR, TY - TR, 1, TR * 2);
    }
}

// ============================================================================
// Zone Timer
// ============================================================================

void ASBInGameHUD::DrawZoneTimer()
{
    for (TActorIterator<ASBZoneManager> It(GetWorld()); It; ++It)
    {
        ASBZoneManager* Zone = *It;
        if (!Zone) continue;

        UFont* Font = GEngine->GetTinyFont();
        float CX = Canvas->SizeX * 0.5f;

        if (Zone->IsShrinking())
        {
            float Progress = Zone->GetShrinkProgress();
            FString Str = FString::Printf(TEXT("Zone Shrinking: %.0f%%"), Progress * 100.0f);
            Canvas->SetDrawColor(FColor(255, 80, 80));
            float TW, TH;
            Canvas->TextSize(Font, Str, TW, TH);
            Canvas->DrawText(Font, Str, CX - TW * 0.5f, 55.0f);
        }
        else
        {
            float TimeLeft = Zone->GetTimeUntilShrink();
            if (TimeLeft > 0.0f)
            {
                int32 Minutes = FMath::FloorToInt(TimeLeft / 60.0f);
                int32 Seconds = FMath::FloorToInt(FMath::Fmod(TimeLeft, 60.0f));
                FString Str = FString::Printf(TEXT("Zone %d — %d:%02d"), Zone->GetCurrentPhase() + 1, Minutes, Seconds);
                Canvas->SetDrawColor(FColor::White);
                float TW, TH;
                Canvas->TextSize(Font, Str, TW, TH);
                Canvas->DrawText(Font, Str, CX - TW * 0.5f, 55.0f);
            }
        }
        break;
    }
}

// ============================================================================
// Alive Count
// ============================================================================

void ASBInGameHUD::DrawAliveCount()
{
    int32 Alive = 1;
    ASBBattleRoyaleGameState* GS = Cast<ASBBattleRoyaleGameState>(UGameplayStatics::GetGameState(this));
    if (GS) Alive = GS->GetAlivePlayerCount();

    UFont* Font = GEngine->GetSmallFont();
    FString Str = FString::Printf(TEXT("%d"), Alive);
    float TW, TH;
    Canvas->TextSize(Font, Str, TW, TH);

    float X = Canvas->SizeX - 60.0f;
    float Y = 30.0f;

    // Icon-like background
    DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.7f), X - 5, Y - 2, TW + 30, TH + 4);
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, Str, X + 18, Y);

    // Person icon (simple)
    DrawRect(FLinearColor::White, X + 5, Y + 2, 6, 6);
    DrawRect(FLinearColor::White, X + 2, Y + 10, 12, 8);
}

// ============================================================================
// Kill Feed
// ============================================================================

void ASBInGameHUD::DrawKillFeed()
{
    if (KillFeed.Num() == 0) return;

    UFont* Font = GEngine->GetTinyFont();
    float X = Canvas->SizeX - 20.0f;
    float Y = 80.0f;

    int32 MaxShown = FMath::Min(KillFeed.Num(), 5);
    for (int32 i = KillFeed.Num() - MaxShown; i < KillFeed.Num(); i++)
    {
        const FSBKillFeedEntry& Entry = KillFeed[i];
        FString Line = FString::Printf(TEXT("%s [%s] %s"), *Entry.KillerName, *Entry.WeaponName, *Entry.VictimName);

        FLinearColor BgColor = Entry.bIsLocalKill ? FLinearColor(0.8f, 0.2f, 0.1f, 0.6f) : FLinearColor(0.1f, 0.1f, 0.1f, 0.6f);

        float TW, TH;
        Canvas->TextSize(Font, Line, TW, TH);
        DrawRect(BgColor, X - TW - 10, Y, TW + 10, TH + 4);
        Canvas->SetDrawColor(FColor::White);
        Canvas->DrawText(Font, Line, X - TW - 5, Y + 2);
        Y += TH + 6;
    }
}

// ============================================================================
// Damage Indicators
// ============================================================================

void ASBInGameHUD::DrawDamageIndicators()
{
    if (DamageIndicators.Num() == 0) return;

    const float CX = Canvas->SizeX * 0.5f;
    const float CY = Canvas->SizeY * 0.5f;
    const float Radius = 100.0f;

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    FRotator ViewRot = PC->GetControlRotation();

    for (const FSBDamageIndicator& Indicator : DamageIndicators)
    {
        FVector FlatDir = Indicator.Direction;
        FlatDir.Z = 0;
        FlatDir.Normalize();

        float Angle = FMath::Atan2(FlatDir.Y, FlatDir.X) - FMath::DegreesToRadians(ViewRot.Yaw);

        float IX = CX + FMath::Cos(Angle) * Radius;
        float IY = CY - FMath::Sin(Angle) * Radius;

        float Alpha = FMath::Clamp(Indicator.TimeRemaining / 2.0f, 0.0f, 1.0f);
        DrawRect(FLinearColor(1.0f, 0.1f, 0.1f, Alpha * 0.8f), IX - 4, IY - 12, 8, 24);
    }
}

// ============================================================================
// Hit Marker
// ============================================================================

void ASBInGameHUD::DrawHitMarkerOverlay()
{
    if (HitMarkerTimer <= 0.0f) return;

    const float CX = Canvas->SizeX * 0.5f;
    const float CY = Canvas->SizeY * 0.5f;
    const float Size = 12.0f;
    const float Gap = 4.0f;
    float Alpha = FMath::Clamp(HitMarkerTimer / 0.3f, 0.0f, 1.0f);

    FLinearColor Color = bHitMarkerHeadshot ?
        FLinearColor(1.0f, 0.2f, 0.2f, Alpha) : FLinearColor(1.0f, 1.0f, 1.0f, Alpha);

    // X-shaped hit marker
    float Thick = 2.0f;
    DrawRect(Color, CX - Size, CY - Thick * 0.5f - Gap, Size - Gap, Thick);
    DrawRect(Color, CX + Gap, CY - Thick * 0.5f - Gap, Size - Gap, Thick);
    DrawRect(Color, CX - Size, CY - Thick * 0.5f + Gap, Size - Gap, Thick);
    DrawRect(Color, CX + Gap, CY - Thick * 0.5f + Gap, Size - Gap, Thick);
}

// ============================================================================
// FPS / Ping
// ============================================================================

void ASBInGameHUD::DrawFPSPing()
{
    UFont* Font = GEngine->GetTinyFont();
    float X = 20.0f;
    float Y = Canvas->SizeY - 100.0f;

    // FPS
    Canvas->SetDrawColor(CachedFPS >= 55.0f ? FColor(100, 255, 100) : FColor(255, 100, 100));
    Canvas->DrawText(Font, FString::Printf(TEXT("FPS: %.0f"), CachedFPS), X, Y);

    // Ping
    APlayerController* PC = GetOwningPlayerController();
    if (PC && PC->PlayerState)
    {
        float Ping = PC->PlayerState->GetPingInMilliseconds();
        FColor PingColor = Ping < 60 ? FColor(100, 255, 100) : (Ping < 120 ? FColor(255, 200, 50) : FColor(255, 50, 50));
        Canvas->SetDrawColor(PingColor);
        Canvas->DrawText(Font, FString::Printf(TEXT("Ping: %.0fms"), Ping), X, Y + 14);
    }
}

// ============================================================================
// Loot Popups
// ============================================================================

void ASBInGameHUD::DrawLootPopups()
{
    if (LootPopups.Num() == 0) return;

    UFont* Font = GEngine->GetTinyFont();
    float CX = Canvas->SizeX * 0.5f;
    float Y = Canvas->SizeY * 0.65f;

    for (const FLootPopup& Popup : LootPopups)
    {
        float Alpha = FMath::Clamp(Popup.Timer / 2.0f, 0.0f, 1.0f);
        FString Str = FString::Printf(TEXT("+ %s x%d"), *Popup.ItemName, Popup.Count);

        float TW, TH;
        Canvas->TextSize(Font, Str, TW, TH);
        DrawRect(FLinearColor(0, 0, 0, 0.4f * Alpha), CX - TW * 0.5f - 5, Y, TW + 10, TH + 4);
        Canvas->SetDrawColor(FColor(255, 255, 100, (uint8)(255 * Alpha)));
        Canvas->DrawText(Font, Str, CX - TW * 0.5f, Y + 2);
        Y -= TH + 6;
    }
}

// ============================================================================
// Public API
// ============================================================================

void ASBInGameHUD::AddKillFeedEntry(const FString& Killer, const FString& Victim, const FString& Weapon, bool bLocalKill)
{
    FSBKillFeedEntry Entry;
    Entry.KillerName = Killer;
    Entry.VictimName = Victim;
    Entry.WeaponName = Weapon;
    Entry.TimeStamp = GetWorld()->GetTimeSeconds();
    Entry.bIsLocalKill = bLocalKill;
    KillFeed.Add(Entry);

    if (KillFeed.Num() > 20) KillFeed.RemoveAt(0);
}

void ASBInGameHUD::AddDamageIndicator(const FVector& DamageDirection)
{
    FSBDamageIndicator Ind;
    Ind.Direction = DamageDirection;
    Ind.TimeRemaining = 2.0f;
    DamageIndicators.Add(Ind);
}

void ASBInGameHUD::ShowHitMarker(bool bIsHeadshot)
{
    HitMarkerTimer = 0.3f;
    bHitMarkerHeadshot = bIsHeadshot;
}

void ASBInGameHUD::ShowLootPopup(const FString& ItemName, int32 Count)
{
    FLootPopup Popup;
    Popup.ItemName = ItemName;
    Popup.Count = Count;
    Popup.Timer = 3.0f;
    LootPopups.Add(Popup);

    if (LootPopups.Num() > 5) LootPopups.RemoveAt(0);
}

// ============================================================================
// Helpers
// ============================================================================

void ASBInGameHUD::DrawBar(float X, float Y, float W, float H, float Percent, FLinearColor FillColor, FLinearColor BgColor)
{
    DrawRect(BgColor, X, Y, W, H);
    DrawRect(FillColor, X, Y, W * FMath::Clamp(Percent, 0.0f, 1.0f), H);
}

void ASBInGameHUD::DrawTextCentered(UFont* Font, const FString& Text, float X, float Y, FColor Color)
{
    float TW, TH;
    Canvas->TextSize(Font, Text, TW, TH);
    Canvas->SetDrawColor(Color);
    Canvas->DrawText(Font, Text, X - TW * 0.5f, Y - TH * 0.5f);
}

ASBCharacterBase* ASBInGameHUD::GetPlayerCharacter() const
{
    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return nullptr;
    return Cast<ASBCharacterBase>(PC->GetPawn());
}
