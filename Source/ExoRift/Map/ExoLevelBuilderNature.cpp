// ExoLevelBuilderNature.cpp — Water features, crystal formations, alien flora
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildWaterFeatures()
{
	FLinearColor WaterColor(0.02f, 0.06f, 0.12f);
	UMaterialInterface* WaterMat = FExoMaterialFactory::GetLitEmissive();

	// River channel running NE to SW through terrain patches
	float RiverWidth = 4000.f;
	int32 RiverPoints = 8;
	FVector RiverPath[] = {
		{-30000.f, 30000.f, GroundZ - 100.f},
		{-20000.f, 20000.f, GroundZ - 100.f},
		{-12000.f, 14000.f, GroundZ - 100.f},
		{-6000.f, 8000.f, GroundZ - 100.f},
		{4000.f, -4000.f, GroundZ - 100.f},
		{12000.f, -10000.f, GroundZ - 100.f},
		{20000.f, -16000.f, GroundZ - 100.f},
		{30000.f, -24000.f, GroundZ - 100.f},
	};

	for (int32 i = 0; i < RiverPoints - 1; i++)
	{
		FVector S = RiverPath[i];
		FVector E = RiverPath[i + 1];
		FVector Mid = (S + E) * 0.5f;
		FVector Dir = E - S;
		float Len = Dir.Size();
		FRotator Rot = Dir.Rotation();

		// PBR water surface — metallic reflective with sci-fi emissive glow
		UStaticMeshComponent* Water = SpawnStaticMesh(Mid,
			FVector(Len / 100.f, RiverWidth / 100.f, 0.05f),
			Rot, CubeMesh, WaterColor);
		if (Water && WaterMat)
		{
			UMaterialInstanceDynamic* WMat = UMaterialInstanceDynamic::Create(WaterMat, this);
			if (!WMat) { return; }
			WMat->SetVectorParameterValue(TEXT("BaseColor"), WaterColor);
			WMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.06f, 0.18f, 0.35f));
			WMat->SetScalarParameterValue(TEXT("Metallic"), 0.95f);
			WMat->SetScalarParameterValue(TEXT("Roughness"), 0.05f);
			Water->SetMaterial(0, WMat);
		}

		// Ambient glow at each river bend
		if (i > 0 && i < RiverPoints - 2)
		{
			UPointLightComponent* WGlow = NewObject<UPointLightComponent>(this);
			WGlow->SetupAttachment(RootComponent);
			WGlow->SetWorldLocation(Mid + FVector(0.f, 0.f, 100.f));
			WGlow->SetIntensity(2000.f);
			WGlow->SetAttenuationRadius(RiverWidth * 0.8f);
			WGlow->SetLightColor(FLinearColor(0.05f, 0.2f, 0.4f));
			WGlow->CastShadows = false;
			WGlow->RegisterComponent();
		}
	}

	// River bank edge strips — faint cyan glow at waterline
	UMaterialInterface* EdgeMat = FExoMaterialFactory::GetEmissiveAdditive();
	for (int32 i = 0; i < RiverPoints - 1; i++)
	{
		FVector S = RiverPath[i];
		FVector E = RiverPath[i + 1];
		FVector Mid = (S + E) * 0.5f;
		FVector Dir = E - S;
		float Len = Dir.Size();
		FRotator Rot = Dir.Rotation();
		FVector Right = FRotationMatrix(Rot).GetScaledAxis(EAxis::Y);
		for (int32 Side = -1; Side <= 1; Side += 2)
		{
			FVector EdgePos = Mid + Right * (RiverWidth * 0.48f * Side) + FVector(0.f, 0.f, 5.f);
			UStaticMeshComponent* Edge = SpawnStaticMesh(EdgePos,
				FVector(Len / 100.f, 0.5f, 0.04f), Rot, CubeMesh,
				FLinearColor(0.03f, 0.1f, 0.2f));
			if (Edge && EdgeMat)
			{
				UMaterialInstanceDynamic* EMat = UMaterialInstanceDynamic::Create(EdgeMat, this);
				if (!EMat) { return; }
				EMat->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(0.1f, 0.4f, 0.8f));
				Edge->SetMaterial(0, EMat);
			}
		}
	}

	// Industrial cooling pond near East compound
	UStaticMeshComponent* Pond = SpawnStaticMesh(
		FVector(17000.f, 2000.f, GroundZ - 50.f),
		FVector(60.f, 40.f, 0.1f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.03f, 0.06f, 0.08f));
	if (Pond && WaterMat)
	{
		UMaterialInstanceDynamic* PMat = UMaterialInstanceDynamic::Create(WaterMat, this);
		if (!PMat) { return; }
		PMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.03f, 0.06f, 0.08f));
		PMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.08f, 0.2f, 0.4f));
		PMat->SetScalarParameterValue(TEXT("Metallic"), 0.95f);
		PMat->SetScalarParameterValue(TEXT("Roughness"), 0.05f);
		Pond->SetMaterial(0, PMat);
	}

	// Pond ambient glow
	UPointLightComponent* PondGlow = NewObject<UPointLightComponent>(this);
	PondGlow->SetupAttachment(RootComponent);
	PondGlow->SetWorldLocation(FVector(17000.f, 2000.f, GroundZ));
	PondGlow->SetIntensity(2000.f);
	PondGlow->SetAttenuationRadius(3000.f);
	PondGlow->SetLightColor(FLinearColor(0.05f, 0.15f, 0.3f));
	PondGlow->CastShadows = false;
	PondGlow->RegisterComponent();

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Water features placed"));
}

