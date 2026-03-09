#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ExoPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
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
	void HandlePing();

	// Ability input handlers
	void HandleAbility1();
	void HandleAbility2();
	void HandleAbility3();

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
	UInputAction* PingAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* Ability1Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* Ability2Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* Ability3Action;

private:
	void SetupEnhancedInput();

	/** True once OnCharacterDied has been called — prevents double-transition. */
	bool bIsSpectating = false;
};
