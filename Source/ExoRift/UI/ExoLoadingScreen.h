#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;

/**
 * Plain C++ loading screen drawer. Renders a black background with pulsing
 * progress bar, ExoRift logo text, and random gameplay tips on Canvas.
 */
class FExoLoadingScreen
{
public:
	FExoLoadingScreen();

	/** Call once to pick a random tip and reset animation state. */
	void Activate();

	/** Advance pulsing animations. */
	void Tick(float DeltaTime);

	/** Draw the full loading screen onto the HUD canvas. */
	void Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font);

private:
	void DrawLogo(AHUD* HUD, UCanvas* Canvas, UFont* Font);
	void DrawProgressBar(AHUD* HUD, UCanvas* Canvas);
	void DrawTip(AHUD* HUD, UCanvas* Canvas, UFont* Font);
	void DrawScanLines(AHUD* HUD, UCanvas* Canvas);
	void DrawCornerBrackets(AHUD* HUD, UCanvas* Canvas);
	void DrawHexOverlay(AHUD* HUD, UCanvas* Canvas);
	void DrawLoadingSpinner(AHUD* HUD, UCanvas* Canvas);

	float ElapsedTime = 0.f;
	int32 TipIndex = 0;

	static const FString Tips[];
	static const int32 TipCount;
};
