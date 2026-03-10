#pragma once

#include "CoreMinimal.h"
#include "Core/ExoTypes.h"

/**
 * Static helper for spawning actor-based weapon VFX:
 * tracers, muzzle flashes, bullet impacts, and explosions.
 */
class FExoTracerManager
{
public:
	/** Spawn a weapon-colored tracer beam from Start to End. */
	static void SpawnTracer(UWorld* World, const FVector& Start, const FVector& End,
		bool bIsHit, EWeaponType WeaponType = EWeaponType::Rifle);

	/** Spawn a weapon-colored muzzle flash at the given location. */
	static void SpawnMuzzleFlash(UWorld* World, const FVector& Location,
		const FRotator& Rotation, EWeaponType WeaponType = EWeaponType::Rifle);

	/** Spawn impact sparks at a bullet hit location. */
	static void SpawnImpactEffect(UWorld* World, const FVector& Location,
		const FVector& HitNormal, bool bHitCharacter);

	/** Spawn an ejected shell casing from weapon fire. */
	static void SpawnShellCasing(UWorld* World, const FVector& Location,
		const FVector& EjectDir, EWeaponType WeaponType);

	/** Spawn an explosion effect (grenade / projectile). */
	static void SpawnExplosionEffect(UWorld* World, const FVector& Location, float Radius);

	/** Get the signature energy color for each weapon type. */
	static FLinearColor GetWeaponTracerColor(EWeaponType Type);
};
