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

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Props and accent lighting placed"));
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
