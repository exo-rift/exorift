#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ExoPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AExoSpectatorPawn;
class UExoGameSettings;
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

	// Character input handlers
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleJump();
	void HandleJumpReleased();
	void HandleFire();
	void HandleFireReleased();
	void HandleSwapWeapon();
	void HandleSprint();
	void HandleSprintReleased();
	void HandleCrouch();
	void HandleCrouchReleased();
	void HandleInteract();
	void HandleDropWeapon();
	void HandlePing();

	// Ability input handlers
	void HandleAbility1();
	void HandleAbility2();
	void HandleAbility3();

	// Settings menu input handlers
	void HandlePause();
	void HandleMenuUp();
	void HandleMenuDown();
	void HandleMenuLeft();
	void HandleMenuRight();

	// Comms wheel
	void HandleCommsOpen();
	void HandleCommsClose();

	// Spectator cycling
	void HandleSpectateNext();
	void HandleSpectatePrev();

	// Input assets
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* FireAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SwapWeaponAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* DropWeaponAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* PingAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* Ability1Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* Ability2Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* Ability3Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* PauseAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MenuUpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MenuDownAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MenuLeftAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MenuRightAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* CommsAction;

private:
	void SetupEnhancedInput();

	/** True once OnCharacterDied has been called — prevents double-transition. */
	bool bIsSpectating = false;
};
