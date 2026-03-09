// ExoLevelBuilderStructures.cpp — Buildings, towers, walls, ramps, cover
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoLevelBuilder::BuildStructures()
{
	// === CENTRAL HUB — large command center ===
	SpawnBuilding(FVector(0.f, 0.f, 0.f), FVector(8000.f, 6000.f, 2500.f));
	SpawnBuilding(FVector(0.f, 0.f, 2500.f), FVector(4000.f, 3000.f, 1500.f));
	SpawnPlatform(FVector(5000.f, 0.f, 1500.f), FVector(3000.f, 2000.f, 100.f));
	SpawnRamp(FVector(6500.f, 0.f, 0.f), 2000.f, 1500.f, 500.f, 180.f);
	// Hub interior platforms
	SpawnPlatform(FVector(-2000.f, 0.f, 1200.f), FVector(2000.f, 1500.f, 80.f));
	SpawnRamp(FVector(-3500.f, 0.f, 0.f), 1500.f, 1200.f, 500.f, 0.f);

	// Hub comm tower (center roof)
	SpawnTower(FVector(0.f, 0.f, 4000.f), 400.f, 2000.f);
	SpawnAntenna(FVector(0.f, 0.f, 6000.f), 2000.f);

	// Landing pad (east of hub) — flat platform with emissive markings
	FVector PadCenter(8000.f, 0.f, 30.f);
	SpawnStaticMesh(PadCenter, FVector(30.f, 30.f, 0.3f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.06f, 0.06f, 0.07f));
	// Pad border stripes
	UStaticMeshComponent* PadBorderN = SpawnStaticMesh(
		PadCenter + FVector(0.f, 1500.f, 5.f), FVector(30.f, 0.1f, 0.05f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.1f, 0.4f, 0.1f));
	UStaticMeshComponent* PadBorderS = SpawnStaticMesh(
		PadCenter + FVector(0.f, -1500.f, 5.f), FVector(30.f, 0.1f, 0.05f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.1f, 0.4f, 0.1f));
	// Center H marking
	SpawnStaticMesh(PadCenter + FVector(0.f, 0.f, 5.f),
		FVector(0.15f, 6.f, 0.04f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.8f, 0.8f, 0.8f)); // Vertical bar of H
	SpawnStaticMesh(PadCenter + FVector(300.f, 0.f, 5.f),
		FVector(0.15f, 6.f, 0.04f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.8f, 0.8f, 0.8f)); // Second vertical
	SpawnStaticMesh(PadCenter + FVector(150.f, 0.f, 5.f),
		FVector(3.f, 0.15f, 0.04f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.8f, 0.8f, 0.8f)); // Cross bar

	// === NORTH COMPOUND — industrial facility ===
	float NY = 80000.f;
	// Main warehouse
	SpawnBuilding(FVector(-5000.f, NY, 0.f), FVector(6000.f, 4000.f, 3000.f), 15.f);
	SpawnBuilding(FVector(5000.f, NY, 0.f), FVector(4000.f, 5000.f, 2000.f));
	// Crane/gantry structure
	SpawnPlatform(FVector(0.f, NY, 4000.f), FVector(12000.f, 800.f, 200.f));
	SpawnStaticMesh(FVector(-5500.f, NY, 2000.f), FVector(0.5f, 0.5f, 40.f),
		FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.12f, 0.12f, 0.14f));
	SpawnStaticMesh(FVector(5500.f, NY, 2000.f), FVector(0.5f, 0.5f, 40.f),
		FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.12f, 0.12f, 0.14f));
	// Perimeter wall with walkway
	SpawnWall(FVector(-10000.f, NY - 4000.f, 0.f), FVector(10000.f, NY - 4000.f, 0.f), 1500.f);
	SpawnPlatform(FVector(0.f, NY - 4000.f, 1500.f), FVector(20000.f, 600.f, 80.f));
	SpawnTower(FVector(0.f, NY + 5000.f, 0.f), 800.f, 5000.f);
	// Storage tanks
	for (int32 i = 0; i < 3; i++)
	{
		float TankX = -8000.f + i * 4000.f;
		SpawnStaticMesh(FVector(TankX, NY + 6000.f, 1500.f),
			FVector(10.f, 10.f, 30.f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.1f, 0.1f, 0.12f));
	}

	// === SOUTH COMPOUND — research labs ===
	float SY = -80000.f;
	SpawnBuilding(FVector(3000.f, SY, 0.f), FVector(5000.f, 7000.f, 2200.f), -10.f);
	SpawnBuilding(FVector(-6000.f, SY + 2000.f, 0.f), FVector(3500.f, 3500.f, 1800.f));
	SpawnBuilding(FVector(-6000.f, SY - 3000.f, 0.f), FVector(3500.f, 3000.f, 2000.f));
	// Elevated walkway between labs
	SpawnPlatform(FVector(-1000.f, SY, 2500.f), FVector(5000.f, 1500.f, 100.f));
	// Research dome (large sphere slice)
	SpawnStaticMesh(FVector(3000.f, SY, 2200.f), FVector(25.f, 25.f, 12.f),
		FRotator::ZeroRotator, SphereMesh, FLinearColor(0.08f, 0.1f, 0.14f));
	// Containment area
	SpawnWall(FVector(-9000.f, SY - 5000.f, 0.f), FVector(-9000.f, SY + 5000.f, 0.f), 1800.f);
	SpawnWall(FVector(-9000.f, SY + 5000.f, 0.f), FVector(9000.f, SY + 5000.f, 0.f), 1800.f);

	// === EAST COMPOUND — power station ===
	float EX = 80000.f;
	SpawnBuilding(FVector(EX, -3000.f, 0.f), FVector(5000.f, 8000.f, 3500.f));
	SpawnTower(FVector(EX + 5000.f, 3000.f, 0.f), 1000.f, 6000.f);
	SpawnTower(FVector(EX + 5000.f, -5000.f, 0.f), 1000.f, 6000.f);
	SpawnWall(FVector(EX - 4000.f, -8000.f, 0.f), FVector(EX - 4000.f, 8000.f, 0.f), 1200.f);
	SpawnRamp(FVector(EX + 3000.f, 0.f, 0.f), 2500.f, 3500.f, 600.f, 270.f);
	// Power pylons
	for (int32 i = 0; i < 4; i++)
	{
		float PylonY = -6000.f + i * 4000.f;
		// H-frame pylon
		SpawnStaticMesh(FVector(EX + 8000.f, PylonY, 2000.f),
			FVector(0.3f, 0.3f, 40.f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.12f, 0.12f, 0.14f));
		SpawnStaticMesh(FVector(EX + 8400.f, PylonY, 2000.f),
			FVector(0.3f, 0.3f, 40.f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.12f, 0.12f, 0.14f));
		SpawnStaticMesh(FVector(EX + 8200.f, PylonY, 3800.f),
			FVector(5.f, 0.3f, 0.15f), FRotator::ZeroRotator, CubeMesh,
			FLinearColor(0.12f, 0.12f, 0.14f));
	}
	// Cooling towers
	SpawnStaticMesh(FVector(EX - 5000.f, 5000.f, 1500.f),
		FVector(15.f, 15.f, 30.f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.09f, 0.1f, 0.11f));
	SpawnStaticMesh(FVector(EX - 5000.f, -5000.f, 1500.f),
		FVector(15.f, 15.f, 30.f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.09f, 0.1f, 0.11f));

	// === WEST COMPOUND — barracks ===
	float WX = -80000.f;
	for (int32 i = 0; i < 4; i++)
	{
		float OffsetY = (i - 1.5f) * 5000.f;
		SpawnBuilding(FVector(WX, OffsetY, 0.f), FVector(3000.f, 3500.f, 1800.f));
	}
	// Central parade ground platform
	SpawnPlatform(FVector(WX + 5000.f, 0.f, 50.f), FVector(6000.f, 12000.f, 50.f));
	SpawnWall(FVector(WX - 3000.f, -12000.f, 0.f), FVector(WX - 3000.f, 12000.f, 0.f), 1000.f);
	SpawnTower(FVector(WX - 3000.f, -12000.f, 0.f), 600.f, 4000.f);
	SpawnTower(FVector(WX - 3000.f, 12000.f, 0.f), 600.f, 4000.f);
	// Guard posts at entrance
	SpawnBuilding(FVector(WX + 4000.f, -6000.f, 0.f), FVector(1500.f, 1500.f, 2500.f));
	SpawnBuilding(FVector(WX + 4000.f, 6000.f, 0.f), FVector(1500.f, 1500.f, 2500.f));

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
		// Small bunker nearby
		SpawnBuilding(C + FVector(-2500.f, 1500.f, 0.f), FVector(2000.f, 2000.f, 1200.f));
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
	FLinearColor WallColor(0.08f, 0.09f, 0.11f);
	FLinearColor RoofColor(0.06f, 0.07f, 0.09f);
	FLinearColor TrimColor(0.12f, 0.14f, 0.18f);
	float WallThickness = 80.f;
	FRotator Rot(0.f, Rotation, 0.f);

	float HalfX = Size.X * 0.5f;
	float HalfY = Size.Y * 0.5f;
	float HalfZ = Size.Z * 0.5f;

	// Floor
	SpawnStaticMesh(Center + FVector(0.f, 0.f, 10.f),
		FVector(Size.X / 100.f, Size.Y / 100.f, 0.2f), Rot, CubeMesh,
		FLinearColor(0.07f, 0.07f, 0.08f));

	// Four walls (with door gap in front wall)
	// Back wall
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY, HalfZ)),
		FVector(Size.X / 100.f, WallThickness / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);

	// Left wall
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(-HalfX, 0.f, HalfZ)),
		FVector(WallThickness / 100.f, Size.Y / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);

	// Right wall
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(HalfX, 0.f, HalfZ)),
		FVector(WallThickness / 100.f, Size.Y / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);

	// Front wall — two segments with door gap
	float DoorHalf = 600.f;
	float SegX_Left = (-HalfX + (-DoorHalf)) * 0.5f;
	float SegX_Right = (DoorHalf + HalfX) * 0.5f;
	float SegW_Left = (-DoorHalf) - (-HalfX);
	float SegW_Right = HalfX - DoorHalf;

	SpawnStaticMesh(Center + Rot.RotateVector(FVector(SegX_Left, HalfY, HalfZ)),
		FVector(SegW_Left / 100.f, WallThickness / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(SegX_Right, HalfY, HalfZ)),
		FVector(SegW_Right / 100.f, WallThickness / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);

	// Door frame header
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY, Size.Z - 200.f)),
		FVector(DoorHalf * 2.f / 100.f, WallThickness / 100.f + 0.1f, 2.f), Rot, CubeMesh, TrimColor);

	// Roof
	SpawnStaticMesh(Center + FVector(0.f, 0.f, Size.Z),
		FVector(Size.X / 100.f + 0.3f, Size.Y / 100.f + 0.3f, 0.3f), Rot, CubeMesh, RoofColor);

	// Roof edge trim
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 15.f, Size.Z + 20.f)),
		FVector(Size.X / 100.f + 0.4f, 0.1f, 0.4f), Rot, CubeMesh, TrimColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY - 15.f, Size.Z + 20.f)),
		FVector(Size.X / 100.f + 0.4f, 0.1f, 0.4f), Rot, CubeMesh, TrimColor);

	// Emissive accent strips (sci-fi window line effect on front/back walls)
	float StripZ = Size.Z * 0.6f;
	FLinearColor StripCol(0.05f, 0.15f, 0.3f); // Cool blue accent
	UStaticMeshComponent* FrontStrip = SpawnStaticMesh(
		Center + Rot.RotateVector(FVector(0.f, HalfY + 1.f, StripZ)),
		FVector(Size.X / 100.f * 0.8f, 0.02f, 0.15f), Rot, CubeMesh, StripCol);
	if (FrontStrip && BaseMaterial)
	{
		UMaterialInstanceDynamic* StripMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		StripMat->SetVectorParameterValue(TEXT("BaseColor"), StripCol);
		StripMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.1f, 0.5f, 1.2f));
		FrontStrip->SetMaterial(0, StripMat);
	}
	UStaticMeshComponent* BackStrip = SpawnStaticMesh(
		Center + Rot.RotateVector(FVector(0.f, -HalfY - 1.f, StripZ)),
		FVector(Size.X / 100.f * 0.8f, 0.02f, 0.15f), Rot, CubeMesh, StripCol);
	if (BackStrip && BaseMaterial)
	{
		UMaterialInstanceDynamic* StripMat2 = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		StripMat2->SetVectorParameterValue(TEXT("BaseColor"), StripCol);
		StripMat2->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.1f, 0.5f, 1.2f));
		BackStrip->SetMaterial(0, StripMat2);
	}

	// Side wall emissive strips (window line effect)
	UStaticMeshComponent* LeftStrip = SpawnStaticMesh(
		Center + Rot.RotateVector(FVector(-HalfX - 1.f, 0.f, StripZ)),
		FVector(0.02f, Size.Y / 100.f * 0.7f, 0.12f), Rot, CubeMesh, StripCol);
	if (LeftStrip && BaseMaterial)
	{
		UMaterialInstanceDynamic* LSM = Cast<UMaterialInstanceDynamic>(
			LeftStrip->GetMaterial(0));
		if (LSM) LSM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.08f, 0.35f, 0.9f));
	}
	UStaticMeshComponent* RightStrip = SpawnStaticMesh(
		Center + Rot.RotateVector(FVector(HalfX + 1.f, 0.f, StripZ)),
		FVector(0.02f, Size.Y / 100.f * 0.7f, 0.12f), Rot, CubeMesh, StripCol);
	if (RightStrip && BaseMaterial)
	{
		UMaterialInstanceDynamic* RSM = Cast<UMaterialInstanceDynamic>(
			RightStrip->GetMaterial(0));
		if (RSM) RSM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.08f, 0.35f, 0.9f));
	}

	// Doorway light (warm amber glow at entrance)
	UPointLightComponent* DoorLight = NewObject<UPointLightComponent>(this);
	DoorLight->SetupAttachment(RootComponent);
	DoorLight->SetWorldLocation(
		Center + Rot.RotateVector(FVector(0.f, HalfY + 50.f, Size.Z - 300.f)));
	DoorLight->SetIntensity(2500.f);
	DoorLight->SetAttenuationRadius(800.f);
	DoorLight->SetLightColor(FLinearColor(0.8f, 0.6f, 0.3f));
	DoorLight->CastShadows = false;
	DoorLight->RegisterComponent();

	// Interior light
	UPointLightComponent* IntLight = NewObject<UPointLightComponent>(this);
	IntLight->SetupAttachment(RootComponent);
	IntLight->SetWorldLocation(Center + FVector(0.f, 0.f, Size.Z - 100.f));
	IntLight->SetIntensity(2000.f);
	IntLight->SetAttenuationRadius(FMath::Max(Size.X, Size.Y) * 0.6f);
	IntLight->SetLightColor(FLinearColor(0.5f, 0.6f, 0.8f));
	IntLight->CastShadows = false;
	IntLight->RegisterComponent();

	// Rooftop equipment (AC unit / vent box on larger buildings)
	if (Size.X > 3000.f && Size.Y > 3000.f)
	{
		FVector VentPos = Center + FVector(
			FMath::RandRange(-HalfX * 0.3f, HalfX * 0.3f),
			FMath::RandRange(-HalfY * 0.3f, HalfY * 0.3f),
			Size.Z + 60.f);
		SpawnStaticMesh(VentPos, FVector(1.5f, 1.f, 0.6f), Rot, CubeMesh,
			FLinearColor(0.08f, 0.08f, 0.1f));
		// Vent grille
		SpawnStaticMesh(VentPos + FVector(0.f, 0.f, 35.f),
			FVector(1.3f, 0.8f, 0.05f), Rot, CubeMesh,
			FLinearColor(0.06f, 0.06f, 0.07f));
	}
}

