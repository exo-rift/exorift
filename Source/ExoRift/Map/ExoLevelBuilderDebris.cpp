// ExoLevelBuilderDebris.cpp — Crashed ships, debris fields, scorch marks
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"

void AExoLevelBuilder::BuildEnvironmentalDebris()
{
	// === CRASHED DROPSHIPS — large wrecked vessels scattered across the map ===
	SpawnCrashedShip(FVector(35000.f, 55000.f, 0.f), 25.f, 1.f);
	SpawnCrashedShip(FVector(-65000.f, 35000.f, 0.f), -40.f, 1.3f);
	SpawnCrashedShip(FVector(90000.f, -70000.f, 0.f), 110.f, 0.8f);
	SpawnCrashedShip(FVector(-45000.f, -90000.f, 0.f), 175.f, 1.1f);
	SpawnCrashedShip(FVector(15000.f, -40000.f, 0.f), -65.f, 0.7f);

	// === DEBRIS FIELDS — scattered wreckage from orbital bombardment ===
	SpawnDebrisField(FVector(25000.f, 60000.f, 0.f), 5000.f, 35);
	SpawnDebrisField(FVector(-70000.f, 30000.f, 0.f), 4000.f, 28);
	SpawnDebrisField(FVector(80000.f, -65000.f, 0.f), 6000.f, 40);
	SpawnDebrisField(FVector(-40000.f, -85000.f, 0.f), 3500.f, 24);
	SpawnDebrisField(FVector(50000.f, -10000.f, 0.f), 3000.f, 20);
	// Additional debris fields in open areas
	SpawnDebrisField(FVector(-15000.f, 40000.f, 0.f), 3500.f, 18);
	SpawnDebrisField(FVector(65000.f, 45000.f, 0.f), 4500.f, 30);
	SpawnDebrisField(FVector(-55000.f, -50000.f, 0.f), 3000.f, 22);

	// === SCORCH MARKS — impact burns from previous battles ===
	SpawnScorchMark(FVector(35000.f, 55000.f, 1.f), 3000.f);
	SpawnScorchMark(FVector(-65000.f, 35000.f, 1.f), 4000.f);
	SpawnScorchMark(FVector(90000.f, -70000.f, 1.f), 2500.f);
	SpawnScorchMark(FVector(-45000.f, -90000.f, 1.f), 3500.f);
	SpawnScorchMark(FVector(10000.f, 30000.f, 1.f), 1800.f, 45.f);
	SpawnScorchMark(FVector(-30000.f, -15000.f, 1.f), 2000.f, -20.f);
	SpawnScorchMark(FVector(60000.f, 20000.f, 1.f), 1500.f);

	// === DOWNED COMM TOWER — toppled structure near south compound ===
	{
		FVector TowerBase(-15000.f, -65000.f, 0.f);
		FRotator TowerRot(75.f, 30.f, 0.f); // Fallen at steep angle
		// Main shaft (horizontal, on ground)
		SpawnStaticMesh(TowerBase + FVector(0.f, 0.f, 200.f),
			FVector(0.4f, 0.4f, 50.f), TowerRot, CylinderMesh,
			FLinearColor(0.1f, 0.1f, 0.12f));
		// Bent antenna section
		SpawnStaticMesh(TowerBase + TowerRot.RotateVector(FVector(0.f, 0.f, 2500.f)),
			FVector(0.2f, 0.2f, 15.f), TowerRot + FRotator(-20.f, 0.f, 0.f),
			CylinderMesh, FLinearColor(0.12f, 0.12f, 0.14f));
		// Cross arms (broken, askew)
		SpawnStaticMesh(TowerBase + TowerRot.RotateVector(FVector(0.f, 0.f, 1800.f)),
			FVector(6.f, 0.15f, 0.15f), TowerRot + FRotator(0.f, 0.f, 12.f),
			CubeMesh, FLinearColor(0.11f, 0.11f, 0.13f));
		// Sparking light (still flickering)
		UPointLightComponent* SparkLight = NewObject<UPointLightComponent>(this);
		SparkLight->SetupAttachment(RootComponent);
		SparkLight->SetWorldLocation(TowerBase + TowerRot.RotateVector(FVector(0.f, 0.f, 2400.f)));
		SparkLight->SetIntensity(3000.f);
		SparkLight->SetAttenuationRadius(1200.f);
		SparkLight->SetLightColor(FLinearColor(0.2f, 0.5f, 1.f));
		SparkLight->CastShadows = false;
		SparkLight->RegisterComponent();
	}

	// === DESTROYED VEHICLE CONVOY — line of wrecked transports ===
	{
		FVector ConvoyStart(55000.f, 40000.f, 0.f);
		FVector ConvoyDir = FVector(1.f, 0.3f, 0.f).GetSafeNormal();
		float ConvoyYaw = ConvoyDir.Rotation().Yaw;
		FLinearColor BurntMetal(0.04f, 0.04f, 0.045f);
		for (int32 i = 0; i < 4; i++)
		{
			FVector VPos = ConvoyStart + ConvoyDir * (i * 2500.f);
			float Tilt = FMath::RandRange(-8.f, 8.f);
			float YawJitter = ConvoyYaw + FMath::RandRange(-15.f, 15.f);
			FRotator VRot(Tilt, YawJitter, FMath::RandRange(-3.f, 3.f));

			// Chassis
			SpawnStaticMesh(VPos + FVector(0.f, 0.f, 120.f),
				FVector(9.f, 3.5f, 2.f), VRot, CubeMesh, BurntMetal);
			// Cab (some missing)
			if (i != 2)
			{
				SpawnStaticMesh(VPos + VRot.RotateVector(FVector(500.f, 0.f, 220.f)),
					FVector(3.f, 3.2f, 2.5f), VRot, CubeMesh,
					FLinearColor(0.05f, 0.05f, 0.055f));
			}
			// Scorch mark under each vehicle
			SpawnScorchMark(VPos, 800.f, YawJitter);
		}
	}

	// === IMPACT CRATERS WITH GLOWING RESIDUE ===
	{
		struct FGlowCrater { FVector Pos; float Radius; FLinearColor GlowCol; };
		TArray<FGlowCrater> GlowCraters = {
			// Fire orange — orbital bombardment sites
			{{45000.f, -55000.f, 0.f}, 2000.f, FLinearColor(0.4f, 0.1f, 0.02f)},
			{{-20000.f, 45000.f, 0.f}, 1600.f, FLinearColor(0.6f, 0.15f, 0.03f)},
			{{75000.f, 30000.f, 0.f}, 2200.f, FLinearColor(0.5f, 0.12f, 0.02f)},
			// Energy blue — weapons platform impacts
			{{-80000.f, 50000.f, 0.f}, 2500.f, FLinearColor(0.05f, 0.2f, 0.4f)},
			{{60000.f, -85000.f, 0.f}, 1900.f, FLinearColor(0.08f, 0.25f, 0.5f)},
			{{-30000.f, -60000.f, 0.f}, 1400.f, FLinearColor(0.04f, 0.15f, 0.35f)},
			// Plasma purple — exotic ordnance
			{{30000.f, 100000.f, 0.f}, 1800.f, FLinearColor(0.2f, 0.05f, 0.3f)},
			{{-95000.f, -30000.f, 0.f}, 2100.f, FLinearColor(0.25f, 0.06f, 0.35f)},
			// Radiation green — reactor breach sites
			{{-60000.f, -45000.f, 0.f}, 2800.f, FLinearColor(0.08f, 0.3f, 0.05f)},
			{{50000.f, 70000.f, 0.f}, 1500.f, FLinearColor(0.1f, 0.35f, 0.06f)},
			// Hot white — recent kinetic impacts
			{{-10000.f, -95000.f, 0.f}, 1200.f, FLinearColor(0.4f, 0.35f, 0.3f)},
			{{95000.f, 10000.f, 0.f}, 1700.f, FLinearColor(0.35f, 0.3f, 0.25f)},
		};
		for (const auto& GC : GlowCraters)
		{
			SpawnCrater(GC.Pos, GC.Radius);
			// Glowing residue pool at crater center
			UStaticMeshComponent* Pool = SpawnStaticMesh(
				GC.Pos + FVector(0.f, 0.f, -30.f),
				FVector(GC.Radius * 0.5f / 50.f, GC.Radius * 0.5f / 50.f, 0.03f),
				FRotator::ZeroRotator, CylinderMesh, GC.GlowCol);
			if (Pool)
			{
				UMaterialInterface* PoolEmissiveMat = FExoMaterialFactory::GetEmissiveAdditive();
				UMaterialInstanceDynamic* GM = UMaterialInstanceDynamic::Create(PoolEmissiveMat, this);
				if (!GM) { return; }
				GM->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(GC.GlowCol.R * 8.f, GC.GlowCol.G * 8.f, GC.GlowCol.B * 8.f));
				Pool->SetMaterial(0, GM);
			}
			// Eerie glow light
			UPointLightComponent* GL = NewObject<UPointLightComponent>(this);
			GL->SetupAttachment(RootComponent);
			GL->SetWorldLocation(GC.Pos + FVector(0.f, 0.f, 50.f));
			GL->SetIntensity(8000.f);
			GL->SetAttenuationRadius(GC.Radius * 2.f);
			GL->SetLightColor(GC.GlowCol);
			GL->CastShadows = false;
			GL->RegisterComponent();
		}
	}
}

