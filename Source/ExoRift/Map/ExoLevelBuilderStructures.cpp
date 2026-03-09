// ExoLevelBuilderStructures.cpp — Buildings, towers, walls, ramps, cover
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoLevelBuilder::BuildStructures()
{
	// === CENTRAL HUB — large command center ===
	SpawnBuilding(FVector(0.f, 0.f, 0.f), FVector(8000.f, 6000.f, 2500.f));
	SpawnBuilding(FVector(0.f, 0.f, 2500.f), FVector(4000.f, 3000.f, 1500.f)); // Second floor
	SpawnPlatform(FVector(5000.f, 0.f, 1500.f), FVector(3000.f, 2000.f, 100.f)); // Balcony
	SpawnRamp(FVector(6500.f, 0.f, 0.f), 2000.f, 1500.f, 500.f, 180.f);

	// === NORTH COMPOUND — industrial facility ===
	float NY = 80000.f;
	SpawnBuilding(FVector(-5000.f, NY, 0.f), FVector(6000.f, 4000.f, 3000.f), 15.f);
	SpawnBuilding(FVector(5000.f, NY, 0.f), FVector(4000.f, 5000.f, 2000.f));
	SpawnWall(FVector(-10000.f, NY - 4000.f, 0.f), FVector(10000.f, NY - 4000.f, 0.f), 1500.f);
	SpawnTower(FVector(0.f, NY + 5000.f, 0.f), 800.f, 5000.f);

	// === SOUTH COMPOUND — research labs ===
	float SY = -80000.f;
	SpawnBuilding(FVector(3000.f, SY, 0.f), FVector(5000.f, 7000.f, 2200.f), -10.f);
	SpawnBuilding(FVector(-6000.f, SY + 2000.f, 0.f), FVector(3500.f, 3500.f, 1800.f));
	SpawnBuilding(FVector(-6000.f, SY - 3000.f, 0.f), FVector(3500.f, 3000.f, 2000.f));
	SpawnPlatform(FVector(-1000.f, SY, 2500.f), FVector(5000.f, 1500.f, 100.f));

	// === EAST COMPOUND — power station ===
	float EX = 80000.f;
	SpawnBuilding(FVector(EX, -3000.f, 0.f), FVector(5000.f, 8000.f, 3500.f));
	SpawnTower(FVector(EX + 5000.f, 3000.f, 0.f), 1000.f, 6000.f);
	SpawnTower(FVector(EX + 5000.f, -5000.f, 0.f), 1000.f, 6000.f);
	SpawnWall(FVector(EX - 4000.f, -8000.f, 0.f), FVector(EX - 4000.f, 8000.f, 0.f), 1200.f);
	SpawnRamp(FVector(EX + 3000.f, 0.f, 0.f), 2500.f, 3500.f, 600.f, 270.f);

	// === WEST COMPOUND — barracks ===
	float WX = -80000.f;
	for (int32 i = 0; i < 4; i++)
	{
		float OffsetY = (i - 1.5f) * 5000.f;
		SpawnBuilding(FVector(WX, OffsetY, 0.f), FVector(3000.f, 3500.f, 1800.f));
	}
	SpawnWall(FVector(WX - 3000.f, -12000.f, 0.f), FVector(WX - 3000.f, 12000.f, 0.f), 1000.f);
	SpawnTower(FVector(WX - 3000.f, -12000.f, 0.f), 600.f, 4000.f);
	SpawnTower(FVector(WX - 3000.f, 12000.f, 0.f), 600.f, 4000.f);

	// === CORNER OUTPOSTS ===
	float CornerDist = 120000.f;
	FVector Corners[] = {
		{CornerDist, CornerDist, 0.f}, {-CornerDist, CornerDist, 0.f},
		{CornerDist, -CornerDist, 0.f}, {-CornerDist, -CornerDist, 0.f}
	};
	for (const FVector& C : Corners)
	{
		SpawnBuilding(C, FVector(4000.f, 4000.f, 2000.f), FMath::RandRange(0.f, 45.f));
		SpawnTower(C + FVector(3000.f, 3000.f, 0.f), 500.f, 3500.f);
	}

	// === SCATTERED STRUCTURES across the map ===
	struct FBldg { FVector Pos; FVector Size; float Rot; };
	TArray<FBldg> Scattered = {
		{{40000.f, 40000.f, 0.f},   {4000.f, 3000.f, 2000.f}, 30.f},
		{{-50000.f, 50000.f, 0.f},  {3000.f, 5000.f, 1500.f}, -20.f},
		{{60000.f, -40000.f, 0.f},  {3500.f, 3500.f, 2200.f}, 45.f},
		{{-30000.f, -50000.f, 0.f}, {5000.f, 3000.f, 1800.f}, 10.f},
		{{20000.f, -70000.f, 0.f},  {4000.f, 4000.f, 2500.f}, 0.f},
		{{-70000.f, -20000.f, 0.f}, {3000.f, 6000.f, 2000.f}, -15.f},
		{{50000.f, 70000.f, 0.f},   {3500.f, 4000.f, 1600.f}, 60.f},
		{{-100000.f, -80000.f, 0.f},{4500.f, 3500.f, 2400.f}, 25.f},
		{{90000.f, 50000.f, 0.f},   {3000.f, 3000.f, 3000.f}, 0.f},
		{{-20000.f, 120000.f, 0.f}, {5000.f, 4000.f, 2000.f}, -35.f},
	};
	for (const auto& B : Scattered)
	{
		SpawnBuilding(B.Pos, B.Size, B.Rot);
	}
}

