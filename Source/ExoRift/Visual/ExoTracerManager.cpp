#include "Visual/ExoTracerManager.h"
#include "Visual/ExoTracer.h"
#include "Visual/ExoMuzzleFlash.h"
#include "Visual/ExoImpactEffect.h"
#include "Visual/ExoExplosionEffect.h"

void FExoTracerManager::SpawnTracer(UWorld* World, const FVector& Start, const FVector& End, bool bIsHit)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoTracer* Tracer = World->SpawnActor<AExoTracer>(AExoTracer::StaticClass(),
		FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (Tracer)
	{
		Tracer->InitTracer(Start, End, bIsHit);
	}
}

void FExoTracerManager::SpawnMuzzleFlash(UWorld* World, const FVector& Location, const FRotator& Rotation)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoMuzzleFlash* Flash = World->SpawnActor<AExoMuzzleFlash>(AExoMuzzleFlash::StaticClass(),
		Location, FRotator::ZeroRotator, Params);
	if (Flash)
	{
		Flash->InitFlash(Rotation);
	}
}

void FExoTracerManager::SpawnImpactEffect(UWorld* World, const FVector& Location,
	const FVector& HitNormal, bool bHitCharacter)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoImpactEffect* Impact = World->SpawnActor<AExoImpactEffect>(AExoImpactEffect::StaticClass(),
		Location, FRotator::ZeroRotator, Params);
	if (Impact)
	{
		Impact->InitEffect(HitNormal, bHitCharacter);
	}
}

void FExoTracerManager::SpawnExplosionEffect(UWorld* World, const FVector& Location, float Radius)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoExplosionEffect* Explosion = World->SpawnActor<AExoExplosionEffect>(
		AExoExplosionEffect::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Explosion)
	{
		Explosion->InitExplosion(Radius);
	}
}
