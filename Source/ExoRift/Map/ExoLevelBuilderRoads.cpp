// ExoLevelBuilderRoads.cpp — Roads, bridges, water, foliage details
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildRoads()
{
	FLinearColor RoadColor(0.03f, 0.03f, 0.04f);     // Dark asphalt
	FLinearColor LineColor(0.5f, 0.45f, 0.1f);         // Yellow road lines
	float RoadWidth = 3000.f;

	// === MAIN CROSS ROADS ===
	// North-South highway
	SpawnRoadSegment(FVector(0.f, -MapHalfSize * 0.9f, GroundZ),
		FVector(0.f, MapHalfSize * 0.9f, GroundZ), RoadWidth);

	// East-West highway
	SpawnRoadSegment(FVector(-MapHalfSize * 0.9f, 0.f, GroundZ),
		FVector(MapHalfSize * 0.9f, 0.f, GroundZ), RoadWidth);

	// === COMPOUND ACCESS ROADS ===
	// NE diagonal to NE outpost
	SpawnRoadSegment(FVector(10000.f, 10000.f, GroundZ),
		FVector(100000.f, 100000.f, GroundZ), 2000.f);

	// SW diagonal to SW outpost
	SpawnRoadSegment(FVector(-10000.f, -10000.f, GroundZ),
		FVector(-100000.f, -100000.f, GroundZ), 2000.f);

	// NW diagonal
	SpawnRoadSegment(FVector(-10000.f, 10000.f, GroundZ),
		FVector(-100000.f, 100000.f, GroundZ), 2000.f);

	// SE diagonal
	SpawnRoadSegment(FVector(10000.f, -10000.f, GroundZ),
		FVector(100000.f, -100000.f, GroundZ), 2000.f);

	// === BRIDGES over terrain patches ===
	SpawnBridge(FVector(-80000.f, -55000.f, GroundZ), FVector(-80000.f, -65000.f, GroundZ),
		2500.f, 400.f);
	SpawnBridge(FVector(55000.f, 80000.f, GroundZ), FVector(65000.f, 80000.f, GroundZ),
		2500.f, 350.f);

	// === CENTER ROUNDABOUT ===
	int32 RoundaboutSegments = 16;
	float RoundaboutRadius = 12000.f;
	for (int32 i = 0; i < RoundaboutSegments; i++)
	{
		float A1 = (2.f * PI * i) / RoundaboutSegments;
		float A2 = (2.f * PI * (i + 1)) / RoundaboutSegments;
		FVector P1(FMath::Cos(A1) * RoundaboutRadius, FMath::Sin(A1) * RoundaboutRadius, GroundZ + 5.f);
		FVector P2(FMath::Cos(A2) * RoundaboutRadius, FMath::Sin(A2) * RoundaboutRadius, GroundZ + 5.f);
		SpawnRoadSegment(P1, P2, 2000.f);
	}

	// Center island (raised circular platform)
	SpawnStaticMesh(FVector(0.f, 0.f, GroundZ + 30.f),
		FVector(80.f, 80.f, 0.3f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.05f, 0.06f, 0.04f));

	// Emissive ring around roundabout center
	UStaticMeshComponent* CenterRing = SpawnStaticMesh(
		FVector(0.f, 0.f, GroundZ + 35.f),
		FVector(85.f, 85.f, 0.06f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.05f, 0.2f, 0.4f));
	if (CenterRing)
	{
		UMaterialInterface* RingEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(RingEmissiveMat, this);
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.08f, 0.3f, 0.6f));
		CenterRing->SetMaterial(0, RingMat);
	}

	// Center monument — tall pylon with glow
	SpawnStaticMesh(FVector(0.f, 0.f, GroundZ + 500.f),
		FVector(2.f, 2.f, 10.f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.07f, 0.07f, 0.09f));

	UPointLightComponent* MonumentLight = NewObject<UPointLightComponent>(this);
	MonumentLight->SetupAttachment(RootComponent);
	MonumentLight->SetWorldLocation(FVector(0.f, 0.f, GroundZ + 1000.f));
	MonumentLight->SetIntensity(8000.f);
	MonumentLight->SetAttenuationRadius(3000.f);
	MonumentLight->SetLightColor(FLinearColor(0.1f, 0.4f, 0.8f));
	MonumentLight->CastShadows = false;
	MonumentLight->RegisterComponent();

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Roads and bridges placed"));
}

