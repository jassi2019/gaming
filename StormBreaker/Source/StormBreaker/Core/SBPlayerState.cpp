// Copyright StormBreaker Games. All Rights Reserved.

#include "Core/SBPlayerState.h"
#include "StormBreaker.h"
#include "AbilitySystemComponent.h"
#include "Core/SBAttributeSet.h"
#include "Net/UnrealNetwork.h"

ASBPlayerState::ASBPlayerState()
    : Kills(0)
    , Assists(0)
    , DamageDealt(0)
    , PlayerStatus(ESBPlayerStatus::Alive)
    , TeamId(-1)
{
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    AttributeSet = CreateDefaultSubobject<USBAttributeSet>(TEXT("AttributeSet"));

    // Net update frequency for battle royale
    NetUpdateFrequency = 20.0f;
}

UAbilitySystemComponent* ASBPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ASBPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASBPlayerState, Kills);
    DOREPLIFETIME(ASBPlayerState, Assists);
    DOREPLIFETIME(ASBPlayerState, DamageDealt);
    DOREPLIFETIME(ASBPlayerState, PlayerStatus);
    DOREPLIFETIME(ASBPlayerState, TeamId);
}

void ASBPlayerState::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
    }
}

void ASBPlayerState::AddKill()
{
    if (HasAuthority())
    {
        Kills++;
    }
}

void ASBPlayerState::AddAssist()
{
    if (HasAuthority())
    {
        Assists++;
    }
}

void ASBPlayerState::AddDamage(int32 Amount)
{
    if (HasAuthority())
    {
        DamageDealt += Amount;
    }
}

void ASBPlayerState::SetPlayerStatus(ESBPlayerStatus NewStatus)
{
    if (HasAuthority())
    {
        PlayerStatus = NewStatus;
    }
}

void ASBPlayerState::SetTeamId(int32 InTeamId)
{
    if (HasAuthority())
    {
        TeamId = InTeamId;
    }
}
