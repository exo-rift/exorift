#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;

/**
 * Full-screen tactical map overlay (M key).
 * Shows named locations, zone circle, and player position.
 */
struct FExoTacticalMap
{
	static bool bIsOpen;

	/** Draw the tactical map overlay. */
	static void Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font,
		const FVector& PlayerPos, float ZoneRadius, const FVector& ZoneCenter,
		float NextZoneRadius, const FVector& NextZoneCenter);

private:
	/** Convert world position to map screen position. */
	static FVector2D WorldToMap(const FVector& WorldPos, const FVector2D& MapCenter,
		float MapRadius, float WorldRadius);
};