void AExoLevelBuilder::SpawnBuilding(const FVector& Center, const FVector& Size, float Rotation)
{
	FLinearColor WallColor(0.08f, 0.09f, 0.11f);  // Dark metallic
	FLinearColor RoofColor(0.06f, 0.07f, 0.09f);   // Darker roof
	float WallThickness = 80.f;
	FRotator Rot(0.f, Rotation, 0.f);

	float HalfX = Size.X * 0.5f;
	float HalfY = Size.Y * 0.5f;
	float HalfZ = Size.Z * 0.5f;

	// Floor
	SpawnStaticMesh(Center + FVector(0.f, 0.f, 10.f),
		FVector(Size.X / 100.f, Size.Y / 100.f, 0.2f), Rot, CubeMesh,
		FLinearColor(0.07f, 0.07f, 0.08f));

	// Four walls (with door gaps in front wall)
	// Back wall
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY, HalfZ)),
		FVector(Size.X / 100.f, WallThickness / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);

	// Left wall
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(-HalfX, 0.f, HalfZ)),
		FVector(WallThickness / 100.f, Size.Y / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);

	// Right wall
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(HalfX, 0.f, HalfZ)),
		FVector(WallThickness / 100.f, Size.Y / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);

	// Front wall — two segments with door gap (1200 units wide)
	float DoorHalf = 600.f;
	float SegX_Left = (-HalfX + (-DoorHalf)) * 0.5f;
	float SegX_Right = (DoorHalf + HalfX) * 0.5f;
	float SegW_Left = (-DoorHalf) - (-HalfX);
	float SegW_Right = HalfX - DoorHalf;

	SpawnStaticMesh(Center + Rot.RotateVector(FVector(SegX_Left, HalfY, HalfZ)),
		FVector(SegW_Left / 100.f, WallThickness / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(SegX_Right, HalfY, HalfZ)),
		FVector(SegW_Right / 100.f, WallThickness / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);

	// Roof
	SpawnStaticMesh(Center + FVector(0.f, 0.f, Size.Z),
		FVector(Size.X / 100.f + 0.3f, Size.Y / 100.f + 0.3f, 0.3f), Rot, CubeMesh, RoofColor);
}

void AExoLevelBuilder::SpawnTower(const FVector& Base, float Radius, float Height)
{
	if (!CylinderMesh) return;
	float ScaleXY = Radius / 50.f; // Cylinder is 100 units diameter
	float ScaleZ = Height / 100.f;  // Cylinder is 100 units tall
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height * 0.5f),
		FVector(ScaleXY, ScaleXY, ScaleZ), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.1f, 0.11f, 0.13f));

	// Observation deck on top
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height),
		FVector(Radius * 1.8f / 100.f, Radius * 1.8f / 100.f, 0.3f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.07f, 0.08f, 0.1f));
}

void AExoLevelBuilder::SpawnWall(const FVector& Start, const FVector& End,
	float Height, float Thickness)
{
	FVector Mid = (Start + End) * 0.5f + FVector(0.f, 0.f, Height * 0.5f);
	FVector Dir = End - Start;
	float Length = Dir.Size();
	FRotator Rot = Dir.Rotation();

	SpawnStaticMesh(Mid,
		FVector(Length / 100.f, Thickness / 100.f, Height / 100.f), Rot, CubeMesh,
		FLinearColor(0.09f, 0.1f, 0.12f));
}

void AExoLevelBuilder::SpawnPlatform(const FVector& Center, const FVector& Size)
{
	SpawnStaticMesh(Center,
		FVector(Size.X / 100.f, Size.Y / 100.f, Size.Z / 100.f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.06f, 0.08f, 0.1f));
}

void AExoLevelBuilder::SpawnRamp(const FVector& Base, float Length, float Height,
	float Width, float Yaw)
{
	// Approximate ramp with 5 stepped segments
	int32 Steps = 5;
	float StepLen = Length / Steps;
	float StepH = Height / Steps;

	FRotator Rot(0.f, Yaw, 0.f);
	FVector Forward = Rot.RotateVector(FVector(1.f, 0.f, 0.f));

	for (int32 i = 0; i < Steps; i++)
	{
		FVector StepPos = Base + Forward * (StepLen * (i + 0.5f))
			+ FVector(0.f, 0.f, StepH * (i + 0.5f));
		SpawnStaticMesh(StepPos,
			FVector(StepLen / 100.f, Width / 100.f, StepH / 100.f), Rot, CubeMesh,
			FLinearColor(0.07f, 0.08f, 0.09f));
	}
}
