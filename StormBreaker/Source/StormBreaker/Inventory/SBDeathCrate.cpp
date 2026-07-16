// Copyright StormBreaker Games. All Rights Reserved.

#include "Inventory/SBDeathCrate.h"
#include "Inventory/SBInventoryComponent.h"
#include "Character/SBCharacterBase.h"
#include "StormBreaker.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ASBDeathCrate::ASBDeathCrate()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = false;

    CrateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrateMesh"));
    RootComponent = CrateMesh;
    CrateMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CrateMesh->SetCollisionResponseToAllChannels(ECR_Block);
    CrateMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->SetBoxExtent(FVector(100.0f, 100.0f, 50.0f));
    InteractionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    SetNetCullDistanceSquared(10000.0f * 10000.0f);
}

void ASBDeathCrate::BeginPlay()
{
    Super::BeginPlay();

    if (LifeSpan > 0.0f)
    {
        SetLifeSpan(LifeSpan);
    }
}

void ASBDeathCrate::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASBDeathCrate, Contents);
    DOREPLIFETIME(ASBDeathCrate, DeadPlayerName);
}

void ASBDeathCrate::InitFromPlayerItems(const TArray<FSBInventoryItem>& PlayerItems, const FString& PlayerName)
{
    Contents = PlayerItems;
    DeadPlayerName = PlayerName;

    UE_LOG(LogSBInventory, Log, TEXT("Death crate created for %s with %d item stacks."),
        *PlayerName, Contents.Num());
}

bool ASBDeathCrate::TakeItem(APawn* Looter, const FName& ItemID, int32 Count)
{
    if (!HasAuthority() || !Looter) return false;

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(Looter);
    if (!Character) return false;

    USBInventoryComponent* Inv = Character->FindComponentByClass<USBInventoryComponent>();
    if (!Inv) return false;

    // Find item in contents
    for (int32 i = Contents.Num() - 1; i >= 0; i--)
    {
        if (Contents[i].ItemID == ItemID)
        {
            int32 ToTake = FMath::Min(Count, Contents[i].StackCount);
            if (Inv->AddItem(ItemID, ToTake))
            {
                Contents[i].StackCount -= ToTake;
                if (Contents[i].StackCount <= 0)
                {
                    Contents.RemoveAt(i);
                }

                if (IsEmpty())
                {
                    SetLifeSpan(5.0f);
                }

                return true;
            }
            break;
        }
    }

    return false;
}

void ASBDeathCrate::TakeAll(APawn* Looter)
{
    if (!HasAuthority() || !Looter) return;

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(Looter);
    if (!Character) return;

    USBInventoryComponent* Inv = Character->FindComponentByClass<USBInventoryComponent>();
    if (!Inv) return;

    for (int32 i = Contents.Num() - 1; i >= 0; i--)
    {
        if (Inv->AddItem(Contents[i].ItemID, Contents[i].StackCount))
        {
            Contents.RemoveAt(i);
        }
    }

    if (IsEmpty())
    {
        SetLifeSpan(5.0f);
    }
}
