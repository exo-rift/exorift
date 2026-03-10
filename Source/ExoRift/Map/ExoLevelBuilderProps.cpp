// ExoLevelBuilderProps.cpp — Light posts, antennas, pipes, accent lighting
#include "Map/ExoLevelBuilder.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildProps()
{
	// === STREET LIGHTS along main routes ===
	// North-South road
	for (float Y = -70000.f; Y <= 70000.f; Y += 10000.f)
	{
		SpawnLightPost(FVector(2000.f, Y, GroundZ), 1200.f,
			FLinearColor(0.6f, 0.8f, 1.f));
		SpawnLightPost(FVector(-2000.f, Y, GroundZ), 1200.f,
			FLinearColor(0.6f, 0.8f, 1.f));
	}

	// East-West road
	for (float X = -70000.f; X <= 70000.f; X += 10000.f)
	{
		if (FMath::Abs(X) < 3000.f) continue; // Skip center intersection
		SpawnLightPost(FVector(X, 2000.f, GroundZ), 1200.f,
			FLinearColor(0.6f, 0.8f, 1.f));
	}

	// === COMPOUND ACCENT LIGHTS ===
	// Red warning lights on towers
	SpawnLightPost(FVector(0.f, 85000.f, 5000.f), 0.f,
		FLinearColor(1.f, 0.1f, 0.05f)); // North tower top
	SpawnLightPost(FVector(85000.f, 3000.f, 6000.f), 0.f,
		FLinearColor(1.f, 0.1f, 0.05f)); // East tower
	SpawnLightPost(FVector(85000.f, -5000.f, 6000.f), 0.f,
		FLinearColor(1.f, 0.1f, 0.05f)); // East tower 2

	// Green landing pad lights at center hub
	for (int32 i = 0; i < 4; i++)
	{
		float Angle = (PI * 0.5f) * i + PI * 0.25f;
		FVector Pos(FMath::Cos(Angle) * 5500.f, FMath::Sin(Angle) * 4500.f, GroundZ);
		SpawnLightPost(Pos, 200.f, FLinearColor(0.1f, 1.f, 0.3f));
	}

	// === ANTENNAS ===
	SpawnAntenna(FVector(0.f, 0.f, 4000.f), 3000.f);         // Central hub roof
	SpawnAntenna(FVector(80000.f, 0.f, 3500.f), 2500.f);     // East compound
	SpawnAntenna(FVector(-80000.f, 0.f, 1800.f), 2000.f);    // West compound
	SpawnAntenna(FVector(0.f, 80000.f, 3000.f), 2000.f);     // North compound
	SpawnAntenna(FVector(120000.f, 120000.f, 2000.f), 1500.f); // NE outpost

	// === PIPES connecting compounds ===
	// Central hub to North compound (elevated pipe)
	SpawnPipeRun(FVector(0.f, 6000.f, 1500.f), FVector(0.f, 75000.f, 1500.f), 80.f);

	// Central hub to East compound
	SpawnPipeRun(FVector(6000.f, 0.f, 1200.f), FVector(75000.f, 0.f, 1200.f), 80.f);

	// Ground-level pipes near industrial areas
	SpawnPipeRun(FVector(-82000.f, -10000.f, 200.f), FVector(-82000.f, 10000.f, 200.f), 60.f);
	SpawnPipeRun(FVector(78000.f, -7000.f, 200.f), FVector(83000.f, -7000.f, 200.f), 50.f);

	// === SATELLITE DISHES on corner outposts ===
	float CornerDist = 120000.f;
	FVector CornerPositions[] = {
		{CornerDist, CornerDist, 2000.f}, {-CornerDist, CornerDist, 2000.f},
		{CornerDist, -CornerDist, 2000.f}, {-CornerDist, -CornerDist, 2000.f}
	};
	for (const FVector& CP : CornerPositions)
	{
		// Dish = flat cylinder tilted upward
		SpawnStaticMesh(CP + FVector(0.f, 0.f, 200.f),
			FVector(8.f, 8.f, 0.5f),
			FRotator(30.f, FMath::RandRange(0.f, 360.f), 0.f),
			CylinderMesh, FLinearColor(0.15f, 0.15f, 0.17f));

		// Dish support pillar
		SpawnStaticMesh(CP,
			FVector(1.f, 1.f, 2.f), FRotator::ZeroRotator,
			CylinderMesh, FLinearColor(0.1f, 0.1f, 0.12f));
	}

	// === ROAD INTERSECTION FURNITURE ===
	// Barrier bollards at key intersections
	struct FBollardRow { FVector Start; FVector End; int32 Count; };
	TArray<FBollardRow> Bollards = {
		// Roundabout perimeter
		{{14000.f, 0.f, GroundZ}, {10000.f, 10000.f, GroundZ}, 5},
		{{0.f, 14000.f, GroundZ}, {-10000.f, 10000.f, GroundZ}, 5},
		{{-14000.f, 0.f, GroundZ}, {-10000.f, -10000.f, GroundZ}, 5},
		{{0.f, -14000.f, GroundZ}, {10000.f, -10000.f, GroundZ}, 5},
	};
	for (const auto& BR : Bollards)
	{
		for (int32 j = 0; j < BR.Count; j++)
		{
			float T = (float)j / (float)(BR.Count - 1);
			FVector BPos = FMath::Lerp(BR.Start, BR.End, T);
			// Short glowing bollard
			SpawnStaticMesh(BPos + FVector(0.f, 0.f, 60.f),
				FVector(0.15f, 0.15f, 1.2f), FRotator::ZeroRotator,
				CylinderMesh, FLinearColor(0.1f, 0.1f, 0.12f));
			// Emissive top cap
			UStaticMeshComponent* Cap = SpawnStaticMesh(
				BPos + FVector(0.f, 0.f, 125.f),
				FVector(0.2f, 0.2f, 0.06f), FRotator::ZeroRotator,
				CylinderMesh, FLinearColor(0.8f, 0.5f, 0.1f));
			if (Cap && BaseMaterial)
			{
				UMaterialInstanceDynamic* CM = Cast<UMaterialInstanceDynamic>(
					Cap->GetMaterial(0));
				if (CM) CM->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(2.f, 1.2f, 0.3f));
			}
		}
	}

	// === CARGO CONTAINERS at compound loading areas ===
	struct FContainer { FVector Pos; float Yaw; FLinearColor Color; };
	TArray<FContainer> Containers = {
		// North compound dock
		{{-8000.f, 84000.f, 300.f},  10.f,  FLinearColor(0.15f, 0.06f, 0.03f)},
		{{-5000.f, 84000.f, 300.f},  5.f,   FLinearColor(0.03f, 0.08f, 0.12f)},
		{{-6500.f, 84000.f, 900.f},  8.f,   FLinearColor(0.1f, 0.1f, 0.04f)}, // stacked
		// East compound yard
		{{82000.f, 6000.f, 300.f},   85.f,  FLinearColor(0.04f, 0.1f, 0.05f)},
		{{82000.f, 8000.f, 300.f},   90.f,  FLinearColor(0.12f, 0.04f, 0.04f)},
		// West compound parade ground
		{{-78000.f, -8000.f, 300.f}, 0.f,   FLinearColor(0.06f, 0.06f, 0.08f)},
		{{-78000.f, 8000.f, 300.f},  0.f,   FLinearColor(0.08f, 0.06f, 0.03f)},
		// South compound
		{{6000.f, -83000.f, 300.f},  -25.f, FLinearColor(0.03f, 0.06f, 0.1f)},
		// Central hub
		{{6500.f, 3000.f, 300.f},    45.f,  FLinearColor(0.1f, 0.08f, 0.04f)},
	};
	for (const auto& C : Containers)
	{
		SpawnStaticMesh(C.Pos, FVector(12.f, 5.f, 6.f),
			FRotator(0.f, C.Yaw, 0.f), CubeMesh, C.Color);
	}

	// === CRASHED VEHICLE HULKS at road intersections ===
	// Flattened elongated boxes as wrecked transports
	auto SpawnWreck = [&](const FVector& Pos, float Yaw, float Tilt)
	{
		FLinearColor HullCol(0.06f, 0.06f, 0.07f);
		FRotator WRot(Tilt, Yaw, 0.f);
		// Main hull
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 150.f),
			FVector(8.f, 3.5f, 2.5f), WRot, CubeMesh, HullCol);
		// Cab/front section (slightly raised)
		SpawnStaticMesh(Pos + WRot.RotateVector(FVector(450.f, 0.f, 200.f)),
			FVector(3.f, 3.f, 3.f), WRot, CubeMesh, FLinearColor(0.05f, 0.05f, 0.06f));
		// Wheel axle stubs
		SpawnStaticMesh(Pos + WRot.RotateVector(FVector(-250.f, 0.f, 40.f)),
			FVector(0.6f, 4.f, 0.6f), WRot, CylinderMesh, FLinearColor(0.04f, 0.04f, 0.04f));
	};

	SpawnWreck(FVector(15000.f, 3000.f, GroundZ), 35.f, 3.f);
	SpawnWreck(FVector(-25000.f, -1500.f, GroundZ), -10.f, -2.f);
	SpawnWreck(FVector(45000.f, 45000.f, GroundZ), 70.f, 5.f);
	SpawnWreck(FVector(-60000.f, -50000.f, GroundZ), 140.f, -4.f);

	// === BARRICADES — tactical cover between compounds ===
	auto SpawnBarricade = [&](const FVector& Pos, float Yaw, float Width)
	{
		FRotator R(0.f, Yaw, 0.f);
		FLinearColor BarricadeCol(0.07f, 0.07f, 0.08f);
		FLinearColor StripeCol(0.6f, 0.4f, 0.05f);

		// Main barrier wall
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 120.f),
			FVector(Width / 100.f, 0.4f, 2.4f), R, CubeMesh, BarricadeCol);
		// Top rail
		SpawnStaticMesh(Pos + R.RotateVector(FVector(0.f, 0.f, 250.f)),
			FVector(Width / 100.f * 1.05f, 0.5f, 0.15f), R, CubeMesh,
			FLinearColor(0.09f, 0.09f, 0.1f));
		// Warning stripe
		SpawnStaticMesh(Pos + R.RotateVector(FVector(0.f, 22.f, 180.f)),
			FVector(Width / 100.f * 0.8f, 0.02f, 0.08f), R, CubeMesh, StripeCol);
		// Support feet
		for (float FX : {-Width * 0.4f, Width * 0.4f})
		{
			SpawnStaticMesh(Pos + R.RotateVector(FVector(FX, 0.f, 15.f)),
				FVector(0.6f, 1.f, 0.15f), R, CubeMesh, BarricadeCol);
		}
	};

	// Road checkpoint barricades
	SpawnBarricade(FVector(30000.f, 2000.f, GroundZ), 0.f, 600.f);
	SpawnBarricade(FVector(30000.f, -2000.f, GroundZ), 0.f, 600.f);
	SpawnBarricade(FVector(-30000.f, 1500.f, GroundZ), 10.f, 500.f);
	SpawnBarricade(FVector(-30000.f, -1500.f, GroundZ), -5.f, 500.f);
	// North approach
	SpawnBarricade(FVector(2500.f, 40000.f, GroundZ), 85.f, 700.f);
	SpawnBarricade(FVector(-2500.f, 40000.f, GroundZ), 95.f, 500.f);
	// South approach
	SpawnBarricade(FVector(1500.f, -45000.f, GroundZ), 90.f, 600.f);
	SpawnBarricade(FVector(-1500.f, -45000.f, GroundZ), 80.f, 600.f);

	// === DAMAGED WALL SECTIONS — battle-scarred concrete ===
	auto SpawnDamagedWall = [&](const FVector& Pos, float Yaw)
	{
		FRotator R(0.f, Yaw, 0.f);
		FLinearColor WallCol(0.06f, 0.06f, 0.065f);
		// Main wall section (lower half intact)
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 100.f),
			FVector(4.f, 0.3f, 2.f), R, CubeMesh, WallCol);
		// Broken top — jagged smaller blocks
		SpawnStaticMesh(Pos + R.RotateVector(FVector(-120.f, 0.f, 250.f)),
			FVector(1.5f, 0.3f, 0.8f), R + FRotator(0.f, 0.f, 5.f), CubeMesh, WallCol);
		SpawnStaticMesh(Pos + R.RotateVector(FVector(80.f, 0.f, 230.f)),
			FVector(1.f, 0.3f, 0.5f), R + FRotator(0.f, 0.f, -8.f), CubeMesh, WallCol);
		// Rubble chunks at base
		for (int32 D = 0; D < 3; D++)
		{
			float Ox = FMath::RandRange(-200.f, 200.f);
			float Oy = FMath::RandRange(30.f, 80.f);
			float S = FMath::RandRange(0.3f, 0.8f);
			SpawnStaticMesh(Pos + R.RotateVector(FVector(Ox, Oy, S * 25.f)),
				FVector(S, S * 0.7f, S * 0.5f),
				R + FRotator(FMath::RandRange(-10.f, 10.f), FMath::RandRange(0.f, 45.f), 0.f),
				CubeMesh, FLinearColor(0.05f, 0.05f, 0.055f));
		}
	};

	SpawnDamagedWall(FVector(20000.f, 20000.f, GroundZ), 30.f);
	SpawnDamagedWall(FVector(-40000.f, 30000.f, GroundZ), -15.f);
	SpawnDamagedWall(FVector(60000.f, -20000.f, GroundZ), 75.f);
	SpawnDamagedWall(FVector(-20000.f, -60000.f, GroundZ), 120.f);
	SpawnDamagedWall(FVector(35000.f, -55000.f, GroundZ), 45.f);

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Props, containers, barricades, and vehicles placed"));
}

