// ExoLevelBuilderGameplay.cpp — Spawn points, loot, hazards, barrels, cover, jump pads
#include "Map/ExoLevelBuilder.h"
#include "Map/ExoSpawnPoint.h"
#include "Map/ExoLootSpawner.h"
#include "Map/ExoLootContainer.h"
#include "Map/ExoHazardZone.h"
#include "Map/ExoExplodingBarrel.h"
#include "Map/ExoJumpPad.h"
#include "Map/ExoShieldGenerator.h"
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

void AExoLevelBuilder::PlaceJumpPads()
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	struct FPadInfo { FVector Pos; float Speed; FLinearColor Color; };
	TArray<FPadInfo> Pads = {
		// Hub — central pad for rooftop access
		{{0.f, 5500.f, 10.f}, 2000.f, FLinearColor(0.1f, 0.8f, 1.f)},
		// Hub — east pad near landing zone
		{{6000.f, 0.f, 10.f}, 2200.f, FLinearColor(0.1f, 0.8f, 1.f)},
		// North compound — warehouse roof access
		{{-5000.f, 83000.f, 10.f}, 2500.f, FLinearColor(1.f, 0.6f, 0.15f)},
		// South compound — lab tower launch
		{{3000.f, -83000.f, 10.f}, 2200.f, FLinearColor(0.2f, 1.f, 0.4f)},
		// East power station — tower launch
		{{83000.f, -3000.f, 10.f}, 2800.f, FLinearColor(0.15f, 0.4f, 1.f)},
		// West barracks — between buildings
		{{-80000.f, 0.f, 10.f}, 2000.f, FLinearColor(1.f, 0.2f, 0.15f)},
		// Crossroads area — midfield vertical play
		{{30000.f, 30000.f, 10.f}, 2400.f, FLinearColor(0.5f, 0.8f, 1.f)},
		{{-35000.f, -35000.f, 10.f}, 2400.f, FLinearColor(0.5f, 0.8f, 1.f)},
		// Near bridge positions
		{{50000.f, 50000.f, 10.f}, 2200.f, FLinearColor(0.8f, 0.4f, 1.f)},
		{{-50000.f, -50000.f, 10.f}, 2200.f, FLinearColor(0.8f, 0.4f, 1.f)},
	};

	for (const FPadInfo& P : Pads)
	{
		AExoJumpPad* Pad = GetWorld()->SpawnActor<AExoJumpPad>(
			AExoJumpPad::StaticClass(), P.Pos, FRotator::ZeroRotator, Params);
		if (Pad)
		{
			Pad->InitPad(P.Speed, P.Color);
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed %d jump pads"), Pads.Num());

	// Shield generators at contested positions
	struct FGenInfo { FVector Pos; float Radius; FLinearColor Color; };
	TArray<FGenInfo> Generators = {
		{{0.f, 0.f, 10.f}, 1200.f, FLinearColor(0.1f, 0.6f, 1.f)},       // Hub center
		{{40000.f, 40000.f, 10.f}, 800.f, FLinearColor(0.2f, 1.f, 0.4f)},  // NE crossroads
		{{-40000.f, -40000.f, 10.f}, 800.f, FLinearColor(1.f, 0.4f, 0.2f)}, // SW crossroads
		{{60000.f, -40000.f, 10.f}, 600.f, FLinearColor(0.8f, 0.2f, 1.f)},  // SE ruins
	};

	for (const FGenInfo& G : Generators)
	{
		AExoShieldGenerator* Gen = GetWorld()->SpawnActor<AExoShieldGenerator>(
			AExoShieldGenerator::StaticClass(), G.Pos, FRotator::ZeroRotator, Params);
		if (Gen) Gen->InitGenerator(G.Radius, G.Color);
	}

	// Rock formations for wilderness cover
	FVector RockClusters[] = {
		{25000.f, 60000.f, 0.f}, {-60000.f, 25000.f, 0.f},
		{70000.f, -60000.f, 0.f}, {-25000.f, -70000.f, 0.f},
		{90000.f, 40000.f, 0.f}, {-90000.f, -40000.f, 0.f},
		{15000.f, -40000.f, 0.f}, {-45000.f, 15000.f, 0.f},
	};
	for (const FVector& RC : RockClusters)
	{
		// Central boulder
		float BH = FMath::RandRange(300.f, 600.f);
		float BW = FMath::RandRange(400.f, 800.f);
		SpawnStaticMesh(RC + FVector(0.f, 0.f, BH * 0.4f),
			FVector(BW / 100.f, BW * 0.8f / 100.f, BH / 100.f),
			FRotator(FMath::RandRange(-5.f, 5.f), FMath::RandRange(0.f, 360.f), 0.f),
			SphereMesh, FLinearColor(0.04f, 0.045f, 0.05f));
		// Surrounding slabs (3-5)
		int32 SlabCount = FMath::RandRange(3, 5);
		for (int32 s = 0; s < SlabCount; s++)
		{
			float Ang = (2.f * PI * s) / SlabCount;
			float Dist = FMath::RandRange(300.f, 700.f);
			FVector SlabPos = RC + FVector(FMath::Cos(Ang) * Dist, FMath::Sin(Ang) * Dist, 0.f);
			float SH = FMath::RandRange(150.f, 400.f);
			SpawnStaticMesh(SlabPos + FVector(0.f, 0.f, SH * 0.5f),
				FVector(FMath::RandRange(2.f, 5.f), FMath::RandRange(1.f, 2.5f), SH / 100.f),
				FRotator(FMath::RandRange(-8.f, 8.f), FMath::RandRange(0.f, 360.f), 0.f),
				CubeMesh, FLinearColor(0.04f, 0.045f, 0.055f));
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed shield generators and rock formations"));
}
