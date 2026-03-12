// ExoLevelBuilderRoads.cpp — Roads, bridges, water, foliage details
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildRoads()
{
	FLinearColor RoadColor(0.09f, 0.09f, 0.1f);       // Asphalt grey
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
	SpawnRoadSegment(FVector(2000.f, 2000.f, GroundZ),
		FVector(20000.f, 20000.f, GroundZ), 2000.f);

	// SW diagonal to SW outpost
	SpawnRoadSegment(FVector(-2000.f, -2000.f, GroundZ),
		FVector(-20000.f, -20000.f, GroundZ), 2000.f);

	// NW diagonal
	SpawnRoadSegment(FVector(-2000.f, 2000.f, GroundZ),
		FVector(-20000.f, 20000.f, GroundZ), 2000.f);

	// SE diagonal
	SpawnRoadSegment(FVector(2000.f, -2000.f, GroundZ),
		FVector(20000.f, -20000.f, GroundZ), 2000.f);

	// === BRIDGES over terrain patches ===
	SpawnBridge(FVector(-16000.f, -11000.f, GroundZ), FVector(-16000.f, -13000.f, GroundZ),
		2500.f, 400.f);
	SpawnBridge(FVector(11000.f, 16000.f, GroundZ), FVector(13000.f, 16000.f, GroundZ),
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
		FLinearColor(0.09f, 0.1f, 0.07f));

	// Emissive ring around roundabout center
	UStaticMeshComponent* CenterRing = SpawnStaticMesh(
		FVector(0.f, 0.f, GroundZ + 35.f),
		FVector(85.f, 85.f, 0.06f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.05f, 0.2f, 0.4f));
	if (CenterRing)
	{
		UMaterialInterface* RingEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(RingEmissiveMat, this);
		if (!RingMat) { return; }
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.1f, 0.4f, 0.7f));
		CenterRing->SetMaterial(0, RingMat);
	}

	// Center monument — ExoRift obelisk with floating ring and energy beam
	FLinearColor MonoMetal(0.09f, 0.09f, 0.11f);
	FLinearColor MonoAccent(0.1f, 0.4f, 0.8f);
	UMaterialInterface* MonoEmissive = FExoMaterialFactory::GetEmissiveOpaque();
	UMaterialInterface* MonoAdditive = FExoMaterialFactory::GetEmissiveAdditive();

	// Stepped base platform (3 tiers)
	SpawnStaticMesh(FVector(0.f, 0.f, GroundZ + 30.f),
		FVector(10.f, 10.f, 0.6f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.08f, 0.08f, 0.09f));
	SpawnStaticMesh(FVector(0.f, 0.f, GroundZ + 60.f),
		FVector(7.f, 7.f, 0.4f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.09f, 0.09f, 0.1f));
	SpawnStaticMesh(FVector(0.f, 0.f, GroundZ + 85.f),
		FVector(5.f, 5.f, 0.3f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.1f, 0.1f, 0.11f));

	// Main obelisk — tapered column
	SpawnStaticMesh(FVector(0.f, 0.f, GroundZ + 600.f),
		FVector(2.f, 2.f, 10.f), FRotator::ZeroRotator, CylinderMesh, MonoMetal);
	// Obelisk cap — narrower, brighter
	SpawnStaticMesh(FVector(0.f, 0.f, GroundZ + 1150.f),
		FVector(1.2f, 1.2f, 2.f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.1f, 0.1f, 0.12f));

	// Floating ring around obelisk mid-height
	SpawnStaticMesh(FVector(0.f, 0.f, GroundZ + 700.f),
		FVector(5.f, 5.f, 0.08f), FRotator::ZeroRotator, CylinderMesh, MonoMetal);
	// Ring emissive inner edge
	UStaticMeshComponent* RingGlow = SpawnStaticMesh(
		FVector(0.f, 0.f, GroundZ + 700.f),
		FVector(3.5f, 3.5f, 0.04f), FRotator::ZeroRotator, CylinderMesh, MonoAccent);
	if (RingGlow && MonoEmissive)
	{
		UMaterialInstanceDynamic* RGMat = UMaterialInstanceDynamic::Create(MonoEmissive, this);
		if (!RGMat) { return; }
		RGMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.2f, 0.8f, 1.5f));
		RingGlow->SetMaterial(0, RGMat);
	}

	// Vertical energy beam through the center
	if (MonoAdditive)
	{
		UStaticMeshComponent* Beam = SpawnStaticMesh(
			FVector(0.f, 0.f, GroundZ + 800.f),
			FVector(0.15f, 0.15f, 16.f), FRotator::ZeroRotator, CylinderMesh, MonoAccent);
		if (Beam)
		{
			UMaterialInstanceDynamic* BMat = UMaterialInstanceDynamic::Create(MonoAdditive, this);
			if (!BMat) { return; }
			BMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.4f, 1.5f, 3.f));
			Beam->SetMaterial(0, BMat);
		}
	}

	// Four accent lights around the base
	for (int32 i = 0; i < 4; i++)
	{
		float Ang = (PI * 0.5f) * i;
		FVector LPos(FMath::Cos(Ang) * 350.f, FMath::Sin(Ang) * 350.f, GroundZ + 100.f);
		UPointLightComponent* BL = NewObject<UPointLightComponent>(this);
		BL->SetupAttachment(RootComponent);
		BL->SetWorldLocation(LPos);
		BL->SetIntensity(3000.f);
		BL->SetAttenuationRadius(800.f);
		BL->SetLightColor(MonoAccent);
		BL->CastShadows = false;
		BL->RegisterComponent();
	}

	// Top beacon light
	UPointLightComponent* MonumentLight = NewObject<UPointLightComponent>(this);
	MonumentLight->SetupAttachment(RootComponent);
	MonumentLight->SetWorldLocation(FVector(0.f, 0.f, GroundZ + 1250.f));
	MonumentLight->SetIntensity(8000.f);
	MonumentLight->SetAttenuationRadius(4000.f);
	MonumentLight->SetLightColor(FLinearColor(0.1f, 0.5f, 1.f));
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

	// Road surface — slightly raised from terrain
	FVector RoadPos = Mid + FVector(0.f, 0.f, 2.f);
	SpawnStaticMesh(RoadPos,
		FVector(Length / 100.f, Width / 100.f, 0.05f), Rot, CubeMesh,
		FLinearColor(0.09f, 0.09f, 0.1f));

	// Curbs — raised edges on both sides
	FVector Right = FRotationMatrix(Rot).GetScaledAxis(EAxis::Y);
	FVector EdgeOffset = Right * Width * 0.5f;
	FLinearColor CurbColor(0.12f, 0.12f, 0.13f);
	SpawnStaticMesh(RoadPos + EdgeOffset + FVector(0.f, 0.f, 8.f),
		FVector(Length / 100.f, 0.4f, 0.15f), Rot, CubeMesh, CurbColor);
	SpawnStaticMesh(RoadPos - EdgeOffset + FVector(0.f, 0.f, 8.f),
		FVector(Length / 100.f, 0.4f, 0.15f), Rot, CubeMesh, CurbColor);

	// Edge lines — continuous strips on both sides (inset from curb)
	FVector LineOffset = Right * Width * 0.46f;
	FLinearColor EdgeColor(0.15f, 0.2f, 0.3f);
	SpawnStaticMesh(RoadPos + LineOffset + FVector(0.f, 0.f, 2.f),
		FVector(Length / 100.f, 0.15f, 0.03f), Rot, CubeMesh, EdgeColor);
	SpawnStaticMesh(RoadPos - LineOffset + FVector(0.f, 0.f, 2.f),
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

	// Asphalt repair patches — break up uniform road surface
	float PatchInterval = 8000.f;
	for (float P = PatchInterval; P + 1000.f < Length; P += PatchInterval)
	{
		float PHash = FMath::Abs(FMath::Sin(P * 0.0003f + Start.X * 0.00007f + Start.Y * 0.00011f));
		if (PHash < 0.4f) continue; // Skip some patches for irregular spacing
		FVector PatchPos = Start + DirNorm * P + FVector(0.f, 0.f, 3.f);
		FVector PatchRight = FRotationMatrix(Rot).GetScaledAxis(EAxis::Y);
		PatchPos += PatchRight * (PHash - 0.5f) * Width * 0.3f; // Offset from center
		float PW = 400.f + PHash * 600.f;
		float PL = 300.f + PHash * 500.f;
		SpawnStaticMesh(PatchPos,
			FVector(PL / 100.f, PW / 100.f, 0.02f), Rot, CubeMesh,
			FLinearColor(0.08f + PHash * 0.03f, 0.08f + PHash * 0.025f, 0.085f + PHash * 0.03f));
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
		FLinearColor(0.11f, 0.11f, 0.13f));

	// Guard rails
	FVector Right = FRotationMatrix(Rot).GetScaledAxis(EAxis::Y) * Width * 0.5f;
	SpawnStaticMesh(DeckPos + Right + FVector(0.f, 0.f, 60.f),
		FVector(Length / 100.f, 0.1f, 0.6f), Rot, CubeMesh,
		FLinearColor(0.12f, 0.12f, 0.14f));
	SpawnStaticMesh(DeckPos - Right + FVector(0.f, 0.f, 60.f),
		FVector(Length / 100.f, 0.1f, 0.6f), Rot, CubeMesh,
		FLinearColor(0.12f, 0.12f, 0.14f));

	// Support columns with accent lights
	int32 NumSupports = FMath::Max(2, FMath::CeilToInt(Length / 5000.f));
	FVector DirNorm = Dir.GetSafeNormal();
	for (int32 i = 0; i < NumSupports; i++)
	{
		float T = (i + 0.5f) / NumSupports;
		FVector ColPos = FMath::Lerp(Start, End, T);
		SpawnStaticMesh(FVector(ColPos.X, ColPos.Y, Height * 0.5f),
			FVector(1.f, 1.f, Height / 100.f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.1f, 0.1f, 0.12f));

		// Underside accent light at each support column
		UPointLightComponent* UnderGlow = NewObject<UPointLightComponent>(this);
		UnderGlow->SetupAttachment(RootComponent);
		UnderGlow->SetWorldLocation(FVector(ColPos.X, ColPos.Y, Height * 0.7f));
		UnderGlow->SetIntensity(4000.f);
		UnderGlow->SetAttenuationRadius(Width * 0.8f);
		UnderGlow->SetLightColor(FLinearColor(0.15f, 0.3f, 0.6f));
		UnderGlow->CastShadows = false;
		UnderGlow->RegisterComponent();
	}

	// Rail-mounted emissive strips on both sides
	UMaterialInterface* RailMat = FExoMaterialFactory::GetEmissiveOpaque();
	if (RailMat)
	{
		for (int32 Side = -1; Side <= 1; Side += 2)
		{
			UStaticMeshComponent* Strip = SpawnStaticMesh(
				DeckPos + Right * (float)Side + FVector(0.f, 0.f, 65.f),
				FVector(Length / 100.f, 0.04f, 0.04f), Rot, CubeMesh,
				FLinearColor(0.1f, 0.3f, 0.6f));
			if (Strip)
			{
				UMaterialInstanceDynamic* SM = UMaterialInstanceDynamic::Create(RailMat, this);
				if (!SM) { return; }
				SM->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(0.15f, 0.4f, 0.8f));
				Strip->SetMaterial(0, SM);
			}
		}
	}
}

// BuildWaterFeatures and BuildFoliage moved to ExoLevelBuilderNature.cpp