void AExoLevelBuilder::SpawnLightPost(const FVector& Base, float Height, const FLinearColor& Color)
{
	if (Height > 0.f)
	{
		// Pole
		SpawnStaticMesh(Base + FVector(0.f, 0.f, Height * 0.5f),
			FVector(0.3f, 0.3f, Height / 100.f), FRotator::ZeroRotator,
			CylinderMesh, FLinearColor(0.12f, 0.12f, 0.14f));
	}

	// Light fixture at top
	FVector LightPos = Base + FVector(0.f, 0.f, Height + 50.f);

	UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
	Light->SetupAttachment(RootComponent);
	Light->SetWorldLocation(LightPos);
	Light->SetIntensity(3000.f);
	Light->SetAttenuationRadius(1500.f);
	Light->SetLightColor(Color);
	Light->CastShadows = false;
	Light->RegisterComponent();

	// Small visible bulb sphere
	SpawnStaticMesh(LightPos, FVector(0.15f, 0.15f, 0.1f),
		FRotator::ZeroRotator, SphereMesh, Color * 3.f);
}

void AExoLevelBuilder::SpawnAntenna(const FVector& Base, float Height)
{
	// Main mast
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height * 0.5f),
		FVector(0.2f, 0.2f, Height / 100.f), FRotator::ZeroRotator,
		CylinderMesh, FLinearColor(0.15f, 0.15f, 0.17f));

	// Blinking red light at top
	FVector TopPos = Base + FVector(0.f, 0.f, Height);
	UPointLightComponent* Beacon = NewObject<UPointLightComponent>(this);
	Beacon->SetupAttachment(RootComponent);
	Beacon->SetWorldLocation(TopPos);
	Beacon->SetIntensity(5000.f);
	Beacon->SetAttenuationRadius(3000.f);
	Beacon->SetLightColor(FLinearColor(1.f, 0.05f, 0.02f));
	Beacon->CastShadows = false;
	Beacon->RegisterComponent();

	// Cross arms (two horizontal bars)
	float ArmLength = Height * 0.15f;
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height * 0.7f),
		FVector(ArmLength / 100.f, 0.15f, 0.15f), FRotator::ZeroRotator,
		CubeMesh, FLinearColor(0.13f, 0.13f, 0.15f));
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height * 0.5f),
		FVector(0.15f, ArmLength * 0.8f / 100.f, 0.15f), FRotator::ZeroRotator,
		CubeMesh, FLinearColor(0.13f, 0.13f, 0.15f));
}

