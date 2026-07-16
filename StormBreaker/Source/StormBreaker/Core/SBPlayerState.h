// Copyright StormBreaker Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "SBPlayerState.generated.h"

class UAbilitySystemComponent;
class USBAttributeSet;

UENUM(BlueprintType)
enum class ESBPlayerStatus : uint8
{
    Alive,
    Downed,
    Dead,
    Spectating
};

/**
 * Per-player replicated state.
 * Owns the Ability System Component for GAS integration.
 */
UCLASS()
class STORMBREAKER_API ASBPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ASBPlayerState();

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ----- Stats -----

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Stats")
    int32 GetKills() const { return Kills; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Stats")
    int32 GetAssists() const { return Assists; }

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Stats")
    int32 GetDamageDealt() const { return DamageDealt; }

    void AddKill();
    void AddAssist();
    void AddDamage(int32 Amount);

    // ----- Status -----

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Status")
    ESBPlayerStatus GetPlayerStatus() const { return PlayerStatus; }

    void SetPlayerStatus(ESBPlayerStatus NewStatus);

    // ----- Team -----

    UFUNCTION(BlueprintPure, Category = "StormBreaker|Team")
    int32 GetTeamId() const { return TeamId; }

    void SetTeamId(int32 InTeamId);

    // ----- GAS -----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Abilities")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StormBreaker|Abilities")
    TObjectPtr<USBAttributeSet> AttributeSet;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(Replicated)
    int32 Kills;

    UPROPERTY(Replicated)
    int32 Assists;

    UPROPERTY(Replicated)
    int32 DamageDealt;

    UPROPERTY(Replicated)
    ESBPlayerStatus PlayerStatus;

    UPROPERTY(Replicated)
    int32 TeamId;
};