void AExoLevelBuilder::BuildFoliage()
{
	UMaterialInterface* CrysEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
	FLinearColor StumpColor(0.06f, 0.05f, 0.04f);

	// Multi-colored crystal clusters across the map
	struct FCrystalCluster {
		FVector Pos; int32 Count; float Spread;
		FLinearColor EmissiveColor; FLinearColor LightColor;
	};
	TArray<FCrystalCluster> Clusters = {
		// Blue — river banks, water edges
		{{-10000.f, 10000.f, GroundZ}, 6, 3000.f,
			{0.1f, 0.3f, 0.6f}, {0.1f, 0.3f, 0.8f}},
		{{-18000.f, 18000.f, GroundZ}, 5, 4000.f,
			{0.08f, 0.25f, 0.55f}, {0.1f, 0.3f, 0.7f}},
		{{6000.f, -6000.f, GroundZ}, 3, 2000.f,
			{0.12f, 0.35f, 0.7f}, {0.1f, 0.3f, 0.8f}},
		// Red/orange — fire hazard zone, near bombardment craters
		{{8000.f, 20000.f, GroundZ}, 5, 3500.f,
			{0.6f, 0.12f, 0.03f}, {0.8f, 0.15f, 0.05f}},
		{{15000.f, 6000.f, GroundZ}, 4, 2500.f,
			{0.5f, 0.1f, 0.02f}, {0.7f, 0.12f, 0.04f}},
		// Green — toxic/radiation zones
		{{-22000.f, -14000.f, GroundZ}, 5, 3000.f,
			{0.08f, 0.4f, 0.06f}, {0.1f, 0.5f, 0.08f}},
		{{-12000.f, -9000.f, GroundZ}, 4, 2800.f,
			{0.06f, 0.35f, 0.05f}, {0.08f, 0.45f, 0.06f}},
		// Amber — industrial north compound outskirts
		{{2000.f, 12000.f, GroundZ}, 4, 3000.f,
			{0.5f, 0.3f, 0.05f}, {0.7f, 0.4f, 0.08f}},
		{{-4000.f, 14000.f, GroundZ}, 3, 2000.f,
			{0.45f, 0.25f, 0.04f}, {0.6f, 0.35f, 0.06f}},
		// Purple — exotic ordnance impact sites
		{{12000.f, -12000.f, GroundZ}, 4, 2500.f,
			{0.25f, 0.06f, 0.4f}, {0.35f, 0.08f, 0.55f}},
		{{-19000.f, -6000.f, GroundZ}, 3, 2000.f,
			{0.2f, 0.05f, 0.35f}, {0.3f, 0.07f, 0.5f}},
		// White/silver — recent kinetic impact sites
		{{19000.f, 2000.f, GroundZ}, 3, 1800.f,
			{0.4f, 0.38f, 0.35f}, {0.5f, 0.48f, 0.45f}},
	};

	for (const auto& Cluster : Clusters)
	{
		for (int32 i = 0; i < Cluster.Count; i++)
		{
			FVector Offset(
				FMath::RandRange(-Cluster.Spread, Cluster.Spread),
				FMath::RandRange(-Cluster.Spread, Cluster.Spread), 0.f);
			FVector Pos = Cluster.Pos + Offset;
			float CHeight = FMath::RandRange(200.f, 900.f);
			float CWidth = FMath::RandRange(40.f, 130.f);

			UStaticMeshComponent* Crystal = SpawnStaticMesh(
				Pos + FVector(0.f, 0.f, CHeight * 0.4f),
				FVector(CWidth / 100.f, CWidth * 0.6f / 100.f, CHeight / 100.f),
				FRotator(FMath::RandRange(-20.f, 20.f),
					FMath::RandRange(0.f, 360.f),
					FMath::RandRange(-15.f, 15.f)),
				CubeMesh, Cluster.EmissiveColor);
			if (Crystal && CrysEmissiveMat)
			{
				UMaterialInstanceDynamic* CrysMat =
					UMaterialInstanceDynamic::Create(CrysEmissiveMat, this);
				CrysMat->SetVectorParameterValue(TEXT("EmissiveColor"),
					Cluster.EmissiveColor * 1.5f);
				Crystal->SetMaterial(0, CrysMat);
			}

			// Occasional secondary shard sprouting off the main crystal
			if (i % 2 == 0)
			{
				float SH = CHeight * FMath::RandRange(0.3f, 0.6f);
				float SW = CWidth * 0.5f;
				UStaticMeshComponent* Sub = SpawnStaticMesh(
					Pos + FVector(CWidth * 0.4f, 0.f, SH * 0.3f),
					FVector(SW / 100.f, SW * 0.5f / 100.f, SH / 100.f),
					FRotator(FMath::RandRange(15.f, 35.f),
						FMath::RandRange(0.f, 360.f), 0.f),
					CubeMesh, Cluster.EmissiveColor);
				if (Sub && CrysEmissiveMat)
				{
					UMaterialInstanceDynamic* SM =
						UMaterialInstanceDynamic::Create(CrysEmissiveMat, this);
					FLinearColor Dim(Cluster.EmissiveColor.R * 2.5f,
						Cluster.EmissiveColor.G * 2.5f,
						Cluster.EmissiveColor.B * 2.5f);
					SM->SetVectorParameterValue(TEXT("EmissiveColor"), Dim);
					Sub->SetMaterial(0, SM);
				}
			}
		}

		// Ambient glow at cluster center
		UPointLightComponent* Glow = NewObject<UPointLightComponent>(this);
		Glow->SetupAttachment(RootComponent);
		Glow->SetWorldLocation(Cluster.Pos + FVector(0.f, 0.f, 200.f));
		Glow->SetIntensity(2000.f);
		Glow->SetAttenuationRadius(Cluster.Spread * 0.8f);
		Glow->SetLightColor(Cluster.LightColor);
		Glow->CastShadows = false;
		Glow->RegisterComponent();
	}

	// Rocky outcrops — natural cover in open fields between compounds
	struct FOutcrop { FVector Pos; float Scale; float Yaw; };
	TArray<FOutcrop> Outcrops = {
		{{4000.f, 4000.f, GroundZ}, 1.2f, 30.f},
		{{-5000.f, -4000.f, GroundZ}, 1.0f, 120.f},
		{{9000.f, -3000.f, GroundZ}, 1.5f, -45.f},
		{{-3000.f, 9000.f, GroundZ}, 0.8f, 60.f},
		{{12000.f, 6000.f, GroundZ}, 1.3f, 15.f},
		{{-6000.f, 12000.f, GroundZ}, 1.1f, -30.f},
		{{14000.f, -8000.f, GroundZ}, 0.9f, 75.f},
		{{-9000.f, -14000.f, GroundZ}, 1.4f, 150.f},
		{{-16000.f, 8000.f, GroundZ}, 1.0f, -80.f},
		{{7000.f, 14000.f, GroundZ}, 1.2f, 45.f},
	};
	FLinearColor RockDark(0.035f, 0.04f, 0.045f);
	FLinearColor RockMed(0.05f, 0.055f, 0.06f);
	for (const auto& O : Outcrops)
	{
		float S = O.Scale;
		// Main boulder
		SpawnStaticMesh(O.Pos + FVector(0.f, 0.f, 200.f * S),
			FVector(4.f * S, 3.f * S, 4.f * S),
			FRotator(FMath::RandRange(-8.f, 8.f), O.Yaw, 0.f),
			SphereMesh, RockDark);
		// Side slab
		SpawnStaticMesh(O.Pos + FVector(300.f * S, 150.f * S, 100.f * S),
			FVector(3.f * S, 2.f * S, 2.f * S),
			FRotator(10.f, O.Yaw + 35.f, 5.f),
			CubeMesh, RockMed);
		// Small scatter rocks
		for (int32 j = 0; j < 4; j++)
		{
			float Ang = j * 90.f + FMath::RandRange(-30.f, 30.f);
			float Dist = FMath::RandRange(400.f, 800.f) * S;
			FVector Scatter = O.Pos + FVector(
				FMath::Cos(FMath::DegreesToRadians(Ang)) * Dist,
				FMath::Sin(FMath::DegreesToRadians(Ang)) * Dist,
				FMath::RandRange(20.f, 60.f) * S);
			float RS = FMath::RandRange(0.4f, 1.0f) * S;
			SpawnStaticMesh(Scatter, FVector(RS, RS * 0.7f, RS * 0.5f),
				FRotator(FMath::RandRange(-10.f, 10.f),
					FMath::RandRange(0.f, 360.f), 0.f),
				SphereMesh, (j % 2 == 0) ? RockDark : RockMed);
		}
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
			FRotator(FMath::RandRange(-5.f, 5.f),
				FMath::RandRange(0.f, 360.f), 0.f),
			CylinderMesh, StumpColor);
	}

	// Alien fungal growths — short bulbous shapes with faint bioluminescence
	UMaterialInterface* FungalMat = FExoMaterialFactory::GetLitEmissive();
	struct FFungal { FVector Pos; FLinearColor Color; };
	TArray<FFungal> Fungi = {
		{{-7000.f, 3000.f, GroundZ}, {0.06f, 0.12f, 0.04f}},
		{{5000.f, -9000.f, GroundZ}, {0.04f, 0.1f, 0.08f}},
		{{-14000.f, -4000.f, GroundZ}, {0.08f, 0.06f, 0.12f}},
		{{11000.f, 11000.f, GroundZ}, {0.05f, 0.1f, 0.05f}},
		{{-3000.f, -11000.f, GroundZ}, {0.1f, 0.06f, 0.04f}},
		{{16000.f, -12000.f, GroundZ}, {0.04f, 0.08f, 0.1f}},
	};
	for (const auto& F : Fungi)
	{
		for (int32 i = 0; i < 5; i++)
		{
			FVector Offset(FMath::RandRange(-1500.f, 1500.f),
				FMath::RandRange(-1500.f, 1500.f), 0.f);
			float H = FMath::RandRange(60.f, 200.f);
			float R = FMath::RandRange(30.f, 80.f);
			UStaticMeshComponent* Fungus = SpawnStaticMesh(
				F.Pos + Offset + FVector(0.f, 0.f, H * 0.3f),
				FVector(R / 50.f, R / 50.f, H / 100.f),
				FRotator(FMath::RandRange(-10.f, 10.f),
					FMath::RandRange(0.f, 360.f), 0.f),
				SphereMesh, F.Color);
			if (Fungus && FungalMat)
			{
				UMaterialInstanceDynamic* FM =
					UMaterialInstanceDynamic::Create(FungalMat, this);
				FM->SetVectorParameterValue(TEXT("BaseColor"), F.Color);
				FM->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(F.Color.R * 2.f, F.Color.G * 2.f,
						F.Color.B * 2.f));
				FM->SetScalarParameterValue(TEXT("Metallic"), 0.1f);
				FM->SetScalarParameterValue(TEXT("Roughness"), 0.7f);
				Fungus->SetMaterial(0, FM);
			}
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Foliage, crystals, and rock formations placed"));
}