void AExoLevelBuilder::SpawnPipeRun(const FVector& Start, const FVector& End, float Radius)
{
	if (!CylinderMesh) return;

	FVector Mid = (Start + End) * 0.5f;
	FVector Dir = End - Start;
	float Length = Dir.Size();
	FRotator Rot = Dir.Rotation();
	// Cylinder default orientation is along Z, we need along X
	Rot.Pitch += 90.f;

	float ScaleXY = Radius / 50.f;
	float ScaleZ = Length / 100.f;

	SpawnStaticMesh(Mid, FVector(ScaleXY, ScaleXY, ScaleZ), Rot,
		CylinderMesh, FLinearColor(0.08f, 0.09f, 0.1f));

	// Support pylons every 20000 units
	int32 NumSupports = FMath::Max(1, FMath::FloorToInt(Length / 20000.f));
	FVector DirNorm = Dir.GetSafeNormal();
	for (int32 i = 1; i < NumSupports; i++)
	{
		float T = static_cast<float>(i) / NumSupports;
		FVector SupportPos = FMath::Lerp(Start, End, T);
		float SupportHeight = SupportPos.Z - GroundZ;
		if (SupportHeight > 100.f)
		{
			SpawnStaticMesh(
				FVector(SupportPos.X, SupportPos.Y, GroundZ + SupportHeight * 0.5f),
				FVector(0.4f, 0.4f, SupportHeight / 100.f), FRotator::ZeroRotator,
				CubeMesh, FLinearColor(0.1f, 0.1f, 0.12f));
		}
	}
}
