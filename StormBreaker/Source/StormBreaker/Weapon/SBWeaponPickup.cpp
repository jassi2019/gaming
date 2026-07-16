// Copyright StormBreaker Games. All Rights Reserved.

#include "Weapon/SBWeaponPickup.h"
#include "Weapon/SBWeaponBase.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponDataAsset.h"
#include "Character/SBCharacterBase.h"
#include "StormBreaker.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ASBWeaponPickup::ASBWeaponPickup()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;

    PickupMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
    RootComponent = PickupMeshComp;
    PickupMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->InitSphereRadius(150.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    InteractionSphere->SetGenerateOverlapEvents(true);

    StoredMagazineAmmo = 0;
    StoredReserveAmmo = 0;
    BobTimer = 0.0f;

    // Net cull distance for performance
    SetNetCullDistanceSquared(5000.0f * 5000.0f);
}

void ASBWeaponPickup::InitPickup(USBWeaponDataAsset* InData, int32 InMagAmmo, int32 InReserveAmmo)
{
    WeaponData = InData;
    StoredMagazineAmmo = InMagAmmo;
    StoredReserveAmmo = InReserveAmmo;
}

void ASBWeaponPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASBWeaponPickup, WeaponData);
    DOREPLIFETIME(ASBWeaponPickup, StoredMagazineAmmo);
    DOREPLIFETIME(ASBWeaponPickup, StoredReserveAmmo);
}

void ASBWeaponPickup::BeginPlay()
{
    Super::BeginPlay();

    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASBWeaponPickup::OnOverlapBegin);

    BobTimer = FMath::FRandRange(0.0f, UE_TWO_PI);
    BaseZ = GetActorLocation().Z;
}

void ASBWeaponPickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Gentle bob and rotate
    BobTimer += DeltaTime * 2.0f;
    FVector NewLocation = GetActorLocation();
    NewLocation.Z = BaseZ + FMath::Sin(BobTimer) * 15.0f;
    SetActorLocation(NewLocation);
    AddActorWorldRotation(FRotator(0.0f, 45.0f * DeltaTime, 0.0f));
}

void ASBWeaponPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Auto-pickup on overlap (can be changed to manual interact in Phase 4)
}

void ASBWeaponPickup::Interact(APawn* InteractingPawn)
{
    if (!HasAuthority() || !WeaponData) return;

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(InteractingPawn);
    if (!Character) return;

    USBWeaponComponent* WeaponComp = Character->FindComponentByClass<USBWeaponComponent>();
    if (!WeaponComp) return;

    ESBWeaponSlot Slot = WeaponData->DefaultSlot;
    if (WeaponComp->AddWeapon(WeaponData, Slot))
    {
        // Set ammo from pickup
        ASBWeaponBase* Weapon = WeaponComp->GetWeaponInSlot(Slot);
        if (Weapon)
        {
            Weapon->SetAmmo(StoredMagazineAmmo, StoredReserveAmmo);
        }

        UE_LOG(LogSBWeapon, Log, TEXT("Pickup '%s' collected by %s"),
            *WeaponData->DisplayName.ToString(), *GetNameSafe(Character));

        Destroy();
    }
}
