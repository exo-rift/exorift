#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;

/**
 * Static helper that draws the animated main-menu background,
 * scan-line overlay, and title text. Purely Canvas-based, no UObject.
 */
class FExoMenuBackground
{
public:
	/** Dark pulsing gradient with blue glow and vignette. */
	static void DrawBackground(AHUD* HUD, UCanvas* Canvas, float Time);

	/** Subtle horizontal scan lines + sweeping bright line. */
	static void DrawScanLines(AHUD* HUD, UCanvas* Canvas, float Time);

	/** Floating particle dots for sci-fi ambience. */
	static void DrawParticles(AHUD* HUD, UCanvas* Canvas, float Time);

	/** Subtle hex grid lines for tech aesthetic. */
	static void DrawHexGrid(AHUD* HUD, UCanvas* Canvas, float Time);

	/** "EXORIFT" title, accent lines, and "BATTLE ROYALE" subtitle. */
	static void DrawTitle(AHUD* HUD, UCanvas* Canvas, UFont* Font, float Time);
};
