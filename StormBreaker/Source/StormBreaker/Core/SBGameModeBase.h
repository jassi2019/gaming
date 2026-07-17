// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SBGameModeBase.generated.h"

/**
 * Base game mode — lobby, menus, non-gameplay maps.
 * Subclassed by SBBattleRoyaleGameMode for match logic.
 */
UCLASS()
class STORMBREAKER_API ASBGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASBGameModeBase();
};
