// ExoLevelBuilderPrimitives.cpp — Towers, walls, platforms, ramps
#include "Map/ExoLevelBuilder.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoLevelBuilder::SpawnTower(const FVector& Base, float Radius, float Height)
{
	if (!CylinderMesh) return;
	float ScaleXY = Radius / 50.f;
	float ScaleZ = Height / 100.f;
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height * 0.5f),
		FVector(ScaleXY, ScaleXY, ScaleZ), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.14f, 0.15f, 0.17f));

	// Horizontal ring bands — break up vertical surface
	int32 NumBands = FMath::Max(2, FMath::RoundToInt32(Height / 1500.f));
	for (int32 b = 1; b <= NumBands; b++)
	{
		float BZ = Height * b / (NumBands + 1);
		float BandR = Radius * 1.05f / 50.f;
		SpawnStaticMesh(Base + FVector(0.f, 0.f, BZ),
			FVector(BandR, BandR, 0.15f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.16f, 0.17f, 0.19f));
	}

	// Observation deck on top
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height),
		FVector(Radius * 1.8f / 100.f, Radius * 1.8f / 100.f, 0.3f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.11f, 0.12f, 0.14f));

	// Railing around observation deck (all 4 sides)
	float RailR = Radius * 1.8f;
	for (float Dir : {1.f, -1.f})
	{
		SpawnStaticMesh(Base + FVector(RailR * Dir, 0.f, Height + 50.f),
			FVector(0.1f, RailR * 2.f / 100.f, 0.5f), FRotator::ZeroRotator, CubeMesh,
			FLinearColor(0.16f, 0.16f, 0.18f));
		SpawnStaticMesh(Base + FVector(0.f, RailR * Dir, Height + 50.f),
			FVector(RailR * 2.f / 100.f, 0.1f, 0.5f), FRotator::ZeroRotator, CubeMesh,
			FLinearColor(0.16f, 0.16f, 0.18f));
	}

	// Beacon light at tower top (red warning light — localized)
	UPointLightComponent* Beacon = NewObject<UPointLightComponent>(this);
	Beacon->SetupAttachment(RootComponent);
	Beacon->SetWorldLocation(Base + FVector(0.f, 0.f, Height + 100.f));
	Beacon->SetIntensity(4000.f);
	Beacon->SetAttenuationRadius(Height * 0.6f);
	Beacon->SetLightColor(FLinearColor(1.f, 0.15f, 0.05f));
	Beacon->CastShadows = false;
	Beacon->RegisterComponent();

	// Beacon bulb (small emissive sphere)
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height + 80.f),
		FVector(0.25f, 0.25f, 0.25f), FRotator::ZeroRotator, SphereMesh,
		FLinearColor(1.f, 0.2f, 0.1f));
	UMaterialInterface* BulbEmissive = FExoMaterialFactory::GetEmissiveOpaque();
	if (BulbEmissive)
	{
		UStaticMeshComponent* Bulb = LevelMeshes.Last();
		UMaterialInstanceDynamic* BulbMat = UMaterialInstanceDynamic::Create(BulbEmissive, this);
		if (!BulbMat) { return; }
		BulbMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(4.f, 0.5f, 0.15f));
		Bulb->SetMaterial(0, BulbMat);
	}
}

void AExoLevelBuilder::SpawnWall(const FVector& Start, const FVector& End,
	float Height, float Thickness)
{
	FVector Mid = (Start + End) * 0.5f + FVector(0.f, 0.f, Height * 0.5f);
	FVector Dir = End - Start;
	float Length = Dir.Size();
	FRotator Rot = Dir.Rotation();

	FLinearColor WallCol(0.13f, 0.14f, 0.16f);
	SpawnStaticMesh(Mid,
		FVector(Length / 100.f, Thickness / 100.f, Height / 100.f), Rot, CubeMesh, WallCol);

	// Wall cap — metal capping strip along top
	SpawnStaticMesh(Mid + FVector(0.f, 0.f, Height * 0.5f + 10.f),
		FVector(Length / 100.f + 0.1f, Thickness / 100.f + 0.15f, 0.15f), Rot, CubeMesh,
		FLinearColor(0.16f, 0.17f, 0.19f));

	// Horizontal panel lines for detail
	for (float Frac : {0.33f, 0.66f})
	{
		float PZ = Height * Frac;
		SpawnStaticMesh(
			(Start + End) * 0.5f + FVector(0.f, 0.f, PZ),
			FVector(Length / 100.f * 0.98f, Thickness / 100.f + 0.04f, 0.03f),
			Rot, CubeMesh, FLinearColor(0.1f, 0.11f, 0.13f));
	}
}

