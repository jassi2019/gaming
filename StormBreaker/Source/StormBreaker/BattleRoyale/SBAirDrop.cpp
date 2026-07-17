// Copyright Island Of Death Games. All Rights Reserved.

#include "BattleRoyale/SBAirDrop.h"
#include "Inventory/SBInventoryComponent.h"
#include "Character/SBCharacterBase.h"
#include "StormBreaker.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

ASBAirDrop::ASBAirDrop()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    CrateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrateMesh"));
    RootComponent = CrateMesh;
    CrateMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CrateMesh->SetCollisionResponseToAllChannels(ECR_Block);

    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
    InteractionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    DropState = ESBAirDropState::Flying;
    GroundZ = 0.0f;
}

void ASBAirDrop::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASBAirDrop, DropState);
    DOREPLIFETIME(ASBAirDrop, Loot);
}

void ASBAirDrop::InitAirDrop(const TArray<FSBLootDrop>& InLoot, const FVector& DropLocation)
{
    Loot = InLoot;
    TargetLocation = DropLocation;

    // Trace to find ground
    FHitResult Hit;
    FVector TraceStart = FVector(DropLocation.X, DropLocation.Y, 50000.0f);
    FVector TraceEnd = FVector(DropLocation.X, DropLocation.Y, -1000.0f);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
    {
        GroundZ = Hit.ImpactPoint.Z + 50.0f;
    }
    else
    {
        GroundZ = 0.0f;
    }

    SetActorLocation(FVector(DropLocation.X, DropLocation.Y, DropAltitude));
}

void ASBAirDrop::BeginDrop()
{
    DropState = ESBAirDropState::Dropping;
    SetActorTickEnabled(true);

    if (SmokeTrailVFX.Get())
    {
        ActiveSmokeTrail = UNiagaraFunctionLibrary::SpawnSystemAttached(
            SmokeTrailVFX.Get(), CrateMesh.Get(), NAME_None,
            FVector::ZeroVector, FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset, true);
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Air drop falling to %s"), *TargetLocation.ToString());
}

void ASBAirDrop::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasAuthority() || DropState != ESBAirDropState::Dropping) return;

    FVector Location = GetActorLocation();
    Location.Z -= DropSpeed * DeltaTime;

    if (Location.Z <= GroundZ)
    {
        Location.Z = GroundZ;
        DropState = ESBAirDropState::Landed;
        SetActorTickEnabled(false);

        if (ActiveSmokeTrail)
        {
            ActiveSmokeTrail->DeactivateImmediate();
        }

        UE_LOG(LogSBBattleRoyale, Log, TEXT("Air drop landed at %s"), *Location.ToString());
    }

    SetActorLocation(Location);
}

bool ASBAirDrop::TakeItem(APawn* Looter, int32 ItemIndex)
{
    if (!HasAuthority() || !Looter) return false;
    if (!Loot.IsValidIndex(ItemIndex)) return false;

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(Looter);
    if (!Character) return false;

    USBInventoryComponent* Inv = Character->FindComponentByClass<USBInventoryComponent>();
    if (!Inv) return false;

    const FSBLootDrop& Drop = Loot[ItemIndex];
    if (Inv->AddItem(Drop.ItemID, Drop.Count))
    {
        Loot.RemoveAt(ItemIndex);
        return true;
    }

    return false;
}
