#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ExoMenuGameMode.generated.h"

class AExoMenuHUD;
class AExoMenuPlayerController;

/**
 * Game mode for the main menu level.
 * Spawns a spectator pawn (no combat) and sets up the menu HUD + controller.
 */
UCLASS()
class EXORIFT_API AExoMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AExoMenuGameMode();
};
