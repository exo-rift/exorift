#include "Map/ExoWorldBuilder.h"
#include "Map/ExoProceduralTerrain.h"
#include "Map/ExoLevelLighting.h"
#include "Map/ExoWaterPlane.h"
#include "Map/ExoPOI.h"
#include "Map/ExoCoverObject.h"
#include "Map/ExoSpawnPoint.h"
#include "Map/ExoHazardZone.h"
#include "ExoRift.h"

AExoWorldBuilder::AExoWorldBuilder()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AExoWorldBuilder::BeginPlay()
{
	Super::BeginPlay();
}

void AExoWorldBuilder::BuildWorld()
{
	if (bWorldBuilt) return;

	UE_LOG(LogExoRift, Log, TEXT("WorldBuilder: ====== BUILDING WORLD ======"));

	BuildTerrain();
	BuildLighting();
	BuildWater();
	BuildPOIs();
	BuildOpenWorldCover();
	BuildSpawnPoints();
	BuildHazardZones();

	bWorldBuilt = true;
	UE_LOG(LogExoRift, Log, TEXT("WorldBuilder: ====== WORLD COMPLETE ======"));
}

// ---------------------------------------------------------------------------
// Step 1: Terrain
// ---------------------------------------------------------------------------
void AExoWorldBuilder::BuildTerrain()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Terrain = World->SpawnActor<AExoProceduralTerrain>(
		AExoProceduralTerrain::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);

	if (Terrain)
	{
		Terrain->Seed = 2026;
		Terrain->GridResolution = 256;
		Terrain->MapExtent = 150000.f;
		Terrain->MaxHeight = 8000.f;
		Terrain->GenerateTerrain();
	}

	UE_LOG(LogExoRift, Log, TEXT("WorldBuilder: terrain generated"));
}

// ---------------------------------------------------------------------------
// Step 2: Lighting
// ---------------------------------------------------------------------------
void AExoWorldBuilder::BuildLighting()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Lighting = World->SpawnActor<AExoLevelLighting>(
		AExoLevelLighting::StaticClass(), FVector(0.f, 0.f, 50000.f), FRotator::ZeroRotator, Params);

	if (Lighting)
	{
		// Random time of day for match variety
		TArray<ETimeOfDay> Times = { ETimeOfDay::Morning, ETimeOfDay::Noon, ETimeOfDay::Afternoon, ETimeOfDay::Dawn };
		int32 Idx = FMath::RandRange(0, Times.Num() - 1);
		Lighting->SetTimeOfDay(Times[Idx]);
	}

	UE_LOG(LogExoRift, Log, TEXT("WorldBuilder: lighting set up"));
}

// ---------------------------------------------------------------------------
// Step 3: Water
// ---------------------------------------------------------------------------
void AExoWorldBuilder::BuildWater()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Water = World->SpawnActor<AExoWaterPlane>(
		AExoWaterPlane::StaticClass(), FVector(0.f, 0.f, -200.f), FRotator::ZeroRotator, Params);

	if (Water)
	{
		Water->WaterExtent = 200000.f;
		Water->WaterLevel = -200.f;
	}

	UE_LOG(LogExoRift, Log, TEXT("WorldBuilder: water plane placed"));
}

// ---------------------------------------------------------------------------
// Step 4: POIs — the heart of the level design
// ---------------------------------------------------------------------------
void AExoWorldBuilder::BuildPOIs()
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<FPOIDefinition> Layout = CreateMapLayout();

	for (const FPOIDefinition& Def : Layout)
	{
		AExoPOI* NewPOI = SpawnPOI(Def);
		if (NewPOI)
		{
			POIs.Add(NewPOI);
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("WorldBuilder: %d POIs created"), POIs.Num());
}

