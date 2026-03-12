// ExoLevelBuilderFieldCover.cpp — Rock formations, ruins, and natural cover in open fields
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildFieldCover()
{
	FMath::SRandInit(777);

	// === ROCK FORMATIONS — large clusters providing cover between compounds ===
	struct FRockCluster { FVector Pos; float Scale; float Yaw; };
	TArray<FRockCluster> RockClusters = {
		// Between center and north compound
		{{-5000.f, 30000.f, 0.f}, 1.2f, 15.f},
		{{8000.f, 50000.f, 0.f}, 0.9f, 45.f},
		{{-12000.f, 60000.f, 0.f}, 1.4f, 110.f},
		// Between center and south compound
		{{6000.f, -35000.f, 0.f}, 1.1f, 200.f},
		{{-10000.f, -55000.f, 0.f}, 1.3f, 75.f},
		{{15000.f, -65000.f, 0.f}, 0.8f, 320.f},
		// Between center and east compound
		{{35000.f, 8000.f, 0.f}, 1.0f, 155.f},
		{{55000.f, -5000.f, 0.f}, 1.5f, 270.f},
		{{45000.f, 12000.f, 0.f}, 0.7f, 90.f},
		// Between center and west compound
		{{-30000.f, -8000.f, 0.f}, 1.1f, 340.f},
		{{-50000.f, 10000.f, 0.f}, 1.3f, 60.f},
		{{-65000.f, -15000.f, 0.f}, 0.9f, 180.f},
		// Diagonal corridors
		{{30000.f, 30000.f, 0.f}, 1.0f, 225.f},
		{{-35000.f, 35000.f, 0.f}, 1.2f, 135.f},
		{{40000.f, -40000.f, 0.f}, 1.1f, 315.f},
		{{-45000.f, -40000.f, 0.f}, 0.8f, 50.f},
		// Outer ring
		{{90000.f, 50000.f, 0.f}, 1.4f, 170.f},
		{{-80000.f, 60000.f, 0.f}, 1.0f, 290.f},
		{{70000.f, -70000.f, 0.f}, 1.3f, 100.f},
		{{-90000.f, -50000.f, 0.f}, 1.1f, 240.f},
	};
	for (const auto& RC : RockClusters)
	{
		SpawnRockFormation(RC.Pos, RC.Scale, RC.Yaw);
	}

	// === RUINED STRUCTURES — collapsed buildings in the wasteland ===
	struct FRuin { FVector Pos; float Scale; float Yaw; };
	TArray<FRuin> Ruins = {
		{{20000.f, 40000.f, 0.f}, 1.0f, 30.f},
		{{-25000.f, -45000.f, 0.f}, 0.8f, 145.f},
		{{50000.f, 50000.f, 0.f}, 1.2f, 260.f},
		{{-60000.f, -60000.f, 0.f}, 0.9f, 80.f},
		{{60000.f, -20000.f, 0.f}, 1.1f, 195.f},
		{{-40000.f, 25000.f, 0.f}, 0.7f, 310.f},
		// Near compound approaches
		{{10000.f, 65000.f, 0.f}, 1.0f, 0.f},
		{{65000.f, -10000.f, 0.f}, 0.85f, 90.f},
		{{-15000.f, -65000.f, 0.f}, 1.1f, 180.f},
		{{-65000.f, 8000.f, 0.f}, 0.95f, 270.f},
	};
	for (const auto& R : Ruins)
	{
		SpawnRuinedStructure(R.Pos, R.Scale, R.Yaw);
	}

	// === SCATTERED BOULDERS — individual rocks for micro-cover ===
	FLinearColor RockDark(0.035f, 0.04f, 0.05f);
	FLinearColor RockMid(0.05f, 0.055f, 0.065f);
	for (int32 i = 0; i < 60; i++)
	{
		float Angle = FMath::FRand() * 2.f * PI;
		float Dist = FMath::FRandRange(15000.f, MapHalfSize * 0.9f);
		FVector Pos(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
		float BoulderScale = FMath::FRandRange(200.f, 800.f);
		float BoulderH = BoulderScale * FMath::FRandRange(0.5f, 1.2f);
		FRotator Rot(FMath::FRandRange(-8.f, 8.f), FMath::FRand() * 360.f, 0.f);

		SpawnStaticMesh(Pos + FVector(0.f, 0.f, BoulderH * 0.4f),
			FVector(BoulderScale / 100.f, BoulderScale * 0.8f / 100.f, BoulderH / 100.f),
			Rot, (i % 3 == 0) ? SphereMesh : CubeMesh,
			(i % 2 == 0) ? RockDark : RockMid);
	}

	// === METAL DEBRIS PILES — small junk heaps with faint glow ===
	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
	for (int32 i = 0; i < 25; i++)
	{
		float Angle = FMath::FRand() * 2.f * PI;
		float Dist = FMath::FRandRange(20000.f, MapHalfSize * 0.8f);
		FVector Pos(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);

		// Base pile — dark metal chunks
		float PileW = FMath::FRandRange(300.f, 800.f);
		float PileH = FMath::FRandRange(100.f, 300.f);
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, PileH * 0.5f),
			FVector(PileW / 100.f, PileW * 0.7f / 100.f, PileH / 100.f),
			FRotator(FMath::FRandRange(-5.f, 5.f), FMath::FRand() * 360.f, 0.f),
			CubeMesh, FLinearColor(0.06f, 0.06f, 0.07f));

		// 2-3 scattered chunks
		for (int32 j = 0; j < 2 + (i % 2); j++)
		{
			float Off = FMath::FRandRange(200.f, 600.f);
			float ChunkS = FMath::FRandRange(80.f, 250.f);
			SpawnStaticMesh(
				Pos + FVector(FMath::FRandRange(-Off, Off),
					FMath::FRandRange(-Off, Off), ChunkS * 0.4f),
				FVector(ChunkS / 100.f, ChunkS * 0.6f / 100.f, ChunkS * 0.5f / 100.f),
				FRotator(FMath::FRandRange(-15.f, 15.f), FMath::FRand() * 360.f,
					FMath::FRandRange(-10.f, 10.f)),
				CubeMesh, FLinearColor(0.07f, 0.07f, 0.08f));
		}

		// Faint glow on every 4th pile — exposed wiring/energy leak
		if (i % 4 == 0 && EmissiveMat)
		{
			FLinearColor Glow = (i % 8 == 0)
				? FLinearColor(0.1f, 0.4f, 0.8f)
				: FLinearColor(0.8f, 0.3f, 0.05f);
			UStaticMeshComponent* GlowPart = SpawnStaticMesh(
				Pos + FVector(0.f, 0.f, PileH + 10.f),
				FVector(PileW * 0.3f / 100.f, 0.04f, 0.03f),
				FRotator(0.f, FMath::FRand() * 360.f, 0.f), CubeMesh, Glow);
			if (GlowPart)
			{
				UMaterialInstanceDynamic* GM = UMaterialInstanceDynamic::Create(
					EmissiveMat, this);
				if (!GM) { return; }
				GM->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(Glow.R * 5.f, Glow.G * 5.f, Glow.B * 5.f));
				GlowPart->SetMaterial(0, GM);
			}

			UPointLightComponent* PL = NewObject<UPointLightComponent>(this);
			PL->SetupAttachment(RootComponent);
			PL->SetWorldLocation(Pos + FVector(0.f, 0.f, PileH + 50.f));
			PL->SetIntensity(2000.f);
			PL->SetAttenuationRadius(600.f);
			PL->SetLightColor(Glow);
			PL->CastShadows = false;
			PL->RegisterComponent();
		}
	}

	// === ROADSIDE PYLONS — glowing markers along main routes ===
	FLinearColor RouteBlue(0.1f, 0.4f, 0.9f);
	struct FRoutePylon { FVector Pos; FLinearColor Color; };
	TArray<FRoutePylon> RoutePylons = {
		// Center → North road
		{{0.f, 20000.f, 0.f}, RouteBlue}, {{0.f, 40000.f, 0.f}, RouteBlue},
		{{0.f, 60000.f, 0.f}, RouteBlue},
		// Center → South road
		{{0.f, -20000.f, 0.f}, RouteBlue}, {{0.f, -40000.f, 0.f}, RouteBlue},
		{{0.f, -60000.f, 0.f}, RouteBlue},
		// Center → East road
		{{20000.f, 0.f, 0.f}, RouteBlue}, {{40000.f, 0.f, 0.f}, RouteBlue},
		{{60000.f, 0.f, 0.f}, RouteBlue},
		// Center → West road
		{{-20000.f, 0.f, 0.f}, RouteBlue}, {{-40000.f, 0.f, 0.f}, RouteBlue},
		{{-60000.f, 0.f, 0.f}, RouteBlue},
	};
	for (const auto& RP : RoutePylons)
	{
		SpawnEnergyPylon(RP.Pos, 500.f, RP.Color);
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Field cover placed — %d rock clusters, "
		"%d ruins, 60 boulders, 25 debris piles, %d route pylons"),
		RockClusters.Num(), Ruins.Num(), RoutePylons.Num());
}

