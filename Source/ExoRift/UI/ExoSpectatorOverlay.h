#pragma once

#include "CoreMinimal.h"

class UCanvas;
class UFont;
class AExoSpectatorPawn;

/**
 * Draws the spectator-mode HUD overlay on Canvas:
 * spectated player name, health/shield bars, controls hint,
 * eliminated badge, and a subtle vignette.
 */
struct FExoSpectatorOverlay
{
	static void Draw(UCanvas* Canvas, UFont* Font, AExoSpectatorPawn* Spectator);
};
