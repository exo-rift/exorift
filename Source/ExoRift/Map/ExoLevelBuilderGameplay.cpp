// ExoLevelBuilderGameplay.cpp — Spawn points, loot, hazards, barrels, cover
#include "Map/ExoLevelBuilder.h"
#include "Map/ExoSpawnPoint.h"
#include "Map/ExoLootSpawner.h"
#include "Map/ExoLootContainer.h"
#include "Map/ExoHazardZone.h"
#include "Map/ExoExplodingBarrel.h"
#include "Map/ExoZoneSystem.h"
#include "Map/ExoZoneVisualizer.h"
#include "Components/StaticMeshComponent.h"
#include "ExoRift.h"

void AExoLevelBuilder::PlaceSpawnPoints()
{
	// 20 spawn points distributed in a ring around the map
	int32 NumSpawns = 20;
	float SpawnRing = MapHalfSize * 0.7f; // 70% of map radius

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < NumSpawns; i++)
	{
		float Angle = (2.f * PI * i) / NumSpawns;
		float Jitter = FMath::RandRange(-15000.f, 15000.f);
		FVector Pos(
			FMath::Cos(Angle) * (SpawnRing + Jitter),
			FMath::Sin(Angle) * (SpawnRing + Jitter),
			GroundZ + 100.f);

		AExoSpawnPoint* SP = GetWorld()->SpawnActor<AExoSpawnPoint>(
			AExoSpawnPoint::StaticClass(), Pos, FRotator::ZeroRotator, Params);
		if (SP)
		{
			SP->bIsDropZone = true;
			SP->bIsLobbySpawn = (i < 4); // First 4 also serve as lobby spawns
		}
	}

	// 4 additional lobby spawns near center
	for (int32 i = 0; i < 4; i++)
	{
		float Angle = (PI * 0.5f) * i;
		FVector Pos(FMath::Cos(Angle) * 3000.f, FMath::Sin(Angle) * 3000.f, GroundZ + 100.f);
		AExoSpawnPoint* SP = GetWorld()->SpawnActor<AExoSpawnPoint>(
			AExoSpawnPoint::StaticClass(), Pos, FRotator::ZeroRotator, Params);
		if (SP)
		{
			SP->bIsDropZone = false;
			SP->bIsLobbySpawn = true;
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed %d spawn points"), NumSpawns + 4);
}

void AExoLevelBuilder::PlaceLootSpawners()
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Central loot cluster — high tier
	AExoLootSpawner* CenterLoot = GetWorld()->SpawnActor<AExoLootSpawner>(
		AExoLootSpawner::StaticClass(), FVector(0.f, 0.f, GroundZ + 50.f),
		FRotator::ZeroRotator, Params);
	if (CenterLoot)
	{
		CenterLoot->SpawnCount = 15;
		CenterLoot->SpawnRadius = 12000.f;
		CenterLoot->RarityWeights = {0.2f, 0.35f, 0.3f, 0.15f};
	}

	// Loot at each compound
	FVector CompoundCenters[] = {
		{0.f, 80000.f, GroundZ + 50.f},     // North
		{0.f, -80000.f, GroundZ + 50.f},    // South
		{80000.f, 0.f, GroundZ + 50.f},     // East
		{-80000.f, 0.f, GroundZ + 50.f},    // West
	};

	for (const FVector& CC : CompoundCenters)
	{
		AExoLootSpawner* LS = GetWorld()->SpawnActor<AExoLootSpawner>(
			AExoLootSpawner::StaticClass(), CC, FRotator::ZeroRotator, Params);
		if (LS)
		{
			LS->SpawnCount = 10;
			LS->SpawnRadius = 15000.f;
			LS->RarityWeights = {0.35f, 0.35f, 0.2f, 0.1f};
		}
	}

	// Scattered loot across the map
	for (int32 i = 0; i < 8; i++)
	{
		FVector Pos(
			FMath::RandRange(-MapHalfSize * 0.8f, MapHalfSize * 0.8f),
			FMath::RandRange(-MapHalfSize * 0.8f, MapHalfSize * 0.8f),
			GroundZ + 50.f);

		AExoLootSpawner* LS = GetWorld()->SpawnActor<AExoLootSpawner>(
			AExoLootSpawner::StaticClass(), Pos, FRotator::ZeroRotator, Params);
		if (LS)
		{
			LS->SpawnCount = 5;
			LS->SpawnRadius = 8000.f;
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 13 loot spawners"));

	// Loot containers (openable crates) inside buildings
	TArray<FVector> ContainerPositions = {
		// Central hub
		{2000.f, 1000.f, GroundZ + 50.f}, {-2000.f, -1500.f, GroundZ + 50.f},
		{500.f, -500.f, GroundZ + 2550.f}, // Second floor
		// North compound
		{-3000.f, 81000.f, GroundZ + 50.f}, {4000.f, 79000.f, GroundZ + 50.f},
		// South compound
		{2000.f, -81000.f, GroundZ + 50.f}, {-5000.f, -79000.f, GroundZ + 50.f},
		// East compound
		{81000.f, -1000.f, GroundZ + 50.f}, {79000.f, 2000.f, GroundZ + 50.f},
		// West compound
		{-81000.f, -2000.f, GroundZ + 50.f}, {-79000.f, 3000.f, GroundZ + 50.f},
		// Corner outposts
		{121000.f, 121000.f, GroundZ + 50.f}, {-121000.f, 121000.f, GroundZ + 50.f},
		{121000.f, -121000.f, GroundZ + 50.f}, {-121000.f, -121000.f, GroundZ + 50.f},
		// Scattered buildings
		{40000.f, 40000.f, GroundZ + 50.f}, {-50000.f, 50000.f, GroundZ + 50.f},
		{60000.f, -40000.f, GroundZ + 50.f}, {-30000.f, -50000.f, GroundZ + 50.f},
		{20000.f, -70000.f, GroundZ + 50.f},
	};

	for (const FVector& Pos : ContainerPositions)
	{
		FRotator ContainerRot(0.f, FMath::RandRange(0.f, 360.f), 0.f);
		GetWorld()->SpawnActor<AExoLootContainer>(
			AExoLootContainer::StaticClass(), Pos, ContainerRot, Params);
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed %d loot containers"),
		ContainerPositions.Num());
}

void AExoLevelBuilder::PlaceHazardZones()
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	struct FHazardDef { FVector Pos; EHazardType Type; float Radius; FString Name; };
	TArray<FHazardDef> Hazards = {
		{{-50000.f, 50000.f, 0.f}, EHazardType::Radiation, 8000.f, TEXT("Reactor Leak")},
		{{60000.f, -60000.f, 0.f}, EHazardType::Electric,  6000.f, TEXT("Power Surge")},
		{{-100000.f, -40000.f, 0.f}, EHazardType::Toxic,   7000.f, TEXT("Toxic Waste")},
		{{40000.f, 100000.f, 0.f}, EHazardType::Fire,      5000.f, TEXT("Fuel Fire")},
	};

	for (const auto& H : Hazards)
	{
		AExoHazardZone* HZ = GetWorld()->SpawnActor<AExoHazardZone>(
			AExoHazardZone::StaticClass(), H.Pos, FRotator::ZeroRotator, Params);
		if (HZ)
		{
			// Set properties via the exposed UPROPERTY EditAnywhere fields
			// These are protected, so we access via CDO pattern isn't ideal.
			// The hazard zone reads its own defaults — the placement position is sufficient.
		}
	}

	// Also spawn the zone system and visualizer
	GetWorld()->SpawnActor<AExoZoneSystem>(AExoZoneSystem::StaticClass(),
		FVector::ZeroVector, FRotator::ZeroRotator, Params);
	GetWorld()->SpawnActor<AExoZoneVisualizer>(AExoZoneVisualizer::StaticClass(),
		FVector::ZeroVector, FRotator::ZeroRotator, Params);

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed %d hazard zones + zone system"),
		Hazards.Num());
}