void AExoLevelBuilder::SpawnRockFormation(const FVector& Center, float Scale, float Yaw)
{
	// Use KayKit rock/terrain meshes when available
	if (bHasKayKitAssets && KK_Rock)
	{
		FVector KKScale(Scale * 2.5f);
		SpawnRawMesh(Center, KKScale, FRotator(0.f, Yaw, 0.f), KK_Rock);
		// Flanking rocks
		FRotator BaseRot(0.f, Yaw, 0.f);
		SpawnRawMesh(Center + BaseRot.RotateVector(FVector(400.f * Scale, 300.f * Scale, 0.f)),
			KKScale * 0.6f, FRotator(0.f, Yaw + 40.f, 0.f), KK_Rock);
		SpawnRawMesh(Center + BaseRot.RotateVector(FVector(-350.f * Scale, -250.f * Scale, 0.f)),
			KKScale * 0.4f, FRotator(0.f, Yaw - 60.f, 0.f), KK_Rock);
		// Add a terrain piece at larger formations
		if (KK_TerrainLow && Scale > 1.0f)
		{
			SpawnRawMesh(Center + BaseRot.RotateVector(FVector(0.f, 500.f * Scale, 0.f)),
				FVector(Scale * 2.f), FRotator(0.f, Yaw + 90.f, 0.f), KK_TerrainLow);
		}
		return;
	}

	FRotator BaseRot(0.f, Yaw, 0.f);
	FLinearColor RockColor(0.04f, 0.045f, 0.055f);
	FLinearColor RockLight(0.055f, 0.06f, 0.07f);

	// Central monolith
	float MonolithH = 600.f * Scale;
	float MonolithW = 400.f * Scale;
	SpawnStaticMesh(Center + FVector(0.f, 0.f, MonolithH * 0.5f),
		FVector(MonolithW / 100.f, MonolithW * 0.7f / 100.f, MonolithH / 100.f),
		FRotator(FMath::RandRange(-5.f, 5.f), Yaw, FMath::RandRange(-3.f, 3.f)),
		CubeMesh, RockColor);

	// Flanking slabs
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		float SlabH = MonolithH * FMath::RandRange(0.4f, 0.7f);
		float SlabW = MonolithW * FMath::RandRange(0.6f, 1.0f);
		FVector Offset = BaseRot.RotateVector(
			FVector(Side * MonolithW * 0.8f, Side * MonolithW * 0.3f, 0.f));
		SpawnStaticMesh(Center + Offset + FVector(0.f, 0.f, SlabH * 0.5f),
			FVector(SlabW / 100.f, SlabW * 0.5f / 100.f, SlabH / 100.f),
			FRotator(FMath::RandRange(-8.f, 8.f), Yaw + Side * 20.f, 0.f),
			CubeMesh, RockLight);
	}

	// Scattered smaller rocks at the base
	for (int32 i = 0; i < 4; i++)
	{
		float Off = MonolithW * FMath::RandRange(0.8f, 1.8f);
		float SmallH = MonolithH * FMath::RandRange(0.1f, 0.3f);
		float SmallW = MonolithW * FMath::RandRange(0.3f, 0.6f);
		float A = FMath::RandRange(0.f, 360.f);
		FVector Pos = Center + FVector(FMath::Cos(FMath::DegreesToRadians(A)) * Off,
			FMath::Sin(FMath::DegreesToRadians(A)) * Off, SmallH * 0.5f);
		SpawnStaticMesh(Pos,
			FVector(SmallW / 100.f, SmallW * 0.7f / 100.f, SmallH / 100.f),
			FRotator(FMath::RandRange(-12.f, 12.f), FMath::RandRange(0.f, 360.f), 0.f),
			(i % 2 == 0) ? SphereMesh : CubeMesh, RockColor);
	}
}

