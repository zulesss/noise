// ЦВЕТ-ШУМ — Phase 1 default GameMode.

#include "NoiseGameModeBase.h"

#include "GameFramework/PlayerController.h"
#include "Player/NoiseCharacter.h"

ANoiseGameModeBase::ANoiseGameModeBase()
{
	DefaultPawnClass = ANoiseCharacter::StaticClass();
	PlayerControllerClass = APlayerController::StaticClass();
}
