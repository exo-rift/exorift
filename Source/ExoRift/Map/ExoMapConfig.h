#pragma once

#include "CoreMinimal.h"
#include "ExoMapConfig.generated.h"

/**
 * Central map path references for ExoRift.
 * Config=Game lets designers override paths in DefaultGame.ini if needed.
 */
UCLASS(Config = Game, DefaultConfig)
class EXORIFT_API UExoMapConfig : public UObject
{
	GENERATED_BODY()

public:
	/** Display name shown in UI for the BR map. */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Maps")
	FString BRMapDisplayName = TEXT("First Light");

	/** Display name for the main menu map. */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Maps")
	FString MenuMapDisplayName = TEXT("Main Menu");

	/** Content path to the gameplay map (the template FirstPerson map for now). */
	static FString GetBRMapPath() { return TEXT("/Game/FirstPerson/Lvl_FirstPerson"); }

	/** Content path to the main menu map (shares the gameplay level; game mode override selects menu behavior). */
	static FString GetMenuMapPath() { return TEXT("/Game/FirstPerson/Lvl_FirstPerson"); }
};
