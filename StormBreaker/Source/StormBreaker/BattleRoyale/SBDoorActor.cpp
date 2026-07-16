// Copyright StormBreaker Games. All Rights Reserved.

#include "BattleRoyale/SBDoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"

ASBDoorActor::ASBDoorActor()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;

    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    RootComponent = DoorMesh;

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh)
    {
        DoorMesh->SetStaticMesh(CubeMesh);
        DoorMesh->SetWorldScale3D(FVector(0.04f, 1.0f, 2.0f));
    }

    DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);

    ClosedRotation = FRotator::ZeroRotator;
    bIsOpen = false;
    CurrentAngle = 0.0f;
    TargetAngle = 0.0f;
}

void ASBDoorActor::BeginPlay()
{
    Super::BeginPlay();
    ClosedRotation = GetActorRotation();
}

void ASBDoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASBDoorActor, bIsOpen);
    DOREPLIFETIME(ASBDoorActor, TargetAngle);
}

void ASBDoorActor::ToggleDoor(AActor* Interactor)
{
    if (!HasAuthority()) return;

    bIsOpen = !bIsOpen;

    if (bIsOpen)
    {
        // Determine push direction based on interactor position
        if (Interactor)
        {
            FVector ToInteractor = Interactor->GetActorLocation() - GetActorLocation();
            FVector DoorForward = GetActorForwardVector();
            float Dot = FVector::DotProduct(DoorForward, ToInteractor.GetSafeNormal());
            TargetAngle = (Dot > 0) ? -OpenAngle : OpenAngle;
        }
        else
        {
            TargetAngle = OpenAngle;
        }
    }
    else
    {
        TargetAngle = 0.0f;
    }

    OnRep_IsOpen();
}

void ASBDoorActor::OnRep_IsOpen()
{
    /* TargetAngle is replicated, nothing to do here */
}

void ASBDoorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (FMath::IsNearlyEqual(CurrentAngle, TargetAngle, 0.5f)) return;

    CurrentAngle = FMath::FInterpTo(CurrentAngle, TargetAngle, DeltaTime, OpenSpeed);

    FRotator NewRot = ClosedRotation;
    NewRot.Yaw += CurrentAngle;
    SetActorRotation(NewRot);
}
