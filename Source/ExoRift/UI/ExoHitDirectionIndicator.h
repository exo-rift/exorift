#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;

/** Draws directional damage indicators (red arrows pointing at damage source). */
class FExoHitDirectionIndicator
{
public:
	/** Register a hit from a given world location. */
	static void AddHit(const FVector& SourceLocation);

	/** Draw all active indicators. Called from HUD. */
	static void Draw(AHUD* HUD, UCanvas* Canvas);

	/** Tick fade timers. Call once per frame. */
	static void Tick(float DeltaTime);

private:
	struct FHitIndicator
	{
		FVector SourceWorldLocation;
		float Age = 0.f;
		float Lifetime = 1.5f;
	};

	static TArray<FHitIndicator> ActiveIndicators;
	static constexpr int32 MaxIndicators = 6;
};