TArray<FPOIDefinition> AExoWorldBuilder::CreateMapLayout() const
{
	TArray<FPOIDefinition> Layout;

	// Central hub — "Nexus Core" (high-value military complex)
	{
		FPOIDefinition D;
		D.Name = TEXT("Nexus Core");
		D.Type = EPOIType::MilitaryBase;
		D.Position = FVector2D(0.f, 0.f);
		D.Radius = 8000.f;
		D.BuildingCount = 8;
		D.LootDensity = 1.5f;
		Layout.Add(D);
	}

	// Northern research facility — "Cryo Labs"
	{
		FPOIDefinition D;
		D.Name = TEXT("Cryo Labs");
		D.Type = EPOIType::ResearchFacility;
		D.Position = FVector2D(0.f, 80000.f);
		D.Radius = 6000.f;
		D.BuildingCount = 6;
		D.LootDensity = 1.3f;
		Layout.Add(D);
	}

	// Southern town — "Dusthaven"
	{
		FPOIDefinition D;
		D.Name = TEXT("Dusthaven");
		D.Type = EPOIType::TownCenter;
		D.Position = FVector2D(0.f, -85000.f);
		D.Radius = 7000.f;
		D.BuildingCount = 10;
		D.LootDensity = 1.0f;
		Layout.Add(D);
	}

	// Eastern industrial — "Iron Works"
	{
		FPOIDefinition D;
		D.Name = TEXT("Iron Works");
		D.Type = EPOIType::IndustrialZone;
		D.Position = FVector2D(75000.f, 20000.f);
		D.Radius = 6000.f;
		D.BuildingCount = 5;
		D.LootDensity = 1.1f;
		Layout.Add(D);
	}

	// Western crash site — "Wreck Valley"
	{
		FPOIDefinition D;
		D.Name = TEXT("Wreck Valley");
		D.Type = EPOIType::CrashSite;
		D.Position = FVector2D(-80000.f, 10000.f);
		D.Radius = 5000.f;
		D.BuildingCount = 4;
		D.LootDensity = 1.4f;
		Layout.Add(D);
	}

	// Southeast power plant — "Volt Station"
	{
		FPOIDefinition D;
		D.Name = TEXT("Volt Station");
		D.Type = EPOIType::PowerPlant;
		D.Position = FVector2D(60000.f, -60000.f);
		D.Radius = 5500.f;
		D.BuildingCount = 5;
		D.LootDensity = 1.2f;
		Layout.Add(D);
	}

	// Northwest marketplace — "Trader's Reach"
	{
		FPOIDefinition D;
		D.Name = TEXT("Trader's Reach");
		D.Type = EPOIType::Marketplace;
		D.Position = FVector2D(-55000.f, 65000.f);
		D.Radius = 5000.f;
		D.BuildingCount = 7;
		D.LootDensity = 1.0f;
		Layout.Add(D);
	}

	// Northeast outpost — "Signal Ridge"
	{
		FPOIDefinition D;
		D.Name = TEXT("Signal Ridge");
		D.Type = EPOIType::Outpost;
		D.Position = FVector2D(65000.f, 70000.f);
		D.Radius = 4000.f;
		D.BuildingCount = 3;
		D.LootDensity = 0.8f;
		Layout.Add(D);
	}

	// Southwest outpost — "Quarry Depths"
	{
		FPOIDefinition D;
		D.Name = TEXT("Quarry Depths");
		D.Type = EPOIType::Outpost;
		D.Position = FVector2D(-65000.f, -55000.f);
		D.Radius = 4500.f;
		D.BuildingCount = 4;
		D.LootDensity = 0.9f;
		Layout.Add(D);
	}

	// Far north outpost — "Frostbite Point"
	{
		FPOIDefinition D;
		D.Name = TEXT("Frostbite Point");
		D.Type = EPOIType::Outpost;
		D.Position = FVector2D(-30000.f, 100000.f);
		D.Radius = 3500.f;
		D.BuildingCount = 3;
		D.LootDensity = 0.7f;
		Layout.Add(D);
	}

	// Far east — "Ember Outpost"
	{
		FPOIDefinition D;
		D.Name = TEXT("Ember Outpost");
		D.Type = EPOIType::MilitaryBase;
		D.Position = FVector2D(100000.f, -15000.f);
		D.Radius = 4000.f;
		D.BuildingCount = 4;
		D.LootDensity = 1.2f;
		Layout.Add(D);
	}

	// South-central — "Deadlight Market"
	{
		FPOIDefinition D;
		D.Name = TEXT("Deadlight Market");
		D.Type = EPOIType::Marketplace;
		D.Position = FVector2D(25000.f, -45000.f);
		D.Radius = 4000.f;
		D.BuildingCount = 5;
		D.LootDensity = 1.0f;
		Layout.Add(D);
	}

	return Layout;
}

AExoPOI* AExoWorldBuilder::SpawnPOI(const FPOIDefinition& Def)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	float GroundZ = GetGroundZ(Def.Position);
	FVector SpawnLoc(Def.Position.X, Def.Position.Y, GroundZ);

	FActorSpawnParameters Params;
	AExoPOI* POI = World->SpawnActor<AExoPOI>(
		AExoPOI::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);

	if (POI)
	{
		POI->POIName = Def.Name;
		POI->POIType = Def.Type;
		POI->Radius = Def.Radius;
		POI->BuildingCount = Def.BuildingCount;
		POI->LootDensity = Def.LootDensity;
		POI->PopulatePOI(Terrain);
	}

	return POI;
}

