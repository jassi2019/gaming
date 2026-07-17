// Copyright Island Of Death Games. All Rights Reserved.

#include "UI/SBMatchHUDWidget.h"
#include "Character/SBCharacterBase.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponBase.h"
#include "Core/SBPlayerState.h"
#include "Core/SBAttributeSet.h"
#include "Core/SBBattleRoyaleGameState.h"
#include "StormBreaker.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

void USBMatchHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void USBMatchHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    UpdateHUD();
}

void USBMatchHUDWidget::UpdateHUD()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(PC->GetPawn());

    // --- Health & Shield ---
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

    if (HealthText)
    {
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("HP: %.0f / %.0f"), Health, MaxHealth)));
    }
    if (ShieldText)
    {
        ShieldText->SetText(FText::FromString(FString::Printf(TEXT("Shield: %.0f / %.0f"), Shield, MaxShield)));
    }
    if (HealthBar)
    {
        HealthBar->SetPercent(MaxHealth > 0.0f ? Health / MaxHealth : 0.0f);
    }
    if (ShieldBar)
    {
        ShieldBar->SetPercent(MaxShield > 0.0f ? Shield / MaxShield : 0.0f);
    }

    // --- Ammo ---
    if (AmmoText)
    {
        FString AmmoStr = TEXT("-- / --");
        if (Character)
        {
            USBWeaponComponent* WeaponComp = Character->WeaponComponent;
            if (WeaponComp)
            {
                ASBWeaponBase* Weapon = WeaponComp->GetActiveWeapon();
                if (Weapon)
                {
                    AmmoStr = FString::Printf(TEXT("%d / %d"),
                        Weapon->GetCurrentMagazine(), Weapon->GetCurrentReserve());
                }
            }
        }
        AmmoText->SetText(FText::FromString(AmmoStr));
    }

    // --- Alive Players ---
    if (AliveText)
    {
        int32 AliveCount = 0;
        ASBBattleRoyaleGameState* GS = Cast<ASBBattleRoyaleGameState>(
            UGameplayStatics::GetGameState(this));
        if (GS)
        {
            AliveCount = GS->GetAlivePlayerCount();
        }
        AliveText->SetText(FText::FromString(FString::Printf(TEXT("Alive: %d"), AliveCount)));
    }
}
