// ExoLevelBuilderMechanics.cpp — Jump pads, drones, steam vents
#include "Map/ExoLevelBuilder.h"
#include "Map/ExoJumpPad.h"
#include "Map/ExoShieldGenerator.h"
#include "Map/ExoPatrolDrone.h"
#include "Visual/ExoSteamVent.h"
#include "Components/StaticMeshComponent.h"
#include "ExoRift.h"

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
		{{-5000.f, NorthY + 3000.f, 10.f}, 2500.f, FLinearColor(1.f, 0.6f, 0.15f)},
		// South compound — lab tower launch
		{{3000.f, SouthY - 3000.f, 10.f}, 2200.f, FLinearColor(0.2f, 1.f, 0.4f)},
		// East power station — tower launch
		{{EastX + 3000.f, -3000.f, 10.f}, 2800.f, FLinearColor(0.15f, 0.4f, 1.f)},
		// West barracks — between buildings
		{{WestX, 0.f, 10.f}, 2000.f, FLinearColor(1.f, 0.2f, 0.15f)},
		// Crossroads area — midfield vertical play
		{{6000.f, 6000.f, 10.f}, 2400.f, FLinearColor(0.5f, 0.8f, 1.f)},
		{{-7000.f, -7000.f, 10.f}, 2400.f, FLinearColor(0.5f, 0.8f, 1.f)},
		// Near bridge positions
		{{10000.f, 10000.f, 10.f}, 2200.f, FLinearColor(0.8f, 0.4f, 1.f)},
		{{-10000.f, -10000.f, 10.f}, 2200.f, FLinearColor(0.8f, 0.4f, 1.f)},
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
		{{8000.f, 8000.f, 10.f}, 800.f, FLinearColor(0.2f, 1.f, 0.4f)},  // NE crossroads
		{{-8000.f, -8000.f, 10.f}, 800.f, FLinearColor(1.f, 0.4f, 0.2f)}, // SW crossroads
		{{12000.f, -8000.f, 10.f}, 600.f, FLinearColor(0.8f, 0.2f, 1.f)},  // SE ruins
	};

	for (const FGenInfo& G : Generators)
	{
		AExoShieldGenerator* Gen = GetWorld()->SpawnActor<AExoShieldGenerator>(
			AExoShieldGenerator::StaticClass(), G.Pos, FRotator::ZeroRotator, Params);
		if (Gen) Gen->InitGenerator(G.Radius, G.Color);
	}

	// Rock formations for wilderness cover
	FVector RockClusters[] = {
		{5000.f, 12000.f, 0.f}, {-12000.f, 5000.f, 0.f},
		{14000.f, -12000.f, 0.f}, {-5000.f, -14000.f, 0.f},
		{18000.f, 8000.f, 0.f}, {-18000.f, -8000.f, 0.f},
		{3000.f, -8000.f, 0.f}, {-9000.f, 3000.f, 0.f},
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

void AExoLevelBuilder::PlaceDrones()
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	struct FDroneRoute
	{
		TArray<FVector> Waypoints;
		FLinearColor Color;
		float Speed;
	};

	TArray<FDroneRoute> Routes;

	// Route 1: Perimeter patrol (wide circuit around the map)
	{
		FDroneRoute R;
		R.Color = FLinearColor(0.1f, 0.5f, 1.f);
		R.Speed = 1200.f;
		float Rad = MapHalfSize * 0.6f;
		for (int32 i = 0; i < 8; i++)
		{
			float A = (2.f * PI * i) / 8.f;
			R.Waypoints.Add(FVector(FMath::Cos(A) * Rad, FMath::Sin(A) * Rad, 0.f));
		}
		Routes.Add(R);
	}

	// Route 2: Inner hub patrol
	{
		FDroneRoute R;
		R.Color = FLinearColor(1.f, 0.4f, 0.1f);
		R.Speed = 600.f;
		float Rad = 5000.f;
		for (int32 i = 0; i < 6; i++)
		{
			float A = (2.f * PI * i) / 6.f + PI / 6.f;
			R.Waypoints.Add(FVector(FMath::Cos(A) * Rad, FMath::Sin(A) * Rad, 0.f));
		}
		Routes.Add(R);
	}

	// Route 3: North-south corridor
	{
		FDroneRoute R;
		R.Color = FLinearColor(0.2f, 1.f, 0.4f);
		R.Speed = 900.f;
		R.Waypoints.Add(FVector(0.f, -24000.f, 0.f));
		R.Waypoints.Add(FVector(4000.f, -12000.f, 0.f));
		R.Waypoints.Add(FVector(-2000.f, 0.f, 0.f));
		R.Waypoints.Add(FVector(3000.f, 12000.f, 0.f));
		R.Waypoints.Add(FVector(0.f, 24000.f, 0.f));
		Routes.Add(R);
	}

	for (const FDroneRoute& Route : Routes)
	{
		AExoPatrolDrone* Drone = GetWorld()->SpawnActor<AExoPatrolDrone>(
			AExoPatrolDrone::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (Drone)
		{
			Drone->InitDrone(Route.Waypoints, Route.Color, Route.Speed);
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed %d patrol drones"), Routes.Num());
}

void AExoLevelBuilder::PlaceSteamVents()
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	struct FVentDef { FVector Pos; FLinearColor Color; float Interval; };
	TArray<FVentDef> Vents = {
		// Industrial compound — hot amber vents
		{{2000.f, NorthY, 0.f}, FLinearColor(1.f, 0.5f, 0.1f), 4.f},
		{{-3000.f, NorthY + 2000.f, 0.f}, FLinearColor(1.f, 0.6f, 0.15f), 6.f},
		{{5000.f, NorthY - 2000.f, 0.f}, FLinearColor(1.f, 0.4f, 0.05f), 5.f},
		// Power Station — electric blue
		{{EastX + 2000.f, 2000.f, 0.f}, FLinearColor(0.2f, 0.5f, 1.f), 3.f},
		{{EastX - 2000.f, -3000.f, 0.f}, FLinearColor(0.15f, 0.4f, 1.f), 7.f},
		// Research Labs — green gas
		{{-2000.f, SouthY, 0.f}, FLinearColor(0.2f, 0.9f, 0.3f), 5.f},
		{{3000.f, SouthY - 2000.f, 0.f}, FLinearColor(0.3f, 1.f, 0.4f), 8.f},
	};

	for (const FVentDef& V : Vents)
	{
		AExoSteamVent* Vent = GetWorld()->SpawnActor<AExoSteamVent>(
			AExoSteamVent::StaticClass(), V.Pos, FRotator::ZeroRotator, Params);
		if (Vent) Vent->InitVent(V.Interval, 0.8f, V.Color);
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed %d steam vents"), Vents.Num());
}
