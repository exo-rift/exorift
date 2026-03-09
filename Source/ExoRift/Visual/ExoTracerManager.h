#pragma once

#include "CoreMinimal.h"

/**
 * Static helper for spawning bullet tracers and muzzle flash effects.
 * Plain C++ class — not a UObject. Uses debug draw as placeholder.
 */
class FExoTracerManager
{
public:
	/** Spawn a tracer line from Start to End. Cyan for miss, red-tinted for hit. */
	static void SpawnTracer(UWorld* World, const FVector& Start, const FVector& End, bool bIsHit);

	/** Spawn a brief muzzle flash (debug point placeholder) at the given location. */
	static void SpawnMuzzleFlash(UWorld* World, const FVector& Location, const FRotator& Rotation);
};