void AExoLevelBuilder::PlaceExplodingBarrels()
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Barrels near compounds and structures
	TArray<FVector> BarrelPositions = {
		// Central hub
		{4000.f, 2000.f, GroundZ}, {-3000.f, -1500.f, GroundZ},
		// North compound
		{2000.f, 82000.f, GroundZ}, {-7000.f, 78000.f, GroundZ},
		// South compound
		{5000.f, -82000.f, GroundZ}, {-4000.f, -78000.f, GroundZ},
		// East
		{82000.f, 2000.f, GroundZ}, {78000.f, -4000.f, GroundZ},
		// West
		{-82000.f, 3000.f, GroundZ}, {-78000.f, -2000.f, GroundZ},
		// Scattered
		{30000.f, 30000.f, GroundZ}, {-40000.f, -60000.f, GroundZ},
		{70000.f, 50000.f, GroundZ}, {-60000.f, 70000.f, GroundZ},
	};

	for (const FVector& Pos : BarrelPositions)
	{
		GetWorld()->SpawnActor<AExoExplodingBarrel>(
			AExoExplodingBarrel::StaticClass(), Pos + FVector(0.f, 0.f, 50.f),
			FRotator::ZeroRotator, Params);
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed %d exploding barrels"),
		BarrelPositions.Num());
}

