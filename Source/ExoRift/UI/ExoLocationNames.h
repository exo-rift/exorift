#pragma once

#include "CoreMinimal.h"

class UCanvas;
class UFont;
class AHUD;

/** Named map locations — shows location name when entering a new area. */
struct FExoLocationNames
{
	/** Call each frame from HUD to update and draw location banner. */
	static void Tick(float DeltaTime, const FVector& PlayerLocation);
	static void Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font);

	/** Get name of the location at a world position (empty if wilderness). */
	static FString GetLocationAt(const FVector& WorldPos);

private:
	struct FNamedRegion
	{
		FString Name;
		FVector Center;
		float Radius;
	};
	static const TArray<FNamedRegion>& GetRegions();

	static FString CurrentLocation;
	static FString DisplayLocation;
	static float TransitionAlpha;
	static float DisplayTimer;
	static bool bShowingBanner;
};