void AExoLevelBuilder::SpawnCrashedShip(const FVector& Center, float Yaw, float Scale)
{
	FRotator Rot(FMath::RandRange(-5.f, 5.f), Yaw, FMath::RandRange(-8.f, 8.f));
	FLinearColor HullCol(0.05f, 0.055f, 0.06f);
	FLinearColor DarkCol(0.03f, 0.03f, 0.035f);
	float S = Scale;

	// Main fuselage — long tapered hull
	SpawnStaticMesh(Center + FVector(0.f, 0.f, 300.f * S),
		FVector(20.f * S, 6.f * S, 4.f * S), Rot, CubeMesh, HullCol);

	// Nose section — narrower, angled into ground
	FRotator NoseRot = Rot + FRotator(12.f, 0.f, 0.f);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(1200.f * S, 0.f, 100.f * S)),
		FVector(8.f * S, 4.f * S, 3.f * S), NoseRot, CubeMesh, DarkCol);

	// Engine nacelle (left)
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(-600.f * S, 500.f * S, 250.f * S)),
		FVector(2.f * S, 2.f * S, 12.f * S), Rot + FRotator(0.f, 0.f, 90.f),
		CylinderMesh, FLinearColor(0.06f, 0.06f, 0.07f));

	// Engine nacelle (right — detached, rolled away)
	FVector DetachedPos = Center + Rot.RotateVector(
		FVector(-800.f * S, -700.f * S, 80.f * S));
	SpawnStaticMesh(DetachedPos,
		FVector(2.f * S, 2.f * S, 10.f * S),
		FRotator(FMath::RandRange(20.f, 40.f), Yaw + 35.f, 15.f),
		CylinderMesh, FLinearColor(0.06f, 0.06f, 0.07f));

	// Wing stub (left — partially attached)
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(200.f * S, 600.f * S, 350.f * S)),
		FVector(10.f * S, 2.f * S, 0.4f * S),
		Rot + FRotator(0.f, 15.f, -20.f), CubeMesh, HullCol);

	// Wing debris (right — broken off, on ground)
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(400.f * S, -900.f * S, 30.f * S)),
		FVector(8.f * S, 1.5f * S, 0.3f * S),
		FRotator(5.f, Yaw + 50.f, 0.f), CubeMesh, HullCol);

	// Tail section — vertical stabilizer
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(-1000.f * S, 0.f, 500.f * S)),
		FVector(0.3f * S, 3.f * S, 6.f * S),
		Rot + FRotator(0.f, 0.f, 15.f), CubeMesh, DarkCol);

	// Cockpit canopy — darkened glass
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(900.f * S, 0.f, 450.f * S)),
		FVector(4.f * S, 3.5f * S, 2.f * S), NoseRot, SphereMesh,
		FLinearColor(0.015f, 0.02f, 0.03f));

	// Smoke column — dark rising pillar (static, suggests recent crash)
	SpawnStaticMesh(Center + FVector(0.f, 0.f, 600.f * S),
		FVector(1.5f * S, 1.5f * S, 20.f * S), FRotator::ZeroRotator,
		CylinderMesh, FLinearColor(0.03f, 0.03f, 0.03f));

	// Fire glow inside wreck
	UPointLightComponent* FireGlow = NewObject<UPointLightComponent>(this);
	FireGlow->SetupAttachment(RootComponent);
	FireGlow->SetWorldLocation(Center + FVector(0.f, 0.f, 200.f * S));
	FireGlow->SetIntensity(5000.f * S);
	FireGlow->SetAttenuationRadius(1500.f * S);
	FireGlow->SetLightColor(FLinearColor(1.f, 0.4f, 0.08f));
	FireGlow->CastShadows = false;
	FireGlow->RegisterComponent();

	// Emissive damage glow along fuselage breach
	UStaticMeshComponent* Breach = SpawnStaticMesh(
		Center + Rot.RotateVector(FVector(300.f * S, 0.f, 350.f * S)),
		FVector(5.f * S, 3.f * S, 0.08f * S), Rot, CubeMesh,
		FLinearColor(0.8f, 0.3f, 0.05f));
	if (Breach)
	{
		UMaterialInterface* BreachEmissiveMat = FExoMaterialFactory::GetEmissiveAdditive();
		UMaterialInstanceDynamic* BM = UMaterialInstanceDynamic::Create(BreachEmissiveMat, this);
		if (!BM) { return; }
		BM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(10.f, 4.f, 0.8f));
		Breach->SetMaterial(0, BM);
	}
}

