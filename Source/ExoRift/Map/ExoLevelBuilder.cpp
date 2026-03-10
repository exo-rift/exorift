// ExoLevelBuilder.cpp — Terrain, lighting, skybox, and mesh helper
#include "Map/ExoLevelBuilder.h"
#include "Map/ExoLootCrate.h"
#include "Map/ExoTargetDummy.h"
#include "Map/ExoPowerUpTerminal.h"
#include "Map/ExoReactorCore.h"
#include "Map/ExoCrashedCapitalShip.h"
#include "Map/ExoRelayTower.h"
#include "Map/ExoFuelDepot.h"
#include "Map/ExoMiningExcavation.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Visual/ExoAmbientParticles.h"
#include "ExoRift.h"

AExoLevelBuilder::AExoLevelBuilder()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Cache engine meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded()) CubeMesh = CubeFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFind.Succeeded()) CylinderMesh = CylFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphFind(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphFind.Succeeded()) SphereMesh = SphFind.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFind(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFind.Succeeded()) BaseMaterial = MatFind.Object;
}

void AExoLevelBuilder::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	UE_LOG(LogExoRift, Log, TEXT("ExoLevelBuilder: Generating level..."));

	BuildTerrain();
	BuildLighting();
	BuildSkybox();
	BuildStructures();
	PlaceSpawnPoints();
	PlaceLootSpawners();
	PlaceHazardZones();
	PlaceExplodingBarrels();
	PlaceCoverElements();
	BuildProps();
	BuildEnvironmentalDebris();
	BuildRoads();
	BuildWaterFeatures();
	BuildFoliage();
	BuildGroundDetail();
	BuildInteriors();
	BuildSignage();
	BuildAtmosphere();
	BuildCatwalks();
	BuildTunnels();
	BuildCompoundLighting();
	PlaceJumpPads();
	PlaceDrones();
	PlaceSteamVents();

	// Ambient floating particles (dust motes / energy wisps)
	AExoAmbientParticles* Particles = AExoAmbientParticles::Get(GetWorld());
	if (Particles) Particles->SetStyle(true); // Energy wisps for sci-fi

	// Loot crates scattered at key locations
	{
		FActorSpawnParameters Crate;
		Crate.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FVector CratePositions[] = {
			{1500.f, -2000.f, 50.f}, {-1800.f, 3000.f, 50.f},
			{3500.f, 79500.f, 50.f}, {-4500.f, 81500.f, 50.f},
			{2000.f, -80500.f, 50.f}, {-3500.f, -82000.f, 50.f},
			{81500.f, 1500.f, 50.f}, {79000.f, -2500.f, 50.f},
			{-80500.f, 2500.f, 50.f}, {-82000.f, -1500.f, 50.f},
			{45000.f, 45000.f, 50.f}, {-55000.f, -55000.f, 50.f},
		};
		for (const FVector& P : CratePositions)
		{
			AExoLootCrate* LC = GetWorld()->SpawnActor<AExoLootCrate>(
				AExoLootCrate::StaticClass(), P,
				FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f), Crate);
			if (LC) LC->ItemCount = FMath::RandRange(1, 3);
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 12 loot crates"));
	}

	// Target dummies near hub for warmup practice
	{
		FActorSpawnParameters Dum;
		Dum.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		struct FDummyDef { FVector Pos; FLinearColor Color; };
		FDummyDef Dummies[] = {
			{{5000.f, 5000.f, 10.f}, FLinearColor(1.f, 0.3f, 0.1f)},
			{{-5000.f, 5500.f, 10.f}, FLinearColor(0.1f, 0.8f, 1.f)},
			{{5500.f, -5000.f, 10.f}, FLinearColor(0.2f, 1.f, 0.3f)},
			{{-5500.f, -5500.f, 10.f}, FLinearColor(1.f, 0.7f, 0.1f)},
			{{7000.f, 0.f, 10.f}, FLinearColor(0.8f, 0.2f, 1.f)},
			{{-7000.f, 0.f, 10.f}, FLinearColor(1.f, 0.1f, 0.4f)},
		};
		for (const FDummyDef& D : Dummies)
		{
			AExoTargetDummy* TD = GetWorld()->SpawnActor<AExoTargetDummy>(
				AExoTargetDummy::StaticClass(), D.Pos, FRotator::ZeroRotator, Dum);
			if (TD) TD->InitDummy(D.Color, 200.f);
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 6 target dummies"));
	}

	// Crashed capital ship — major landmark between hub and north compound
	{
		FActorSpawnParameters ShipP;
		ShipP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoCrashedCapitalShip* Ship = GetWorld()->SpawnActor<AExoCrashedCapitalShip>(
			AExoCrashedCapitalShip::StaticClass(),
			FVector(30000.f, 40000.f, 0.f),
			FRotator(0.f, -25.f, 0.f), ShipP);
		if (Ship) Ship->BuildShip();
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed crashed capital ship landmark"));
	}

	// Energy reactor centerpiece at hub — visible from across the map
	{
		FActorSpawnParameters ReactorP;
		ReactorP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoReactorCore* Reactor = GetWorld()->SpawnActor<AExoReactorCore>(
			AExoReactorCore::StaticClass(), FVector(0.f, -3000.f, 10.f),
			FRotator::ZeroRotator, ReactorP);
		if (Reactor) Reactor->InitReactor();
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed energy reactor at hub center"));
	}

	// Relay towers — tall landmarks for orientation
	{
		FActorSpawnParameters TowerP;
		TowerP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		struct FTowerDef { FVector Pos; float Yaw; };
		FTowerDef Towers[] = {
			{{60000.f, -60000.f, 0.f}, 15.f},   // SE quadrant
			{{-70000.f, 70000.f, 0.f}, -30.f},   // NW quadrant
			{{100000.f, 80000.f, 0.f}, 45.f},    // Far NE
		};
		for (const FTowerDef& T : Towers)
		{
			AExoRelayTower* Tower = GetWorld()->SpawnActor<AExoRelayTower>(
				AExoRelayTower::StaticClass(), T.Pos,
				FRotator(0.f, T.Yaw, 0.f), TowerP);
			if (Tower) Tower->BuildTower();
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 3 relay towers"));
	}

	// Fuel depots — industrial storage at key routes
	{
		FActorSpawnParameters DepotP;
		DepotP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		struct FDepotDef { FVector Pos; float Yaw; };
		FDepotDef Depots[] = {
			{{-50000.f, 30000.f, 0.f}, 20.f},
			{{40000.f, -50000.f, 0.f}, -35.f},
		};
		for (const FDepotDef& D : Depots)
		{
			AExoFuelDepot* Depot = GetWorld()->SpawnActor<AExoFuelDepot>(
				AExoFuelDepot::StaticClass(), D.Pos,
				FRotator(0.f, D.Yaw, 0.f), DepotP);
			if (Depot) Depot->BuildDepot();
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 2 fuel depots"));
	}

	// Mining excavation — quarry with mineral veins
	{
		FActorSpawnParameters MineP;
		MineP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoMiningExcavation* Mine = GetWorld()->SpawnActor<AExoMiningExcavation>(
			AExoMiningExcavation::StaticClass(),
			FVector(-40000.f, -60000.f, 0.f),
			FRotator(0.f, 15.f, 0.f), MineP);
		if (Mine) Mine->BuildSite();
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed mining excavation site"));
	}

	// Power-up terminals at strategic compound locations
	{
		FActorSpawnParameters TermP;
		TermP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		struct FTermDef { FVector Pos; EPowerUpType Type; };
		FTermDef Terminals[] = {
			{{2500.f, -1000.f, 10.f}, EPowerUpType::SpeedBoost},
			{{-2500.f, 1000.f, 10.f}, EPowerUpType::DamageBoost},
			{{4000.f, 80000.f, 10.f}, EPowerUpType::ShieldRecharge},
			{{-4000.f, -81000.f, 10.f}, EPowerUpType::OverheatReset},
			{{80000.f, 2000.f, 10.f}, EPowerUpType::SpeedBoost},
			{{-81000.f, -2000.f, 10.f}, EPowerUpType::DamageBoost},
			{{45000.f, 46000.f, 10.f}, EPowerUpType::ShieldRecharge},
			{{-55000.f, -54000.f, 10.f}, EPowerUpType::OverheatReset},
		};
		for (const FTermDef& T : Terminals)
		{
			AExoPowerUpTerminal* Term = GetWorld()->SpawnActor<AExoPowerUpTerminal>(
				AExoPowerUpTerminal::StaticClass(), T.Pos,
				FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f), TermP);
			if (Term) Term->InitTerminal(T.Type);
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 8 power-up terminals"));
	}

	UE_LOG(LogExoRift, Log, TEXT("ExoLevelBuilder: Level complete — %d mesh components, ready."),
		LevelMeshes.Num());
}

void AExoLevelBuilder::BuildTerrain()
{
	// Large ground plane
	float GroundHalf = MapHalfSize * 1.5f; // Extend past zone
	float GroundScale = GroundHalf / 50.f;  // Cube is 100 units
	SpawnStaticMesh(
		FVector(0.f, 0.f, -50.f), // Slightly below zero
		FVector(GroundScale, GroundScale, 1.f),
		FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.04f, 0.045f, 0.05f)); // Dark sci-fi floor

	// Terrain variation — raised platforms across the map
	struct FTerrainPatch { FVector Pos; float SizeX; float SizeY; float Height; };
	TArray<FTerrainPatch> Patches = {
		{{-80000.f, -60000.f, 0.f}, 40000.f, 30000.f, 200.f},
		{{60000.f, 80000.f, 0.f}, 35000.f, 25000.f, 150.f},
		{{-40000.f, 100000.f, 0.f}, 50000.f, 20000.f, 300.f},
		{{100000.f, -50000.f, 0.f}, 30000.f, 40000.f, 250.f},
		{{-120000.f, 30000.f, 0.f}, 25000.f, 35000.f, 180.f},
	};

	for (const auto& P : Patches)
	{
		SpawnStaticMesh(
			FVector(P.Pos.X, P.Pos.Y, P.Height * 0.5f),
			FVector(P.SizeX / 100.f, P.SizeY / 100.f, P.Height / 100.f),
			FRotator::ZeroRotator, CubeMesh,
			FLinearColor(0.05f, 0.055f, 0.06f));
	}

	// Rocky ridgelines — stacked angular slabs for natural cover
	struct FRidge { FVector Start; FVector End; float Height; float Width; };
	TArray<FRidge> Ridges = {
		{{30000.f, 20000.f, 0.f}, {50000.f, 35000.f, 0.f}, 400.f, 2000.f},
		{{-40000.f, -10000.f, 0.f}, {-60000.f, -30000.f, 0.f}, 350.f, 1800.f},
		{{70000.f, -80000.f, 0.f}, {90000.f, -60000.f, 0.f}, 500.f, 2500.f},
		{{-90000.f, 70000.f, 0.f}, {-70000.f, 80000.f, 0.f}, 300.f, 1500.f},
		{{100000.f, 30000.f, 0.f}, {110000.f, 50000.f, 0.f}, 450.f, 2200.f},
	};
	for (const auto& R : Ridges)
	{
		FVector Dir = R.End - R.Start;
		float Len = Dir.Size();
		FRotator Rot = Dir.Rotation();
		int32 Segments = FMath::Max(2, FMath::RoundToInt32(Len / 5000.f));
		for (int32 i = 0; i < Segments; i++)
		{
			float T = (float)i / (float)Segments;
			FVector Pos = FMath::Lerp(R.Start, R.End, T + 0.5f / Segments);
			float SegH = R.Height * (0.6f + 0.4f * FMath::Sin(T * PI));
			float SegW = R.Width * (0.8f + 0.2f * FMath::Sin(T * 3.f));
			SpawnStaticMesh(
				FVector(Pos.X, Pos.Y, SegH * 0.5f),
				FVector(Len / Segments / 100.f, SegW / 100.f, SegH / 100.f),
				FRotator(FMath::RandRange(-3.f, 3.f), Rot.Yaw, 0.f),
				CubeMesh, FLinearColor(0.04f, 0.045f, 0.055f));
		}
	}

	// Hills — large rounded terrain bumps using spheres
	struct FHill { FVector Pos; float Radius; float Height; };
	TArray<FHill> Hills = {
		{{-20000.f, 40000.f, -100.f}, 6000.f, 600.f},
		{{50000.f, -20000.f, -100.f}, 5000.f, 500.f},
		{{-60000.f, 70000.f, -100.f}, 7000.f, 400.f},
		{{80000.f, 60000.f, -100.f}, 5500.f, 550.f},
		{{-100000.f, -100000.f, -100.f}, 8000.f, 350.f},
		{{110000.f, -30000.f, -100.f}, 4500.f, 450.f},
	};
	for (const auto& H : Hills)
	{
		float SR = H.Radius / 50.f;
		float SH = H.Height / 50.f;
		SpawnStaticMesh(H.Pos,
			FVector(SR, SR, SH), FRotator::ZeroRotator, SphereMesh,
			FLinearColor(0.045f, 0.05f, 0.055f));
	}
}

void AExoLevelBuilder::BuildLighting()
{
	// Directional light (sun) — create as component on this actor
	UDirectionalLightComponent* Sun = NewObject<UDirectionalLightComponent>(this);
	Sun->SetupAttachment(RootComponent);
	Sun->RegisterComponent();
	Sun->SetWorldRotation(FRotator(-45.f, 30.f, 0.f));
	Sun->SetIntensity(3.f);
	Sun->SetLightColor(FLinearColor(0.9f, 0.85f, 0.7f));
	Sun->CastShadows = true;

	// Sky light for ambient fill
	USkyLightComponent* Sky = NewObject<USkyLightComponent>(this);
	Sky->SetupAttachment(RootComponent);
	Sky->RegisterComponent();
	Sky->SetIntensity(1.5f);
	Sky->SetLightColor(FLinearColor(0.4f, 0.5f, 0.7f));

	// Atmospheric fog
	UExponentialHeightFogComponent* Fog = NewObject<UExponentialHeightFogComponent>(this);
	Fog->SetupAttachment(RootComponent);
	Fog->RegisterComponent();
	Fog->SetFogDensity(0.001f);
	Fog->SetFogHeightFalloff(0.2f);
	Fog->SetFogInscatteringColor(FLinearColor(0.15f, 0.2f, 0.3f));

	// Global post-process — lower priority than ExoPostProcess actor (priority 0 < 1)
	UPostProcessComponent* PP = NewObject<UPostProcessComponent>(this);
	PP->SetupAttachment(RootComponent);
	PP->bUnbound = true;
	PP->Priority = 0.f;
	PP->Settings.bOverride_AutoExposureBias = true;
	PP->Settings.AutoExposureBias = 0.3f;
	PP->RegisterComponent();

	// Nav mesh bounds for bot navigation
	FActorSpawnParameters NavParams;
	NavParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ANavMeshBoundsVolume* NavBounds = GetWorld()->SpawnActor<ANavMeshBoundsVolume>(
		ANavMeshBoundsVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, NavParams);
	if (NavBounds)
	{
		NavBounds->SetActorScale3D(FVector(MapHalfSize / 100.f, MapHalfSize / 100.f, 500.f));
	}
}

UStaticMeshComponent* AExoLevelBuilder::SpawnStaticMesh(const FVector& Location,
	const FVector& Scale, const FRotator& Rotation, UStaticMesh* Mesh, const FLinearColor& Color)
{
	if (!Mesh) return nullptr;

	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
	Comp->SetupAttachment(RootComponent);
	Comp->SetStaticMesh(Mesh);
	Comp->SetWorldLocation(Location);
	Comp->SetWorldScale3D(Scale);
	Comp->SetWorldRotation(Rotation);
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Comp->SetCollisionResponseToAllChannels(ECR_Block);
	Comp->CastShadow = true;
	Comp->RegisterComponent();

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		DynMat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Comp->SetMaterial(0, DynMat);
	}

	LevelMeshes.Add(Comp);
	return Comp;
}