void AExoLevelBuilder::SpawnRuinedStructure(const FVector& Center, float Scale, float Yaw)
{
	FRotator Rot(0.f, Yaw, 0.f);
	FLinearColor WallColor(0.06f, 0.065f, 0.075f);
	FLinearColor FloorColor(0.05f, 0.05f, 0.06f);
	float S = Scale;

	// Broken foundation slab
	float FloorW = 2000.f * S;
	float FloorD = 1500.f * S;
	SpawnStaticMesh(Center + FVector(0.f, 0.f, 15.f),
		FVector(FloorW / 100.f, FloorD / 100.f, 0.3f), Rot, CubeMesh, FloorColor);

	// Partial back wall — broken top edge
	float WallH = 800.f * S;
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -FloorD * 0.45f, WallH * 0.5f)),
		FVector(FloorW * 0.9f / 100.f, 0.8f, WallH / 100.f), Rot, CubeMesh, WallColor);
	// Broken upper section — shorter, offset
	SpawnStaticMesh(
		Center + Rot.RotateVector(FVector(-FloorW * 0.2f, -FloorD * 0.45f, WallH * 0.9f)),
		FVector(FloorW * 0.4f / 100.f, 0.8f, WallH * 0.3f / 100.f),
		FRotator(3.f, Yaw, -2.f), CubeMesh, WallColor);

	// Left wall stub — only lower portion remains
	float StubH = WallH * 0.5f;
	SpawnStaticMesh(
		Center + Rot.RotateVector(FVector(-FloorW * 0.45f, 0.f, StubH * 0.5f)),
		FVector(0.8f, FloorD * 0.6f / 100.f, StubH / 100.f), Rot, CubeMesh, WallColor);

	// Collapsed pillar on the ground
	float PillarLen = 600.f * S;
	SpawnStaticMesh(
		Center + Rot.RotateVector(FVector(FloorW * 0.2f, FloorD * 0.2f, 60.f * S)),
		FVector(1.f * S, 1.f * S, PillarLen / 100.f),
		FRotator(85.f, Yaw + 30.f, 0.f), CylinderMesh,
		FLinearColor(0.07f, 0.075f, 0.085f));

	// Rubble chunks around the ruin
	for (int32 i = 0; i < 5; i++)
	{
		float Off = FMath::RandRange(400.f, 1200.f) * S;
		float ChunkS = FMath::RandRange(60.f, 200.f) * S;
		float A = FMath::RandRange(0.f, 360.f);
		SpawnStaticMesh(
			Center + FVector(FMath::Cos(FMath::DegreesToRadians(A)) * Off,
				FMath::Sin(FMath::DegreesToRadians(A)) * Off, ChunkS * 0.4f),
			FVector(ChunkS / 100.f, ChunkS * 0.7f / 100.f, ChunkS * 0.5f / 100.f),
			FRotator(FMath::RandRange(-20.f, 20.f), FMath::RandRange(0.f, 360.f),
				FMath::RandRange(-15.f, 15.f)),
			CubeMesh, WallColor);
	}

	// Faint accent glow from exposed wiring in the ruin
	UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveOpaque();
	FLinearColor RuinGlow(0.1f, 0.3f, 0.6f);
	if (EmMat)
	{
		UStaticMeshComponent* Wire = SpawnStaticMesh(
			Center + Rot.RotateVector(FVector(0.f, -FloorD * 0.4f, WallH * 0.3f)),
			FVector(FloorW * 0.3f / 100.f, 0.03f, 0.04f), Rot, CubeMesh, RuinGlow);
		if (Wire)
		{
			UMaterialInstanceDynamic* WM = UMaterialInstanceDynamic::Create(EmMat, this);
			if (!WM) { return; }
			WM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(RuinGlow.R * 4.f, RuinGlow.G * 4.f, RuinGlow.B * 4.f));
			Wire->SetMaterial(0, WM);
		}
	}

	UPointLightComponent* RuinLight = NewObject<UPointLightComponent>(this);
	RuinLight->SetupAttachment(RootComponent);
	RuinLight->SetWorldLocation(Center + FVector(0.f, 0.f, WallH * 0.4f));
	RuinLight->SetIntensity(3000.f);
	RuinLight->SetAttenuationRadius(1200.f * S);
	RuinLight->SetLightColor(RuinGlow);
	RuinLight->CastShadows = false;
	RuinLight->RegisterComponent();
}