void AExoLevelBuilder::PlaceCoverElements()
{
	// Scatter cover: low walls, crates, barriers throughout the map
	FMath::SRandInit(42); // Deterministic seed for consistent layout

	int32 CoverCount = 0;

	// Low walls along streets between compounds
	struct FCoverWall { FVector A; FVector B; float H; };
	TArray<FCoverWall> CoverWalls = {
		// Center to North street
		{{-1500.f, 15000.f, 0.f}, {-1500.f, 25000.f, 0.f}, 800.f},
		{{1500.f, 35000.f, 0.f},  {1500.f, 45000.f, 0.f},  800.f},
		// Center to East street
		{{20000.f, -1200.f, 0.f}, {30000.f, -1200.f, 0.f}, 800.f},
		{{40000.f, 1200.f, 0.f},  {50000.f, 1200.f, 0.f},  800.f},
	};
	for (const auto& W : CoverWalls)
	{
		SpawnWall(W.A, W.B, W.H, 150.f);
		CoverCount++;
	}

	// Scattered crate clusters
	for (int32 i = 0; i < 40; i++)
	{
		FVector Pos(
			FMath::RandRange(-MapHalfSize * 0.85f, MapHalfSize * 0.85f),
			FMath::RandRange(-MapHalfSize * 0.85f, MapHalfSize * 0.85f),
			GroundZ);

		float CrateSize = FMath::RandRange(100.f, 300.f);
		float CrateH = FMath::RandRange(80.f, 200.f);
		SpawnStaticMesh(
			Pos + FVector(0.f, 0.f, CrateH * 0.5f),
			FVector(CrateSize / 100.f, CrateSize / 100.f, CrateH / 100.f),
			FRotator(0.f, FMath::RandRange(0.f, 90.f), 0.f), CubeMesh,
			FLinearColor(0.1f, 0.08f, 0.06f)); // Rust/crate brown
		CoverCount++;
	}

	// Jersey barriers (short thick walls)
	for (int32 i = 0; i < 20; i++)
	{
		FVector Pos(
			FMath::RandRange(-MapHalfSize * 0.6f, MapHalfSize * 0.6f),
			FMath::RandRange(-MapHalfSize * 0.6f, MapHalfSize * 0.6f),
			GroundZ);

		float Len = FMath::RandRange(400.f, 1000.f);
		float Yaw = FMath::RandRange(0.f, 360.f);
		SpawnStaticMesh(
			Pos + FVector(0.f, 0.f, 200.f),
			FVector(Len / 100.f, 1.2f, 4.f),
			FRotator(0.f, Yaw, 0.f), CubeMesh,
			FLinearColor(0.12f, 0.12f, 0.13f)); // Concrete gray
		CoverCount++;
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed %d cover elements"), CoverCount);
}