void AExoLevelBuilder::SpawnRoadSegment(const FVector& Start, const FVector& End, float Width)
{
	FVector Mid = (Start + End) * 0.5f;
	FVector Dir = End - Start;
	float Length = Dir.Size();
	FRotator Rot = Dir.Rotation();

	// Road surface
	FVector RoadPos = Mid + FVector(0.f, 0.f, 2.f);
	SpawnStaticMesh(RoadPos,
		FVector(Length / 100.f, Width / 100.f, 0.05f), Rot, CubeMesh,
		FLinearColor(0.03f, 0.03f, 0.04f));

	// Edge lines — continuous white strips on both sides
	FVector Right = FRotationMatrix(Rot).GetScaledAxis(EAxis::Y);
	FVector EdgeOffset = Right * Width * 0.48f;
	FLinearColor EdgeColor(0.12f, 0.12f, 0.15f);
	SpawnStaticMesh(RoadPos + EdgeOffset + FVector(0.f, 0.f, 2.f),
		FVector(Length / 100.f, 0.15f, 0.03f), Rot, CubeMesh, EdgeColor);
	SpawnStaticMesh(RoadPos - EdgeOffset + FVector(0.f, 0.f, 2.f),
		FVector(Length / 100.f, 0.15f, 0.03f), Rot, CubeMesh, EdgeColor);

	// Center line (yellow dashes)
	float DashLen = 500.f;
	float DashGap = 500.f;
	FVector DirNorm = Dir.GetSafeNormal();
	float Progress = DashGap;
	while (Progress + DashLen < Length)
	{
		FVector DashPos = Start + DirNorm * (Progress + DashLen * 0.5f) + FVector(0.f, 0.f, 4.f);
		SpawnStaticMesh(DashPos,
			FVector(DashLen / 100.f, 0.3f, 0.03f), Rot, CubeMesh,
			FLinearColor(0.5f, 0.45f, 0.1f));
		Progress += DashLen + DashGap;
	}
}

void AExoLevelBuilder::SpawnBridge(const FVector& Start, const FVector& End,
	float Width, float Height)
{
	FVector Mid = (Start + End) * 0.5f;
	FVector Dir = End - Start;
	float Length = Dir.Size();
	FRotator Rot = Dir.Rotation();

	// Bridge deck
	FVector DeckPos = Mid + FVector(0.f, 0.f, Height);
	SpawnStaticMesh(DeckPos,
		FVector(Length / 100.f, Width / 100.f, 0.3f), Rot, CubeMesh,
		FLinearColor(0.07f, 0.07f, 0.09f));

	// Guard rails
	FVector Right = FRotationMatrix(Rot).GetScaledAxis(EAxis::Y) * Width * 0.5f;
	SpawnStaticMesh(DeckPos + Right + FVector(0.f, 0.f, 60.f),
		FVector(Length / 100.f, 0.1f, 0.6f), Rot, CubeMesh,
		FLinearColor(0.12f, 0.12f, 0.14f));
	SpawnStaticMesh(DeckPos - Right + FVector(0.f, 0.f, 60.f),
		FVector(Length / 100.f, 0.1f, 0.6f), Rot, CubeMesh,
		FLinearColor(0.12f, 0.12f, 0.14f));

	// Support columns
	int32 NumSupports = FMath::Max(2, FMath::CeilToInt(Length / 5000.f));
	FVector DirNorm = Dir.GetSafeNormal();
	for (int32 i = 0; i < NumSupports; i++)
	{
		float T = (i + 0.5f) / NumSupports;
		FVector ColPos = FMath::Lerp(Start, End, T);
		SpawnStaticMesh(FVector(ColPos.X, ColPos.Y, Height * 0.5f),
			FVector(1.f, 1.f, Height / 100.f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.1f, 0.1f, 0.12f));
	}
}

void AExoLevelBuilder::BuildWaterFeatures()
{
	FLinearColor WaterColor(0.02f, 0.05f, 0.1f); // Dark water

	// River channel running NE to SW through terrain patches
	float RiverWidth = 4000.f;
	int32 RiverPoints = 8;
	FVector RiverPath[] = {
		{-150000.f, 150000.f, GroundZ - 100.f},
		{-100000.f, 100000.f, GroundZ - 100.f},
		{-60000.f, 70000.f, GroundZ - 100.f},
		{-30000.f, 40000.f, GroundZ - 100.f},
		{20000.f, -20000.f, GroundZ - 100.f},
		{60000.f, -50000.f, GroundZ - 100.f},
		{100000.f, -80000.f, GroundZ - 100.f},
		{150000.f, -120000.f, GroundZ - 100.f},
	};

	for (int32 i = 0; i < RiverPoints - 1; i++)
	{
		FVector S = RiverPath[i];
		FVector E = RiverPath[i + 1];
		FVector Mid = (S + E) * 0.5f;
		FVector Dir = E - S;
		float Len = Dir.Size();
		FRotator Rot = Dir.Rotation();

		// Water surface with subtle emissive sheen
		UStaticMeshComponent* Water = SpawnStaticMesh(Mid,
			FVector(Len / 100.f, RiverWidth / 100.f, 0.05f),
			Rot, CubeMesh, WaterColor);
		if (Water)
		{
			UMaterialInterface* WaterEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
			UMaterialInstanceDynamic* WMat = UMaterialInstanceDynamic::Create(WaterEmissiveMat, this);
			WMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.005f, 0.015f, 0.03f));
			Water->SetMaterial(0, WMat);
		}
	}

	// Industrial cooling pond near East compound
	UStaticMeshComponent* Pond = SpawnStaticMesh(
		FVector(85000.f, 10000.f, GroundZ - 50.f),
		FVector(60.f, 40.f, 0.1f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.03f, 0.06f, 0.08f));
	if (Pond)
	{
		UMaterialInterface* PondEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* PMat = UMaterialInstanceDynamic::Create(PondEmissiveMat, this);
		PMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.008f, 0.02f, 0.03f));
		Pond->SetMaterial(0, PMat);
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Water features placed"));
}

