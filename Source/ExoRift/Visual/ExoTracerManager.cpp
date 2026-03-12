#include "Visual/ExoTracerManager.h"
#include "Visual/ExoTracer.h"
#include "Visual/ExoTracerWake.h"
#include "Visual/ExoEnergyRibbon.h"
#include "Visual/ExoMuzzleFlash.h"
#include "Visual/ExoMuzzleSmoke.h"
#include "Visual/ExoMuzzleSparks.h"
#include "Visual/ExoImpactEffect.h"
#include "Visual/ExoImpactDecal.h"
#include "Visual/ExoExplosionEffect.h"
#include "Visual/ExoShellCasing.h"

void FExoTracerManager::SpawnTracer(UWorld* World, const FVector& Start, const FVector& End,
	bool bIsHit, EWeaponType WeaponType)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoTracer* Tracer = World->SpawnActor<AExoTracer>(AExoTracer::StaticClass(),
		FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (Tracer)
	{
		Tracer->InitTracer(Start, End, bIsHit, GetWeaponTracerColor(WeaponType), WeaponType);
	}

	// Energy droplets left in the tracer's wake
	FLinearColor WakeColor = GetWeaponTracerColor(WeaponType);
	float WakeSpacing = (WeaponType == EWeaponType::Sniper) ? 200.f :
		(WeaponType == EWeaponType::Shotgun) ? 180.f : 300.f;
	AExoTracerWake::SpawnWake(World, Start, End, WakeColor, WakeSpacing);

	// Persistent energy ribbon contrail — lingers 1.5s after the bolt passes
	float RibbonThickness = (WeaponType == EWeaponType::Sniper) ? 1.6f :
		(WeaponType == EWeaponType::Shotgun) ? 1.4f :
		(WeaponType == EWeaponType::SMG) ? 0.8f : 1.f;
	AExoEnergyRibbon::SpawnRibbon(World, Start, End, WakeColor, RibbonThickness);
}

void FExoTracerManager::SpawnMuzzleFlash(UWorld* World, const FVector& Location,
	const FRotator& Rotation, EWeaponType WeaponType)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoMuzzleFlash* Flash = World->SpawnActor<AExoMuzzleFlash>(AExoMuzzleFlash::StaticClass(),
		Location, FRotator::ZeroRotator, Params);
	if (Flash)
	{
		Flash->InitFlash(Rotation, GetWeaponTracerColor(WeaponType), WeaponType);
	}

	// Lingering smoke wisps at the muzzle
	AExoMuzzleSmoke::SpawnSmoke(World, Location, Rotation);

	// Energy discharge sparks from barrel
	AExoMuzzleSparks::SpawnSparks(World, Location, Rotation,
		GetWeaponTracerColor(WeaponType), WeaponType);
}

void FExoTracerManager::SpawnImpactEffect(UWorld* World, const FVector& Location,
	const FVector& HitNormal, bool bHitCharacter, EWeaponType WeaponType)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoImpactEffect* Impact = World->SpawnActor<AExoImpactEffect>(AExoImpactEffect::StaticClass(),
		Location, FRotator::ZeroRotator, Params);
	if (Impact)
	{
		Impact->InitEffect(HitNormal, bHitCharacter, GetWeaponTracerColor(WeaponType));
	}

	// Leave a scorch mark on surfaces (not on characters)
	if (!bHitCharacter)
	{
		AExoImpactDecal::SpawnDecal(World, Location, HitNormal,
			GetWeaponTracerColor(WeaponType));
	}
}

void FExoTracerManager::SpawnShellCasing(UWorld* World, const FVector& Location,
	const FVector& EjectDir, EWeaponType WeaponType)
{
	if (!World) return;
	// Melee and launcher don't eject casings
	if (WeaponType == EWeaponType::Melee || WeaponType == EWeaponType::GrenadeLauncher) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoShellCasing* Casing = World->SpawnActor<AExoShellCasing>(
		AExoShellCasing::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Casing)
	{
		Casing->InitCasing(EjectDir, GetWeaponTracerColor(WeaponType));
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

FLinearColor FExoTracerManager::GetWeaponTracerColor(EWeaponType Type)
{
	switch (Type)
	{
	case EWeaponType::Rifle:  return FLinearColor(0.2f, 0.65f, 1.f);    // Deep cyan energy
	case EWeaponType::SMG:    return FLinearColor(0.1f, 1.f, 0.3f);     // Vivid green plasma
	case EWeaponType::Pistol: return FLinearColor(1.f, 0.85f, 0.3f);    // Hot amber arc
	case EWeaponType::Shotgun: return FLinearColor(1.f, 0.3f, 0.05f);   // Intense orange scatter
	case EWeaponType::Sniper: return FLinearColor(0.6f, 0.1f, 1.f);     // Deep violet void
	default:                  return FLinearColor(0.2f, 0.65f, 1.f);
	}
}
