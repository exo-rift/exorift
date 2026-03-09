#pragma once

#include "CoreMinimal.h"

/**
 * Static helper for spawning actor-based weapon VFX:
 * tracers, muzzle flashes, bullet impacts, and explosions.
 */
class FExoTracerManager
{
public:
	/** Spawn a glowing tracer beam from Start to End. */
	static void SpawnTracer(UWorld* World, const FVector& Start, const FVector& End, bool bIsHit);

	/** Spawn a brief muzzle flash at the given location. */
	static void SpawnMuzzleFlash(UWorld* World, const FVector& Location, const FRotator& Rotation);

	/** Spawn impact sparks at a bullet hit location. */
	static void SpawnImpactEffect(UWorld* World, const FVector& Location, const FVector& HitNormal, bool bHitCharacter);

	/** Spawn an explosion effect (grenade / projectile). */
	static void SpawnExplosionEffect(UWorld* World, const FVector& Location, float Radius);
};
