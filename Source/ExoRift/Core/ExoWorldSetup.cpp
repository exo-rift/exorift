#include "Core/ExoWorldSetup.h"
#include "Map/ExoZoneSystem.h"
#include "Map/ExoZoneVisualizer.h"
#include "Map/ExoLootSpawner.h"
#include "Map/ExoSpawnPoint.h"
#include "Map/ExoFloatingDust.h"
#include "Map/ExoEnvironmentAnimator.h"
#include "Visual/ExoWeatherSystem.h"
#include "Visual/ExoPostProcess.h"
#include "UI/ExoDamageNumbers.h"
#include "Vehicles/ExoVehicleSpawner.h"
#include "EngineUtils.h"
#include "ExoRift.h"

void UExoWorldSetup::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UWorld* World = &InWorld;
	if (!World) return;

	UE_LOG(LogExoRift, Log, TEXT("ExoWorldSetup: ensuring BR systems exist..."));

	// Zone system — the shrinking circle
	EnsureActorExists(World, AExoZoneSystem::StaticClass());

	// Zone visualizer
	EnsureActorExists(World, AExoZoneVisualizer::StaticClass());

	// Post-process singleton
	EnsureActorExists(World, AExoPostProcess::StaticClass());

	// Damage numbers singleton
	EnsureActorExists(World, AExoDamageNumbers::StaticClass());

	// Weather system
	EnsureActorExists(World, AExoWeatherSystem::StaticClass());

	// Loot spawner — scatters weapon pickups
	EnsureActorExists(World, AExoLootSpawner::StaticClass());

	// Vehicle spawner
	EnsureActorExists(World, AExoVehicleSpawner::StaticClass());

	// Floating atmospheric dust
	EnsureActorExists(World, AExoFloatingDust::StaticClass());

	// Environment animator (neon flicker, spotlight sweep, ambient pulse)
	EnsureActorExists(World, AExoEnvironmentAnimator::StaticClass());

	// Spawn points — create a ring of them if none exist
	bool bHasSpawnPoints = false;
	for (TActorIterator<AExoSpawnPoint> It(World); It; ++It)
	{
		bHasSpawnPoints = true;
		break;
	}

	if (!bHasSpawnPoints)
	{
		UE_LOG(LogExoRift, Log, TEXT("ExoWorldSetup: creating default spawn points"));
		const int32 NumSpawns = 25;
		const float Radius = 5000.f;
		for (int32 i = 0; i < NumSpawns; i++)
		{
			float Angle = (2.f * PI * i) / NumSpawns;
			FVector Loc(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 300.f);
			FActorSpawnParameters Params;
			World->SpawnActor<AExoSpawnPoint>(AExoSpawnPoint::StaticClass(), Loc, FRotator::ZeroRotator, Params);
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("ExoWorldSetup: all BR systems ready"));
}

void UExoWorldSetup::EnsureActorExists(UWorld* World, UClass* ActorClass, const FVector& Location)
{
	// Check if one already exists
	for (TActorIterator<AActor> It(World, ActorClass); It; ++It)
	{
		return; // Already exists
	}

	// Spawn it
	FActorSpawnParameters Params;
	AActor* Spawned = World->SpawnActor(ActorClass, &Location, nullptr, Params);
	if (Spawned)
	{
		UE_LOG(LogExoRift, Log, TEXT("  Spawned: %s"), *ActorClass->GetName());
	}
}
