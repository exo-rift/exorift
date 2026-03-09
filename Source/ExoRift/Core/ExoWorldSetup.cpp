#include "Core/ExoWorldSetup.h"
#include "Map/ExoZoneSystem.h"
#include "Map/ExoZoneVisualizer.h"
#include "Map/ExoLootSpawner.h"
#include "Map/ExoSpawnPoint.h"
#include "Map/ExoWorldBuilder.h"
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

	// === Build the 3D world (terrain, buildings, POIs, lighting) ===
	AExoWorldBuilder* WorldBuilder = nullptr;
	for (TActorIterator<AExoWorldBuilder> It(World); It; ++It)
	{
		WorldBuilder = *It;
		break;
	}
	if (!WorldBuilder)
	{
		FActorSpawnParameters Params;
		WorldBuilder = World->SpawnActor<AExoWorldBuilder>(
			AExoWorldBuilder::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
	}
	if (WorldBuilder && !WorldBuilder->IsWorldBuilt())
	{
		WorldBuilder->BuildWorld();
	}

	// === Core BR systems ===

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

	// Loot spawner — scatters weapon pickups across the terrain
	EnsureActorExists(World, AExoLootSpawner::StaticClass());

	// Vehicle spawner
	EnsureActorExists(World, AExoVehicleSpawner::StaticClass());

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
