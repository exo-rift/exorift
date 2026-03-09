#include "Map/ExoPOI.h"
#include "Map/ExoBuilding.h"
#include "Map/ExoCoverObject.h"
#include "Map/ExoExplodingBarrel.h"
#include "Map/ExoLootContainer.h"
#include "Map/ExoProceduralTerrain.h"
#include "ExoRift.h"

AExoPOI::AExoPOI()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AExoPOI::BeginPlay()
{
	Super::BeginPlay();
}

void AExoPOI::PopulatePOI(AExoProceduralTerrain* Terrain)
{
	if (bPopulated) return;

	UE_LOG(LogExoRift, Log, TEXT("POI [%s]: populating (%d buildings, radius=%.0f)"),
		*POIName, BuildingCount, Radius);

	SpawnBuildings(Terrain);
	SpawnCoverObjects(Terrain);
	SpawnEnvironmentalProps(Terrain);

	bPopulated = true;
}

void AExoPOI::SpawnBuildings(AExoProceduralTerrain* Terrain)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Determine building types based on POI type
	TArray<EBuildingType> AllowedTypes;
	switch (POIType)
	{
	case EPOIType::MilitaryBase:
		AllowedTypes = { EBuildingType::Bunker, EBuildingType::LargeWarehouse, EBuildingType::Tower };
		break;
	case EPOIType::TownCenter:
		AllowedTypes = { EBuildingType::SmallHouse, EBuildingType::MediumHouse, EBuildingType::ShopFront };
		break;
	case EPOIType::IndustrialZone:
		AllowedTypes = { EBuildingType::LargeWarehouse, EBuildingType::Bunker };
		break;
	case EPOIType::ResearchFacility:
		AllowedTypes = { EBuildingType::MediumHouse, EBuildingType::Tower, EBuildingType::Bunker };
		break;
	case EPOIType::CrashSite:
		AllowedTypes = { EBuildingType::Ruins, EBuildingType::SmallHouse };
		break;
	case EPOIType::PowerPlant:
		AllowedTypes = { EBuildingType::LargeWarehouse, EBuildingType::Tower };
		break;
	case EPOIType::Marketplace:
		AllowedTypes = { EBuildingType::ShopFront, EBuildingType::SmallHouse, EBuildingType::MediumHouse };
		break;
	default:
		AllowedTypes = { EBuildingType::SmallHouse, EBuildingType::MediumHouse };
		break;
	}

	const FVector Center = GetActorLocation();

	for (int32 i = 0; i < BuildingCount; ++i)
	{
		// Place buildings in a rough grid with jitter
		float Angle = (2.f * PI * i) / BuildingCount + FMath::RandRange(-0.3f, 0.3f);
		float Dist = FMath::RandRange(Radius * 0.15f, Radius * 0.8f);
		FVector2D Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist);
		FVector2D WorldXY(Center.X + Offset.X, Center.Y + Offset.Y);

		float GroundZ = GetGroundHeight(WorldXY, Terrain);
		FVector SpawnLoc(WorldXY.X, WorldXY.Y, GroundZ);
		FRotator SpawnRot(0.f, FMath::RandRange(0.f, 360.f), 0.f);

		FActorSpawnParameters Params;
		AExoBuilding* Building = World->SpawnActor<AExoBuilding>(
			AExoBuilding::StaticClass(), SpawnLoc, SpawnRot, Params);

		if (Building)
		{
			int32 TypeIdx = FMath::RandRange(0, AllowedTypes.Num() - 1);
			Building->BuildingType = AllowedTypes[TypeIdx];
			Building->GenerateBuilding();
			Buildings.Add(Building);

			// Spawn loot containers inside buildings
			TArray<FVector> LootPositions = Building->GetLootSpawnPositions();
			int32 LootCount = FMath::CeilToInt(LootPositions.Num() * LootDensity * 0.6f);
			for (int32 L = 0; L < FMath::Min(LootCount, LootPositions.Num()); ++L)
			{
				World->SpawnActor<AExoLootContainer>(
					AExoLootContainer::StaticClass(), LootPositions[L], FRotator::ZeroRotator, Params);
			}
		}
	}
}

void AExoPOI::SpawnCoverObjects(AExoProceduralTerrain* Terrain)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Scatter cover objects between buildings
	int32 CoverCount = FMath::CeilToInt(BuildingCount * 1.5f);
	const FVector Center = GetActorLocation();

	TArray<ECoverType> CoverTypes = {
		ECoverType::Crate, ECoverType::Jersey_Barrier, ECoverType::Wall_Half,
		ECoverType::Sandbags, ECoverType::Rocks
	};

	// Military/Industrial POIs get vehicle wrecks
	if (POIType == EPOIType::MilitaryBase || POIType == EPOIType::CrashSite)
	{
		CoverTypes.Add(ECoverType::Vehicle_Wreck);
	}

	for (int32 i = 0; i < CoverCount; ++i)
	{
		FVector2D Offset = FMath::RandPointInCircle(Radius * 0.9f);
		FVector2D WorldXY(Center.X + Offset.X, Center.Y + Offset.Y);
		float GroundZ = GetGroundHeight(WorldXY, Terrain);

		FVector SpawnLoc(WorldXY.X, WorldXY.Y, GroundZ);
		FRotator SpawnRot(0.f, FMath::RandRange(0.f, 360.f), 0.f);

		FActorSpawnParameters Params;
		AExoCoverObject* Cover = World->SpawnActor<AExoCoverObject>(
			AExoCoverObject::StaticClass(), SpawnLoc, SpawnRot, Params);

		if (Cover)
		{
			int32 TypeIdx = FMath::RandRange(0, CoverTypes.Num() - 1);
			Cover->CoverType = CoverTypes[TypeIdx];
			Cover->GenerateCover();
		}
	}
}

void AExoPOI::SpawnEnvironmentalProps(AExoProceduralTerrain* Terrain)
{
	UWorld* World = GetWorld();
	if (!World) return;

	const FVector Center = GetActorLocation();

	// Exploding barrels — 2-4 per POI
	int32 BarrelCount = FMath::RandRange(2, 4);
	for (int32 i = 0; i < BarrelCount; ++i)
	{
		FVector2D Offset = FMath::RandPointInCircle(Radius * 0.7f);
		FVector2D WorldXY(Center.X + Offset.X, Center.Y + Offset.Y);
		float GroundZ = GetGroundHeight(WorldXY, Terrain);

		FVector SpawnLoc(WorldXY.X, WorldXY.Y, GroundZ + 50.f);
		FActorSpawnParameters Params;
		World->SpawnActor<AExoExplodingBarrel>(
			AExoExplodingBarrel::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
	}
}

float AExoPOI::GetGroundHeight(const FVector2D& WorldXY, AExoProceduralTerrain* Terrain) const
{
	if (Terrain && Terrain->IsGenerated())
	{
		return Terrain->GetHeightAtLocation(WorldXY);
	}

	// Fallback: line trace
	UWorld* World = GetWorld();
	if (!World) return 0.f;

	FVector TraceStart(WorldXY.X, WorldXY.Y, 20000.f);
	FVector TraceEnd(WorldXY.X, WorldXY.Y, -20000.f);
	FHitResult Hit;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams))
	{
		return Hit.ImpactPoint.Z;
	}

	return 0.f;
}
