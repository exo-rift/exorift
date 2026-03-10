#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ExoPlayerController.generated.h"

class AExoSpectatorPawn;
struct FInputActionValue;

UCLASS()
class EXORIFT_API AExoPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AExoPlayerController();

	/** Called by AExoCharacter::Die to transition into spectator mode. */
	void OnCharacterDied(AController* Killer);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	// Movement
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleJump();
	void HandleJumpReleased();
	void HandleSprint();
	void HandleSprintReleased();
	void HandleCrouch();
	void HandleCrouchReleased();

	// Combat
	void HandleFire();
	void HandleFireReleased();
	void HandleADS();
	void HandleADSReleased();
	void HandleFireMode();
	void HandleSwapWeapon();
	void HandleDropWeapon();
	void HandleGrenade();
	void HandleMelee();
	void HandleInspect();
	void HandleInspectReleased();
	void HandleScrollWeapon(const FInputActionValue& Value);

	// Interaction
	void HandleInteract();
	void HandlePing();

	// Abilities
	void HandleAbility1();
	void HandleAbility2();
	void HandleAbility3();
	void HandleAbility4();

	// Menu
	void HandleTacticalMap();
	void HandlePause();
	void HandleMenuUp();
	void HandleMenuDown();
	void HandleMenuLeft();
	void HandleMenuRight();

	// Social
	void HandleEmote1();
	void HandleEmote2();
	void HandleEmote3();
	void HandleEmote4();
	void PlayEmoteSlot(int32 SlotIndex);
	void HandleCommsOpen();
	void HandleCommsClose();

	// Spectator
	void HandleSpectateNext();
	void HandleSpectatePrev();

	// Endgame
	void HandleRestartMatch();
	void HandleReturnToMenu();

private:
	void SetupEnhancedInput();
	bool bIsSpectating = false;
};
