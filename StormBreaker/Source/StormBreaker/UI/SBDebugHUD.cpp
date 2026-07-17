// Copyright Island Of Death Games. All Rights Reserved.

#include "UI/SBDebugHUD.h"
#include "Character/SBCharacterBase.h"
#include "Character/SBCharacterMovementComponent.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponBase.h"
#include "Weapon/SBWeaponDataAsset.h"
#include "Core/SBPlayerState.h"
#include "Core/SBAttributeSet.h"
#include "Core/SBBattleRoyaleGameState.h"
#include "Inventory/SBInventoryComponent.h"
#include "StormBreaker.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Kismet/GameplayStatics.h"

void ASBHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas) return;

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(PC->GetPawn());

    const float ScreenW = Canvas->SizeX;
    const float ScreenH = Canvas->SizeY;
    const float Padding = 20.0f;
    const float BarWidth = 250.0f;
    const float BarHeight = 18.0f;
    const float LineHeight = 24.0f;

    UFont* Font = GEngine->GetSmallFont();

    // ===== BOTTOM LEFT: Health + Shield Bars =====
    float BottomY = ScreenH - Padding;

    // Health
    float Health = 100.0f;
    float MaxHealth = 100.0f;
    float Shield = 0.0f;
    float MaxShield = 150.0f;

    if (Character)
    {
        ASBPlayerState* PS = Character->GetPlayerState<ASBPlayerState>();
        if (PS && PS->AttributeSet)
        {
            Health = PS->AttributeSet->GetHealth();
            MaxHealth = PS->AttributeSet->GetMaxHealth();
            Shield = PS->AttributeSet->GetShield();
            MaxShield = PS->AttributeSet->GetMaxShield();
        }
    }

    // Shield bar
    float ShieldY = BottomY - BarHeight * 2 - 8.0f;
    DrawHealthBar(Padding, ShieldY, BarWidth, BarHeight,
        MaxShield > 0.0f ? Shield / MaxShield : 0.0f,
        FLinearColor(0.2f, 0.5f, 1.0f), FString::Printf(TEXT("Shield: %.0f"), Shield));

    // Health bar
    float HealthY = BottomY - BarHeight;
    DrawHealthBar(Padding, HealthY, BarWidth, BarHeight,
        MaxHealth > 0.0f ? Health / MaxHealth : 0.0f,
        FLinearColor(0.1f, 0.9f, 0.2f), FString::Printf(TEXT("HP: %.0f / %.0f"), Health, MaxHealth));

    // ===== BOTTOM RIGHT: Ammo =====
    FString AmmoStr = TEXT("-- / --");
    FString WeaponName = TEXT("No Weapon");
    FString FireModeStr = TEXT("");

    if (Character && Character->WeaponComponent)
    {
        ASBWeaponBase* Weapon = Character->WeaponComponent->GetActiveWeapon();
        if (Weapon)
        {
            AmmoStr = FString::Printf(TEXT("%d / %d"),
                Weapon->GetCurrentMagazine(), Weapon->GetCurrentReserve());

            if (Weapon->WeaponData)
            {
                WeaponName = Weapon->WeaponData->DisplayName.ToString();
            }

            switch (Weapon->GetCurrentFireMode())
            {
            case ESBFireMode::Single: FireModeStr = TEXT("[SINGLE]"); break;
            case ESBFireMode::Burst:  FireModeStr = TEXT("[BURST]");  break;
            case ESBFireMode::Auto:   FireModeStr = TEXT("[AUTO]");   break;
            }
        }
    }

    // Weapon name
    Canvas->SetDrawColor(FColor::White);
    float TextW, TextH;
    Canvas->TextSize(Font, WeaponName, TextW, TextH);
    Canvas->DrawText(Font, WeaponName, ScreenW - Padding - TextW, BottomY - LineHeight * 3);

    // Fire mode
    Canvas->SetDrawColor(FColor(200, 200, 100));
    Canvas->TextSize(Font, FireModeStr, TextW, TextH);
    Canvas->DrawText(Font, FireModeStr, ScreenW - Padding - TextW, BottomY - LineHeight * 2);

    // Ammo
    Canvas->SetDrawColor(FColor::White);
    FString AmmoLabel = FString::Printf(TEXT("Ammo: %s"), *AmmoStr);
    Canvas->TextSize(Font, AmmoLabel, TextW, TextH);
    Canvas->DrawText(Font, AmmoLabel, ScreenW - Padding - TextW, BottomY - LineHeight);

    // ===== TOP RIGHT: Alive Players =====
    int32 AliveCount = 1;
    ASBBattleRoyaleGameState* GS = Cast<ASBBattleRoyaleGameState>(
        UGameplayStatics::GetGameState(this));
    if (GS)
    {
        AliveCount = GS->GetAlivePlayerCount();
    }

    FString AliveStr = FString::Printf(TEXT("Alive: %d"), AliveCount);
    Canvas->SetDrawColor(FColor::White);
    Canvas->TextSize(Font, AliveStr, TextW, TextH);
    Canvas->DrawText(Font, AliveStr, ScreenW - Padding - TextW, Padding);

    // ===== TOP LEFT: Stance + Speed =====
    if (Character)
    {
        FString StanceStr;
        switch (Character->GetCurrentStance())
        {
        case ESBStance::Standing:  StanceStr = TEXT("Standing"); break;
        case ESBStance::Crouching: StanceStr = TEXT("Crouching"); break;
        case ESBStance::Prone:     StanceStr = TEXT("Prone"); break;
        }

        if (Character->GetSBMovement() && Character->GetSBMovement()->IsSprinting())
        {
            StanceStr = TEXT("Sprinting");
        }
        if (Character->IsAiming())
        {
            StanceStr += TEXT(" [ADS]");
        }

        Canvas->SetDrawColor(FColor(180, 180, 180));
        Canvas->DrawText(Font, StanceStr, Padding, Padding);

        float Speed = Character->GetVelocity().Size2D();
        FString SpeedStr = FString::Printf(TEXT("Speed: %.0f cm/s"), Speed);
        Canvas->DrawText(Font, SpeedStr, Padding, Padding + LineHeight);
    }
}

void ASBHUD::DrawHealthBar(float X, float Y, float Width, float Height, float Percent, FLinearColor Color, const FString& Label)
{
    // Background
    FLinearColor BgColor(0.1f, 0.1f, 0.1f, 0.8f);
    DrawRect(BgColor, X, Y, Width, Height);

    // Fill
    float FillWidth = Width * FMath::Clamp(Percent, 0.0f, 1.0f);
    DrawRect(Color, X, Y, FillWidth, Height);

    // Border
    FLinearColor BorderColor(0.5f, 0.5f, 0.5f, 1.0f);
    DrawRect(BorderColor, X, Y, Width, 1.0f);
    DrawRect(BorderColor, X, Y + Height - 1.0f, Width, 1.0f);
    DrawRect(BorderColor, X, Y, 1.0f, Height);
    DrawRect(BorderColor, X + Width - 1.0f, Y, 1.0f, Height);

    // Label
    UFont* Font = GEngine->GetTinyFont();
    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, Label, X + 4.0f, Y + 2.0f);
}