void AExoLevelBuilder::BuildFoliage()
{
	// Sci-fi "foliage" — glowing crystal formations and dead tree stumps

	FLinearColor CrystalColor(0.05f, 0.15f, 0.3f);  // Blue crystal
	FLinearColor StumpColor(0.06f, 0.05f, 0.04f);     // Dead wood

	// Crystal clusters near water and hazard zones
	struct FCrystalCluster { FVector Pos; int32 Count; float Spread; };
	TArray<FCrystalCluster> Clusters = {
		{{-50000.f, 50000.f, GroundZ}, 6, 3000.f},   // Near radiation zone
		{{60000.f, -60000.f, GroundZ}, 4, 2000.f},    // Near electric zone
		{{-90000.f, 90000.f, GroundZ}, 5, 4000.f},    // River bank
		{{30000.f, -30000.f, GroundZ}, 3, 2000.f},    // River crossing
		{{-110000.f, -70000.f, GroundZ}, 4, 3000.f},  // Near toxic zone
	};

	for (const auto& Cluster : Clusters)
	{
		for (int32 i = 0; i < Cluster.Count; i++)
		{
			FVector Offset(
				FMath::RandRange(-Cluster.Spread, Cluster.Spread),
				FMath::RandRange(-Cluster.Spread, Cluster.Spread),
				0.f);
			FVector Pos = Cluster.Pos + Offset;
			float CHeight = FMath::RandRange(200.f, 800.f);
			float CWidth = FMath::RandRange(40.f, 120.f);

			// Crystal shard (tilted elongated cube with emissive glow)
			UStaticMeshComponent* Crystal = SpawnStaticMesh(
				Pos + FVector(0.f, 0.f, CHeight * 0.4f),
				FVector(CWidth / 100.f, CWidth * 0.6f / 100.f, CHeight / 100.f),
				FRotator(FMath::RandRange(-20.f, 20.f), FMath::RandRange(0.f, 360.f),
					FMath::RandRange(-15.f, 15.f)),
				CubeMesh, CrystalColor);
			if (Crystal)
			{
				UMaterialInterface* CrysEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
				UMaterialInstanceDynamic* CrysMat = UMaterialInstanceDynamic::Create(CrysEmissiveMat, this);
				CrysMat->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(0.1f, 0.3f, 0.6f));
				Crystal->SetMaterial(0, CrysMat);
			}
		}

		// Ambient glow at cluster center
		UPointLightComponent* Glow = NewObject<UPointLightComponent>(this);
		Glow->SetupAttachment(RootComponent);
		Glow->SetWorldLocation(Cluster.Pos + FVector(0.f, 0.f, 200.f));
		Glow->SetIntensity(2000.f);
		Glow->SetAttenuationRadius(Cluster.Spread * 0.8f);
		Glow->SetLightColor(FLinearColor(0.1f, 0.3f, 0.8f));
		Glow->CastShadows = false;
		Glow->RegisterComponent();
	}

	// Dead tree stumps scattered across open areas
	for (int32 i = 0; i < 30; i++)
	{
		FVector Pos(
			FMath::RandRange(-MapHalfSize * 0.7f, MapHalfSize * 0.7f),
			FMath::RandRange(-MapHalfSize * 0.7f, MapHalfSize * 0.7f),
			GroundZ);

		float StumpH = FMath::RandRange(100.f, 400.f);
		float StumpR = FMath::RandRange(30.f, 80.f);

		SpawnStaticMesh(Pos + FVector(0.f, 0.f, StumpH * 0.5f),
			FVector(StumpR / 50.f, StumpR / 50.f, StumpH / 100.f),
			FRotator(FMath::RandRange(-5.f, 5.f), FMath::RandRange(0.f, 360.f), 0.f),
			CylinderMesh, StumpColor);
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Foliage and crystal formations placed"));
}