void AExoLevelBuilder::SpawnTower(const FVector& Base, float Radius, float Height)
{
	if (!CylinderMesh) return;
	float ScaleXY = Radius / 50.f;
	float ScaleZ = Height / 100.f;
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height * 0.5f),
		FVector(ScaleXY, ScaleXY, ScaleZ), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.1f, 0.11f, 0.13f));

	// Observation deck on top
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height),
		FVector(Radius * 1.8f / 100.f, Radius * 1.8f / 100.f, 0.3f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.07f, 0.08f, 0.1f));

	// Railing around observation deck
	float RailR = Radius * 1.8f;
	SpawnStaticMesh(Base + FVector(RailR, 0.f, Height + 50.f),
		FVector(0.1f, RailR * 2.f / 100.f, 0.5f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.12f, 0.12f, 0.14f));
	SpawnStaticMesh(Base + FVector(-RailR, 0.f, Height + 50.f),
		FVector(0.1f, RailR * 2.f / 100.f, 0.5f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.12f, 0.12f, 0.14f));

	// Beacon light at tower top (red warning light)
	UPointLightComponent* Beacon = NewObject<UPointLightComponent>(this);
	Beacon->SetupAttachment(RootComponent);
	Beacon->SetWorldLocation(Base + FVector(0.f, 0.f, Height + 100.f));
	Beacon->SetIntensity(5000.f);
	Beacon->SetAttenuationRadius(Height * 0.8f);
	Beacon->SetLightColor(FLinearColor(1.f, 0.15f, 0.05f));
	Beacon->CastShadows = false;
	Beacon->RegisterComponent();

	// Beacon bulb (small emissive sphere)
	UStaticMeshComponent* Bulb = SpawnStaticMesh(
		Base + FVector(0.f, 0.f, Height + 80.f),
		FVector(0.25f, 0.25f, 0.25f), FRotator::ZeroRotator, SphereMesh,
		FLinearColor(1.f, 0.2f, 0.1f));
	if (Bulb && BaseMaterial)
	{
		UMaterialInstanceDynamic* BulbMat = Cast<UMaterialInstanceDynamic>(
			Bulb->GetMaterial(0));
		if (BulbMat)
		{
			BulbMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(5.f, 0.5f, 0.2f));
		}
	}
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
