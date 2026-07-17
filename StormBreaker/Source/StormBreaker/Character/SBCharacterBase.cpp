// Copyright Island Of Death Games. All Rights Reserved.

#include "Character/SBCharacterBase.h"
#include "Character/SBCharacterMovementComponent.h"
#include "StormBreaker.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "AbilitySystemComponent.h"
#include "Weapon/SBWeaponComponent.h"
#include "Weapon/SBWeaponTypes.h"
#include "Inventory/SBInventoryComponent.h"
#include "BattleRoyale/SBKnockReviveComponent.h"
#include "BattleRoyale/SBParachuteComponent.h"
#include "BattleRoyale/SBMinimapDataComponent.h"
#include "Multiplayer/SBAntiCheatComponent.h"
#include "Core/SBPlayerState.h"
#include "Net/UnrealNetwork.h"

ASBCharacterBase::ASBCharacterBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USBCharacterMovementComponent>(
          ACharacter::CharacterMovementComponentName))
    , bIsAiming(false)
    , CurrentBoomLength(0.0f)
    , CurrentFOV(0.0f)
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // Cache custom movement component
    SBMovementComponent = Cast<USBCharacterMovementComponent>(GetCharacterMovement());

    // Capsule
    GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
    GetCapsuleComponent()->SetCapsuleRadius(42.0f);

    // Load Mannequin mesh
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
        TEXT("/Game/Characters/Mannequins/Meshes/SK_Mannequin"));
    if (MeshFinder.Succeeded())
    {
        GetMesh()->SetSkeletalMeshAsset(MeshFinder.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }

    // Don't rotate character to camera
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = DefaultBoomLength;
    CameraBoom->SocketOffset = CameraSocketOffset;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 15.0f;
    CameraBoom->CameraLagMaxDistance = 50.0f;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed = 20.0f;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 12.0f;

    // Follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->FieldOfView = DefaultFOV;

    // Movement component config — orient to movement
    if (SBMovementComponent)
    {
        SBMovementComponent->bOrientRotationToMovement = true;
        SBMovementComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    }

    // Weapon component
    WeaponComponent = CreateDefaultSubobject<USBWeaponComponent>(TEXT("WeaponComponent"));

    // Inventory component
    InventoryComponent = CreateDefaultSubobject<USBInventoryComponent>(TEXT("InventoryComponent"));

    // Knock/Revive component
    KnockReviveComponent = CreateDefaultSubobject<USBKnockReviveComponent>(TEXT("KnockReviveComponent"));

    // Parachute component
    ParachuteComponent = CreateDefaultSubobject<USBParachuteComponent>(TEXT("ParachuteComponent"));

    // Minimap data component
    MinimapDataComponent = CreateDefaultSubobject<USBMinimapDataComponent>(TEXT("MinimapDataComponent"));

    // Anti-cheat component (server-only validation)
    AntiCheatComponent = CreateDefaultSubobject<USBAntiCheatComponent>(TEXT("AntiCheatComponent"));

    // Initialize camera state
    CurrentBoomLength = DefaultBoomLength;
    CurrentFOV = DefaultFOV;

    // Replication
    SetNetUpdateFrequency(60.0f);
    SetMinNetUpdateFrequency(30.0f);
}

void ASBCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // Bind input mapping context for local player
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    InitAbilitySystem();

    UE_LOG(LogSBCharacter, Log, TEXT("Character '%s' BeginPlay complete."), *GetNameSafe(this));
}

void ASBCharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateCameraADS(DeltaTime);
}

void ASBCharacterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitAbilitySystem();
}

void ASBCharacterBase::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitAbilitySystem();
}

// --- GAS ---

void ASBCharacterBase::InitAbilitySystem()
{
    ASBPlayerState* PS = GetPlayerState<ASBPlayerState>();
    if (!PS) return;

    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    if (!ASC) return;

    ASC->InitAbilityActorInfo(PS, this);
}

UAbilitySystemComponent* ASBCharacterBase::GetAbilitySystemComponent() const
{
    ASBPlayerState* PS = GetPlayerState<ASBPlayerState>();
    return PS ? PS->GetAbilitySystemComponent() : nullptr;
}

// --- Replication ---

void ASBCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ASBCharacterBase, bIsAiming, COND_SkipOwner);
}

