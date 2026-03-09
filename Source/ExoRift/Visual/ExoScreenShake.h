#pragma once

#include "CoreMinimal.h"

/**
 * Simple screen shake system. Accumulates shakes and applies to camera.
 * Call Tick() each frame, then GetShakeOffset() for the current offset.
 */
class FExoScreenShake
{
public:
	/** Add a shake event. */
	static void AddShake(float Intensity, float Duration);

	/** Add shake from an explosion at a world position. Falls off with distance. */
	static void AddExplosionShake(const FVector& ExplosionLoc, const FVector& PlayerLoc,
		float Radius, float MaxIntensity);

	/** Tick all active shakes. */
	static void Tick(float DeltaTime);

	/** Get current accumulated camera offset (applied by controller or HUD). */
	static FRotator GetShakeOffset();

	/** True if any shake is active. */
	static bool IsShaking();

private:
	struct FShakeInstance
	{
		float Intensity;
		float Duration;
		float Age = 0.f;
	};

	static TArray<FShakeInstance> ActiveShakes;
	static FRotator CurrentOffset;
};
