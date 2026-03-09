#pragma once

#include "CoreMinimal.h"

class UCanvas;

/**
 * Draws zone boundary warning effects on the HUD canvas.
 * Shows pulsing warnings, directional arrows, and countdown text
 * when the player approaches or exits the zone boundary.
 */
class EXORIFT_API FExoZoneEffects
{
public:
	/**
	 * Draw zone proximity warnings on the HUD.
	 * @param Canvas          The HUD canvas to draw on.
	 * @param ZoneScreenCenter  Zone center projected to screen space.
	 * @param ZoneScreenRadius  Zone radius in screen-space pixels.
	 * @param PlayerScreenPos   Player position in screen space.
	 * @param DistToEdge        Signed distance from player to zone edge (negative = outside).
	 * @param bZoneShrinking    Whether the zone is currently shrinking.
	 * @param ShrinkCountdown   Seconds remaining until next shrink phase.
	 */
	static void Draw(UCanvas* Canvas, FVector2D ZoneScreenCenter,
		float ZoneScreenRadius, FVector2D PlayerScreenPos,
		float DistToEdge, bool bZoneShrinking, float ShrinkCountdown);

private:
	/** Pulsing red edge warning when near boundary. */
	static void DrawEdgeWarning(UCanvas* Canvas, float Proximity01);

	/** Arrow pointing toward zone center when player is outside. */
	static void DrawDirectionalArrow(UCanvas* Canvas, FVector2D PlayerScreenPos,
		FVector2D ZoneScreenCenter);

	/** "ZONE CLOSING" text with countdown. */
	static void DrawShrinkCountdown(UCanvas* Canvas, float SecondsRemaining);
};