// ---------------------------------------------------------------------------
// Step 5: Open world cover (between POIs)
// ---------------------------------------------------------------------------
void AExoWorldBuilder::BuildOpenWorldCover()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const int32 OpenCoverCount = 80;
	const float Extent = 120000.f;

	TArray<ECoverType> Types = {
		ECoverType::Rocks, ECoverType::Rocks, ECoverType::Wall_Half,
		ECoverType::Crate, ECoverType::Jersey_Barrier, ECoverType::Vehicle_Wreck
	};

	int32 Placed = 0;
	for (int32 i = 0; i < OpenCoverCount; ++i)
	{
		FVector2D Pos(FMath::RandRange(-Extent, Extent), FMath::RandRange(-Extent, Extent));
		float GroundZ = GetGroundZ(Pos);

		// Skip if underwater
		if (GroundZ < 0.f) continue;

		FVector SpawnLoc(Pos.X, Pos.Y, GroundZ);
		FRotator SpawnRot(0.f, FMath::RandRange(0.f, 360.f), 0.f);

		FActorSpawnParameters Params;
		AExoCoverObject* Cover = World->SpawnActor<AExoCoverObject>(
			AExoCoverObject::StaticClass(), SpawnLoc, SpawnRot, Params);

		if (Cover)
		{
			int32 TypeIdx = FMath::RandRange(0, Types.Num() - 1);
			Cover->CoverType = Types[TypeIdx];
			Cover->ScaleMultiplier = FMath::RandRange(0.8f, 1.5f);
			Cover->GenerateCover();
			++Placed;
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("WorldBuilder: %d open-world cover objects placed"), Placed);
}

// ---------------------------------------------------------------------------
// Step 6: Spawn points
// ---------------------------------------------------------------------------
void AExoWorldBuilder::BuildSpawnPoints()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Create a ring of drop-zone spawn points around the map
	const int32 NumDropSpawns = 30;
	const float DropRadius = 110000.f;
	const float DropAltitude = 15000.f;

	for (int32 i = 0; i < NumDropSpawns; ++i)
	{
		float Angle = (2.f * PI * i) / NumDropSpawns;
		FVector Loc(FMath::Cos(Angle) * DropRadius, FMath::Sin(Angle) * DropRadius, DropAltitude);

		FActorSpawnParameters Params;
		AExoSpawnPoint* SP = World->SpawnActor<AExoSpawnPoint>(
			AExoSpawnPoint::StaticClass(), Loc, FRotator::ZeroRotator, Params);
		if (SP)
		{
			SP->bIsDropZone = true;
			SP->bIsLobbySpawn = false;
		}
	}

	// Lobby spawns near center
	const int32 NumLobbySpawns = 10;
	const float LobbyRadius = 3000.f;
	for (int32 i = 0; i < NumLobbySpawns; ++i)
	{
		float Angle = (2.f * PI * i) / NumLobbySpawns;
		float Z = GetGroundZ(FVector2D(FMath::Cos(Angle) * LobbyRadius, FMath::Sin(Angle) * LobbyRadius));
		FVector Loc(FMath::Cos(Angle) * LobbyRadius, FMath::Sin(Angle) * LobbyRadius, Z + 100.f);

		FActorSpawnParameters Params;
		AExoSpawnPoint* SP = World->SpawnActor<AExoSpawnPoint>(
			AExoSpawnPoint::StaticClass(), Loc, FRotator::ZeroRotator, Params);
		if (SP)
		{
			SP->bIsDropZone = false;
			SP->bIsLobbySpawn = true;
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("WorldBuilder: %d drop + %d lobby spawn points"),
		NumDropSpawns, NumLobbySpawns);
}

// ---------------------------------------------------------------------------
// Step 7: Hazard zones
// ---------------------------------------------------------------------------
void AExoWorldBuilder::BuildHazardZones()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Radiation zones near the crash site and power plant
	struct HazardDef
	{
		FVector2D Pos;
		float Radius;
		EHazardType Type;
		FString Name;
	};

	TArray<HazardDef> Hazards;
	Hazards.Add({ FVector2D(-80000.f, 15000.f), 3000.f, EHazardType::Radiation, TEXT("Crash Radiation") });
	Hazards.Add({ FVector2D(60000.f, -55000.f), 2500.f, EHazardType::Electric, TEXT("Volt Station Discharge") });
	Hazards.Add({ FVector2D(30000.f, 50000.f), 2000.f, EHazardType::Toxic, TEXT("Chemical Spill") });

	for (const HazardDef& H : Hazards)
	{
		float GroundZ = GetGroundZ(H.Pos);
		FVector SpawnLoc(H.Pos.X, H.Pos.Y, GroundZ);

		FActorSpawnParameters Params;
		AExoHazardZone* Zone = World->SpawnActor<AExoHazardZone>(
			AExoHazardZone::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);

		if (Zone)
		{
			Zone->HazardRadius = H.Radius;
			Zone->HazardName = H.Name;
			Zone->HazardType = H.Type;
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("WorldBuilder: %d hazard zones placed"), Hazards.Num());
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------
float AExoWorldBuilder::GetGroundZ(const FVector2D& WorldXY) const
{
	if (Terrain && Terrain->IsGenerated())
	{
		return Terrain->GetHeightAtLocation(WorldXY);
	}
	return 0.f;
}
