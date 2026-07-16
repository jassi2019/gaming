// Copyright StormBreaker Games. All Rights Reserved.

#include "BattleRoyale/SBParachuteComponent.h"
#include "Character/SBCharacterBase.h"
#include "Character/SBCharacterMovementComponent.h"
#include "StormBreaker.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

USBParachuteComponent::USBParachuteComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    ParachuteState = ESBParachuteState::None;
    WindOffset = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal() * WindStrength;
}

void USBParachuteComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USBParachuteComponent, ParachuteState);
}

void USBParachuteComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    switch (ParachuteState)
    {
    case ESBParachuteState::FreeFall:
        TickFreeFall(DeltaTime);
        break;
    case ESBParachuteState::Deployed:
        TickDeployed(DeltaTime);
        break;
    default:
        break;
    }
}

void USBParachuteComponent::BeginFreeFall()
{
    SetState(ESBParachuteState::FreeFall);
    SetComponentTickEnabled(true);

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (Character)
    {
        UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
        if (CMC)
        {
            CMC->SetMovementMode(MOVE_Flying);
            CMC->GravityScale = 0.0f;
        }
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Free fall started."));
}

void USBParachuteComponent::DeployParachute()
{
    if (ParachuteState != ESBParachuteState::FreeFall) return;

    if (!GetOwner()->HasAuthority())
    {
        Server_DeployParachute();
    }

    SetState(ESBParachuteState::Deployed);

    // Spawn parachute mesh
    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (Character && ParachuteMesh && !ParachuteMeshComp)
    {
        ParachuteMeshComp = NewObject<UStaticMeshComponent>(Character);
        ParachuteMeshComp->SetStaticMesh(ParachuteMesh);
        ParachuteMeshComp->AttachToComponent(Character->GetRootComponent(),
            FAttachmentTransformRules::KeepRelativeTransform);
        ParachuteMeshComp->SetRelativeLocation(FVector(0, 0, 300.0f));
        ParachuteMeshComp->RegisterComponent();
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Parachute deployed."));
}

void USBParachuteComponent::SetSteeringInput(FVector2D Input)
{
    CurrentSteeringInput = Input;
}

float USBParachuteComponent::GetAltitudeAboveGround() const
{
    return TraceAltitude();
}

void USBParachuteComponent::TickFreeFall(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Location = Owner->GetActorLocation();
    FRotator Rotation = Owner->GetActorRotation();

    // Apply steering
    Rotation.Yaw += CurrentSteeringInput.X * SteeringSpeed * DeltaTime;
    Owner->SetActorRotation(Rotation);

    // Move forward and down
    FVector Forward = Rotation.Vector();
    float ForwardInput = FMath::Clamp(CurrentSteeringInput.Y, -1.0f, 1.0f);
    float ActualForwardSpeed = FreeFallForwardSpeed * (0.5f + 0.5f * ForwardInput);

    FVector Velocity = Forward * ActualForwardSpeed + FVector(0, 0, -FreeFallSpeed);
    Location += Velocity * DeltaTime;
    Owner->SetActorLocation(Location);

    // Auto-deploy check
    float Altitude = TraceAltitude();
    if (Altitude <= AutoDeployAltitude && Altitude > 0.0f)
    {
        DeployParachute();
    }
}

void USBParachuteComponent::TickDeployed(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Location = Owner->GetActorLocation();
    FRotator Rotation = Owner->GetActorRotation();

    // Steering
    Rotation.Yaw += CurrentSteeringInput.X * SteeringSpeed * DeltaTime;
    Owner->SetActorRotation(Rotation);

    // Forward + descent
    FVector Forward = Rotation.Vector();
    float ForwardInput = FMath::Clamp(CurrentSteeringInput.Y, 0.0f, 1.0f);

    FVector Velocity = Forward * ParachuteForwardSpeed * ForwardInput
                     + FVector(0, 0, -ParachuteDescendSpeed)
                     + WindOffset * DeltaTime;

    Location += Velocity * DeltaTime;
    Owner->SetActorLocation(Location);

    // Landing check
    float Altitude = TraceAltitude();
    if (Altitude <= LandingAltitude && Altitude > 0.0f)
    {
        Land();
    }
}

void USBParachuteComponent::Land()
{
    SetState(ESBParachuteState::Landed);
    SetComponentTickEnabled(false);

    ASBCharacterBase* Character = Cast<ASBCharacterBase>(GetOwner());
    if (Character)
    {
        UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
        if (CMC)
        {
            CMC->SetMovementMode(MOVE_Falling);
            CMC->GravityScale = 1.0f;
        }
    }

    // Remove parachute mesh
    if (ParachuteMeshComp)
    {
        ParachuteMeshComp->DestroyComponent();
        ParachuteMeshComp = nullptr;
    }

    UE_LOG(LogSBBattleRoyale, Log, TEXT("Landed."));
}

float USBParachuteComponent::TraceAltitude() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return 0.0f;

    FVector Start = Owner->GetActorLocation();
    FVector End = Start - FVector(0, 0, 50000.0f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
    {
        return Start.Z - Hit.ImpactPoint.Z;
    }

    return 50000.0f;
}

void USBParachuteComponent::SetState(ESBParachuteState NewState)
{
    if (ParachuteState == NewState) return;
    ParachuteState = NewState;
    OnParachuteStateChanged.Broadcast(NewState);
}

void USBParachuteComponent::OnRep_ParachuteState()
{
    OnParachuteStateChanged.Broadcast(ParachuteState);
}

void USBParachuteComponent::Server_DeployParachute_Implementation()
{
    DeployParachute();
}