void ASBCharacterBase::OnRep_IsAiming()
{
    if (SBMovementComponent)
    {
        SBMovementComponent->SetAiming(bIsAiming);
    }

    // Update third-person visual (camera only affects locally controlled)
    if (SBMovementComponent)
    {
        SBMovementComponent->bOrientRotationToMovement = !bIsAiming;
        SBMovementComponent->bUseControllerDesiredRotation = bIsAiming;
    }
}

void ASBCharacterBase::Server_SetAiming_Implementation(bool bNewAiming)
{
    bIsAiming = bNewAiming;
    OnRep_IsAiming();
}

// --- Input Setup ---

void ASBCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EIC)
    {
        UE_LOG(LogSBCharacter, Error, TEXT("Failed to cast to UEnhancedInputComponent!"));
        return;
    }

    // Movement
    if (IA_Move)
    {
        EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ASBCharacterBase::Input_Move);
    }

    // Look
    if (IA_Look)
    {
        EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ASBCharacterBase::Input_Look);
    }

    // Jump
    if (IA_Jump)
    {
        EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &ASBCharacterBase::Input_JumpStart);
        EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ASBCharacterBase::Input_JumpStop);
    }

    // Sprint
    if (IA_Sprint)
    {
        EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this, &ASBCharacterBase::Input_SprintStart);
        EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &ASBCharacterBase::Input_SprintStop);
    }

    // Crouch
    if (IA_Crouch)
    {
        EIC->BindAction(IA_Crouch, ETriggerEvent::Started, this, &ASBCharacterBase::Input_CrouchToggle);
    }

    // Prone
    if (IA_Prone)
    {
        EIC->BindAction(IA_Prone, ETriggerEvent::Started, this, &ASBCharacterBase::Input_ProneToggle);
    }

    // ADS
    if (IA_ADS)
    {
        EIC->BindAction(IA_ADS, ETriggerEvent::Started, this, &ASBCharacterBase::Input_ADSStart);
        EIC->BindAction(IA_ADS, ETriggerEvent::Completed, this, &ASBCharacterBase::Input_ADSStop);
    }

    // Interact (also triggers vault/mantle)
    if (IA_Interact)
    {
        EIC->BindAction(IA_Interact, ETriggerEvent::Started, this, &ASBCharacterBase::Input_Interact);
    }

    // Fire
    if (IA_Fire)
    {
        EIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &ASBCharacterBase::Input_FireStart);
        EIC->BindAction(IA_Fire, ETriggerEvent::Completed, this, &ASBCharacterBase::Input_FireStop);
    }

    // Reload
    if (IA_Reload)
    {
        EIC->BindAction(IA_Reload, ETriggerEvent::Started, this, &ASBCharacterBase::Input_Reload);
    }

    // Weapon slots
    if (IA_WeaponSlot1)
    {
        EIC->BindAction(IA_WeaponSlot1, ETriggerEvent::Started, this, &ASBCharacterBase::Input_WeaponSlot1);
    }
    if (IA_WeaponSlot2)
    {
        EIC->BindAction(IA_WeaponSlot2, ETriggerEvent::Started, this, &ASBCharacterBase::Input_WeaponSlot2);
    }
    if (IA_WeaponSlot3)
    {
        EIC->BindAction(IA_WeaponSlot3, ETriggerEvent::Started, this, &ASBCharacterBase::Input_WeaponSlot3);
    }
}

// --- Input Handlers ---

void ASBCharacterBase::Input_Move(const FInputActionValue& Value)
{
    const FVector2D Input = Value.Get<FVector2D>();
    if (Input.IsNearlyZero()) return;

    // Calculate movement direction relative to camera
    const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
    const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDir, Input.Y);
    AddMovementInput(RightDir, Input.X);
}

void ASBCharacterBase::Input_Look(const FInputActionValue& Value)
{
    const FVector2D Input = Value.Get<FVector2D>();
    if (Input.IsNearlyZero()) return;

    AddControllerYawInput(Input.X);
    AddControllerPitchInput(Input.Y);
}

void ASBCharacterBase::Input_JumpStart(const FInputActionValue& Value)
{
    // Try vault/mantle first if near a ledge
    if (SBMovementComponent && SBMovementComponent->TryMantleOrVault())
    {
        return;
    }

    Jump();
}

void ASBCharacterBase::Input_JumpStop(const FInputActionValue& Value)
{
    StopJumping();
}

