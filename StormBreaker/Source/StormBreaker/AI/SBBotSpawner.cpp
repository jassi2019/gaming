// Copyright Island Of Death Games. All Rights Reserved.

#include "AI/SBBotSpawner.h"
#include "AI/SBBotController.h"
#include "Character/SBCharacterBase.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponDataAsset.h"
#include "BattleRoyale/SBKnockReviveComponent.h"
#include "StormBreaker.h"

void USBBotSpawner::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogSBAI, Log, TEXT("Bot Spawner initialized."));
}

void USBBotSpawner::SpawnBots(int32 Count, TSubclassOf<ASBCharacterBase> PawnClass)
{
    UWorld* World = GetWorld();
    if (!World || !PawnClass) return;

    UE_LOG(LogSBAI, Log, TEXT("Spawning %d bots..."), Count);

    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLoc = GetRandomSpawnLocation();
        FRotator SpawnRot = FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ASBCharacterBase* BotPawn = World->SpawnActor<ASBCharacterBase>(PawnClass, SpawnLoc, SpawnRot, SpawnParams);
        if (!BotPawn) continue;

        FActorSpawnParameters ControllerParams;
        ControllerParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        ASBBotController* BotController = World->SpawnActor<ASBBotController>(
            ASBBotController::StaticClass(), SpawnLoc, SpawnRot, ControllerParams);

        if (!BotController)
        {
            BotPawn->Destroy();
            continue;
        }

        BotController->Difficulty = GetDifficultyForIndex(i, Count);
        BotController->Possess(BotPawn);

        // Give bot a starter weapon
        USBWeaponComponent* WeaponComp = BotPawn->WeaponComponent;
        if (WeaponComp)
        {
            USBWeaponDataAsset* BotWeapon = NewObject<USBWeaponDataAsset>(BotController, TEXT("BotWeapon"));
            BotWeapon->WeaponID = FName("BotAK");
            BotWeapon->DisplayName = FText::FromString(TEXT("Bot AK"));
            BotWeapon->WeaponType = ESBWeaponType::AssaultRifle;
            BotWeapon->DefaultSlot = ESBWeaponSlot::Primary;
            BotWeapon->DamageMethod = ESBDamageType::Hitscan;
            BotWeapon->AvailableFireModes = { ESBFireMode::Auto };
            BotWeapon->DefaultFireMode = ESBFireMode::Auto;
            BotWeapon->FireRate = 600.0f;
            BotWeapon->Damage.BaseDamage = 30.0f;
            BotWeapon->Damage.HeadshotMultiplier = 2.0f;
            BotWeapon->Damage.EffectiveRange = 4000.0f;
            BotWeapon->Damage.MaxRange = 15000.0f;
            BotWeapon->MagazineSize = 30;
            BotWeapon->MaxReserveAmmo = 999;
            BotWeapon->ReloadTime = 2.3f;
            BotWeapon->Recoil.VerticalMin = 0.2f;
            BotWeapon->Recoil.VerticalMax = 0.4f;
            BotWeapon->Spread.HipFireSpread = 3.0f;
            BotWeapon->Spread.ADSSpread = 1.0f;

            WeaponComp->AddWeapon(BotWeapon, ESBWeaponSlot::Primary);
        }

        SpawnedBots.Add(BotController);
    }

    UE_LOG(LogSBAI, Log, TEXT("Spawned %d bots successfully."), SpawnedBots.Num());
}

void USBBotSpawner::DespawnAllBots()
{
    for (ASBBotController* Bot : SpawnedBots)
    {
        if (Bot)
        {
            APawn* Pawn = Bot->GetPawn();
            Bot->UnPossess();
            if (Pawn) Pawn->Destroy();
            Bot->Destroy();
        }
    }
    SpawnedBots.Empty();
    UE_LOG(LogSBAI, Log, TEXT("All bots despawned."));
}

int32 USBBotSpawner::GetAliveBotCount() const
{
    int32 Alive = 0;
    for (const ASBBotController* Bot : SpawnedBots)
    {
        if (Bot && Bot->CurrentState != ESBBotState::Dead)
        {
            Alive++;
        }
    }
    return Alive;
}

FVector USBBotSpawner::GetRandomSpawnLocation() const
{
    float Angle = FMath::FRandRange(0.0f, UE_TWO_PI);
    float Radius = FMath::FRandRange(0.0f, SpawnRadius);
    return FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, SpawnHeight);
}

ESBBotDifficulty USBBotSpawner::GetDifficultyForIndex(int32 Index, int32 Total) const
{
    float Normalized = static_cast<float>(Index) / FMath::Max(1, Total);

    if (Normalized < EasyPercent)
        return ESBBotDifficulty::Easy;
    if (Normalized < EasyPercent + NormalPercent)
        return ESBBotDifficulty::Normal;
    if (Normalized < EasyPercent + NormalPercent + HardPercent)
        return ESBBotDifficulty::Hard;

    return ESBBotDifficulty::Pro;
}