void AExoLevelBuilder::SpawnDebrisField(const FVector& Center, float Radius, int32 Count)
{
	FLinearColor MetalDark(0.04f, 0.04f, 0.05f);
	FLinearColor MetalMid(0.06f, 0.065f, 0.07f);

	for (int32 i = 0; i < Count; i++)
	{
		float Angle = FMath::RandRange(0.f, 2.f * PI);
		float Dist = FMath::RandRange(Radius * 0.1f, Radius);
		FVector Pos = Center + FVector(
			FMath::Cos(Angle) * Dist,
			FMath::Sin(Angle) * Dist,
			FMath::RandRange(0.f, 50.f));

		float SX = FMath::RandRange(0.3f, 2.5f);
		float SY = FMath::RandRange(0.2f, 1.5f);
		float SZ = FMath::RandRange(0.1f, 0.8f);
		FRotator Rot(FMath::RandRange(-25.f, 25.f),
			FMath::RandRange(0.f, 360.f),
			FMath::RandRange(-15.f, 15.f));

		UStaticMesh* Mesh = (i % 3 == 0) ? CylinderMesh : CubeMesh;
		FLinearColor Col = (i % 2 == 0) ? MetalDark : MetalMid;
		SpawnStaticMesh(Pos, FVector(SX, SY, SZ), Rot, Mesh, Col);
	}
}