void ASBCharacterBase::Input_SprintStart(const FInputActionValue& Value)
{
    if (SBMovementComponent)
    {
        SBMovementComponent->StartSprinting();
    }
}

void ASBCharacterBase::Input_SprintStop(const FInputActionValue& Value)
{
    if (SBMovementComponent)
    {
        SBMovementComponent->StopSprinting();
    }
}

void ASBCharacterBase::Input_CrouchToggle(const FInputActionValue& Value)
{
    if (!SBMovementComponent) return;

    // Can't crouch while prone
    if (SBMovementComponent->IsProning())
    {
        SBMovementComponent->StopProne();
        return;
    }

    if (bIsCrouched)
    {
        UnCrouch();
    }
    else
    {
        Crouch();
    }
}

void ASBCharacterBase::Input_ProneToggle(const FInputActionValue& Value)
{
    if (SBMovementComponent)
    {
        if (SBMovementComponent->IsProning())
        {
            SBMovementComponent->StopProne();
        }
        else
        {
            SBMovementComponent->StartProne();
        }
    }
}

void ASBCharacterBase::Input_ADSStart(const FInputActionValue& Value)
{
    bIsAiming = true;
    Server_SetAiming(true);
    OnRep_IsAiming();
}

void ASBCharacterBase::Input_ADSStop(const FInputActionValue& Value)
{
    bIsAiming = false;
    Server_SetAiming(false);
    OnRep_IsAiming();
}

void ASBCharacterBase::Input_Interact(const FInputActionValue& Value)
{
    // Try vault/mantle
    if (SBMovementComponent && SBMovementComponent->TryMantleOrVault())
    {
        return;
    }

    // Interaction with world objects will be handled in Phase 4 (Inventory)
    UE_LOG(LogSBCharacter, Verbose, TEXT("Interact pressed."));
}

void ASBCharacterBase::Input_FireStart(const FInputActionValue& Value)
{
    if (WeaponComponent)
    {
        WeaponComponent->StartFire();
    }
}

void ASBCharacterBase::Input_FireStop(const FInputActionValue& Value)
{
    if (WeaponComponent)
    {
        WeaponComponent->StopFire();
    }
}

void ASBCharacterBase::Input_Reload(const FInputActionValue& Value)
{
    if (WeaponComponent)
    {
        WeaponComponent->Reload();
    }
}

void ASBCharacterBase::Input_WeaponSlot1(const FInputActionValue& Value)
{
    if (WeaponComponent)
    {
        WeaponComponent->SwitchToSlot(ESBWeaponSlot::Primary);
    }
}

void ASBCharacterBase::Input_WeaponSlot2(const FInputActionValue& Value)
{
    if (WeaponComponent)
    {
        WeaponComponent->SwitchToSlot(ESBWeaponSlot::Secondary);
    }
}

void ASBCharacterBase::Input_WeaponSlot3(const FInputActionValue& Value)
{
    if (WeaponComponent)
    {
        WeaponComponent->SwitchToSlot(ESBWeaponSlot::Sidearm);
    }
}

// --- Stance ---

ESBStance ASBCharacterBase::GetCurrentStance() const
{
    if (SBMovementComponent && SBMovementComponent->IsProning())
    {
        return ESBStance::Prone;
    }
    if (bIsCrouched)
    {
        return ESBStance::Crouching;
    }
    return ESBStance::Standing;
}

// --- Camera ---

void ASBCharacterBase::UpdateCameraADS(float DeltaTime)
{
    if (!IsLocallyControlled() || !CameraBoom || !FollowCamera)
    {
        return;
    }

    float TargetBoomLength = bIsAiming ? ADSBoomLength : DefaultBoomLength;
    float TargetFOV = bIsAiming ? ADSFOV : DefaultFOV;
    FVector TargetSocketOffset = bIsAiming ? ADSCameraOffset : CameraSocketOffset;

    CurrentBoomLength = FMath::FInterpTo(CurrentBoomLength, TargetBoomLength, DeltaTime, ADSTransitionSpeed);
    CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, ADSTransitionSpeed);

    CameraBoom->TargetArmLength = CurrentBoomLength;
    CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, ADSTransitionSpeed);
    FollowCamera->SetFieldOfView(CurrentFOV);

    // When aiming, face camera direction
    if (SBMovementComponent)
    {
        SBMovementComponent->bOrientRotationToMovement = !bIsAiming;
        SBMovementComponent->bUseControllerDesiredRotation = bIsAiming;
    }
}