void AExoLevelBuilder::SpawnPlatform(const FVector& Center, const FVector& Size)
{
	FLinearColor PlatColor(0.1f, 0.12f, 0.14f);
	SpawnStaticMesh(Center,
		FVector(Size.X / 100.f, Size.Y / 100.f, Size.Z / 100.f),
		FRotator::ZeroRotator, CubeMesh, PlatColor);

	// Edge trim strip — brighter metal lip around platform edge
	FLinearColor TrimCol(0.15f, 0.16f, 0.18f);
	float TopZ = Center.Z + Size.Z * 0.5f + 3.f;
	float HX = Size.X * 0.5f;
	float HY = Size.Y * 0.5f;
	SpawnStaticMesh(FVector(Center.X, Center.Y + HY, TopZ),
		FVector(Size.X / 100.f + 0.1f, 0.15f, 0.06f), FRotator::ZeroRotator, CubeMesh, TrimCol);
	SpawnStaticMesh(FVector(Center.X, Center.Y - HY, TopZ),
		FVector(Size.X / 100.f + 0.1f, 0.15f, 0.06f), FRotator::ZeroRotator, CubeMesh, TrimCol);

	// Support columns under elevated platforms (only if significantly above ground)
	float GroundClearance = Center.Z - Size.Z * 0.5f;
	if (GroundClearance > 200.f)
	{
		FLinearColor ColumnCol(0.12f, 0.13f, 0.15f);
		float ColRadius = FMath::Clamp(FMath::Min(Size.X, Size.Y) * 0.02f, 30.f, 100.f);
		int32 NumColsX = FMath::Max(2, FMath::RoundToInt32(Size.X / 3000.f) + 1);
		int32 NumColsY = FMath::Max(2, FMath::RoundToInt32(Size.Y / 3000.f) + 1);
		float ColH = GroundClearance;
		for (int32 cx = 0; cx < NumColsX; cx++)
		{
			float CX = Center.X - HX * 0.8f + (HX * 1.6f * cx / FMath::Max(1, NumColsX - 1));
			for (int32 cy = 0; cy < NumColsY; cy++)
			{
				float CY = Center.Y - HY * 0.8f + (HY * 1.6f * cy / FMath::Max(1, NumColsY - 1));
				SpawnStaticMesh(FVector(CX, CY, ColH * 0.5f),
					FVector(ColRadius / 50.f, ColRadius / 50.f, ColH / 100.f),
					FRotator::ZeroRotator, CylinderMesh, ColumnCol);
			}
		}
	}
}

void AExoLevelBuilder::SpawnRamp(const FVector& Base, float Length, float Height,
	float Width, float Yaw)
{
	int32 Steps = 5;
	float StepLen = Length / Steps;
	float StepH = Height / Steps;

	FRotator Rot(0.f, Yaw, 0.f);
	FVector Forward = Rot.RotateVector(FVector(1.f, 0.f, 0.f));
	FVector Right = Rot.RotateVector(FVector(0.f, 1.f, 0.f));

	for (int32 i = 0; i < Steps; i++)
	{
		FVector StepPos = Base + Forward * (StepLen * (i + 0.5f))
			+ FVector(0.f, 0.f, StepH * (i + 0.5f));
		// Step tread
		SpawnStaticMesh(StepPos,
			FVector(StepLen / 100.f, Width / 100.f, StepH / 100.f), Rot, CubeMesh,
			FLinearColor(0.11f, 0.12f, 0.13f));
		// Step edge highlight (lighter front edge)
		SpawnStaticMesh(StepPos + Forward * (StepLen * 0.48f) + FVector(0.f, 0.f, StepH * 0.48f),
			FVector(0.08f, Width / 100.f * 0.95f, 0.05f), Rot, CubeMesh,
			FLinearColor(0.15f, 0.16f, 0.18f));
	}

	// Side rails
	FLinearColor RailCol(0.15f, 0.16f, 0.18f);
	float RailH = 60.f;
	for (float Side : {1.f, -1.f})
	{
		FVector RailStart = Base + Right * (Width * 0.5f * Side) + FVector(0.f, 0.f, RailH);
		FVector RailEnd = Base + Forward * Length + Right * (Width * 0.5f * Side)
			+ FVector(0.f, 0.f, Height + RailH);
		FVector RailMid = (RailStart + RailEnd) * 0.5f;
		FVector RailDir = RailEnd - RailStart;
		float RailLen = RailDir.Size();
		FRotator RailRot = RailDir.Rotation();
		SpawnStaticMesh(RailMid,
			FVector(RailLen / 100.f, 0.06f, 0.06f), RailRot, CubeMesh, RailCol);
	}
}
