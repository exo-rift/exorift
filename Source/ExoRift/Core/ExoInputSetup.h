#pragma once

#include "CoreMinimal.h"

class UInputAction;
class UInputMappingContext;

/**
 * Programmatic input setup — creates all input actions and key bindings
 * in C++ so the game runs without editor .uasset files.
 */
struct EXORIFT_API FExoInputSetup
{
	// Movement
	UInputAction* Move = nullptr;
	UInputAction* Look = nullptr;
	UInputAction* Jump = nullptr;
	UInputAction* Sprint = nullptr;
	UInputAction* Crouch = nullptr;

	// Combat
	UInputAction* Fire = nullptr;
	UInputAction* AimDownSight = nullptr;
	UInputAction* FireMode = nullptr;
	UInputAction* SwapWeapon = nullptr;
	UInputAction* DropWeapon = nullptr;
	UInputAction* Grenade = nullptr;
	UInputAction* Melee = nullptr;
	UInputAction* ScrollWeapon = nullptr;

	// Interaction
	UInputAction* Interact = nullptr;
	UInputAction* Ping = nullptr;

	// Abilities
	UInputAction* Ability1 = nullptr;
	UInputAction* Ability2 = nullptr;
	UInputAction* Ability3 = nullptr;
	UInputAction* Ability4 = nullptr;

	// Menu / UI
	UInputAction* Pause = nullptr;
	UInputAction* MenuUp = nullptr;
	UInputAction* MenuDown = nullptr;
	UInputAction* MenuLeft = nullptr;
	UInputAction* MenuRight = nullptr;

	// Social
	UInputAction* Comms = nullptr;
	UInputAction* Emote1 = nullptr;
	UInputAction* Emote2 = nullptr;
	UInputAction* Emote3 = nullptr;
	UInputAction* Emote4 = nullptr;

	// Endgame
	UInputAction* Restart = nullptr;
	UInputAction* ReturnToMenu = nullptr;

	/** Complete mapping context with all key bindings. */
	UInputMappingContext* MappingContext = nullptr;

	/** Singleton — creates everything on first call. */
	static FExoInputSetup& Get();

private:
	void Initialize();
	bool bInitialized = false;
};
