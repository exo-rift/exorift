#include "Core/ExoMenuGameMode.h"
#include "UI/ExoMenuHUD.h"
#include "UI/ExoMenuPlayerController.h"
#include "GameFramework/SpectatorPawn.h"

AExoMenuGameMode::AExoMenuGameMode()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultPawnClass = ASpectatorPawn::StaticClass();
	PlayerControllerClass = AExoMenuPlayerController::StaticClass();
	HUDClass = AExoMenuHUD::StaticClass();
}
