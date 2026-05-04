// ЦВЕТ-ШУМ — Phase 1 default GameMode.
// BP child (BP_GameMode_Default) переопределяет DefaultPawnClass на BP_Player —
// см. docs/dev/bp_player_spec.md §4.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NoiseGameModeBase.generated.h"

UCLASS()
class NOISE_API ANoiseGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANoiseGameModeBase();
};