void AExoLevelBuilder::SpawnScorchMark(const FVector& Center, float Radius, float Yaw)
{
	float Scale = Radius / 50.f;
	// Outer scorch ring — dark burned ground
	SpawnStaticMesh(Center, FVector(Scale, Scale, 0.02f),
		FRotator(0.f, Yaw, 0.f), CylinderMesh,
		FLinearColor(0.015f, 0.012f, 0.01f));
	// Inner concentrated burn
	SpawnStaticMesh(Center + FVector(0.f, 0.f, 1.f),
		FVector(Scale * 0.5f, Scale * 0.5f, 0.02f),
		FRotator(0.f, Yaw + 30.f, 0.f), CylinderMesh,
		FLinearColor(0.01f, 0.008f, 0.005f));
	// Radial blast lines (3 rays outward)
	for (int32 i = 0; i < 3; i++)
	{
		float RayAngle = Yaw + i * 120.f + FMath::RandRange(-20.f, 20.f);
		float RayLen = Radius * FMath::RandRange(0.6f, 1.2f);
		FRotator RayRot(0.f, RayAngle, 0.f);
		FVector RayPos = Center + RayRot.RotateVector(
			FVector(RayLen * 0.5f, 0.f, 2.f));
		SpawnStaticMesh(RayPos,
			FVector(RayLen / 100.f, 0.3f, 0.02f), RayRot,
			CubeMesh, FLinearColor(0.02f, 0.015f, 0.01f));
	}
}
