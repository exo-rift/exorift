#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ExoMenuPlayerController.generated.h"

class AExoMenuHUD;

/**
 * Player controller for the main menu.
 * Shows cursor and routes keyboard input to the menu HUD via SetupInputComponent.
 */
UCLASS()
class EXORIFT_API AExoMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AExoMenuPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void HandleMenuUp();
	void HandleMenuDown();
	void HandleMenuSelect();
	void HandleMenuBack();

	AExoMenuHUD* GetMenuHUD() const;
};
