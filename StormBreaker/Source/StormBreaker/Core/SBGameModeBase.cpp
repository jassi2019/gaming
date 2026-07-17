// Copyright Island Of Death Games. All Rights Reserved.

#include "Core/SBGameModeBase.h"
#include "Core/SBPlayerController.h"

ASBGameModeBase::ASBGameModeBase()
{
    PlayerControllerClass = ASBPlayerController::StaticClass();
}
