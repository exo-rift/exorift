// ExoLevelBuilderDetail.cpp — Ground panels, energy pylons, craters
// Interiors moved to ExoLevelBuilderInteriors.cpp
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoEnvironmentAnimator.h"
#include "Visual/ExoMaterialFactory.h"

void AExoLevelBuilder::BuildGroundDetail()
{
	// Sci-fi floor panels around each compound
	SpawnFloorPanels(FVector(0.f, 0.f, 2.f), 6000.f, 24);          // Central hub
	SpawnFloorPanels(FVector(0.f, 80000.f, 2.f), 5000.f, 18);      // North
	SpawnFloorPanels(FVector(0.f, -80000.f, 2.f), 4500.f, 16);     // South
	SpawnFloorPanels(FVector(80000.f, 0.f, 2.f), 5000.f, 18);      // East
	SpawnFloorPanels(FVector(-80000.f, 0.f, 2.f), 4000.f, 14);     // West

	// Energy pylons at key intersections and compound entries
	FLinearColor CyanGlow(0.1f, 0.6f, 1.f);
	FLinearColor TealGlow(0.05f, 0.8f, 0.5f);
	FLinearColor AmberGlow(1.f, 0.6f, 0.1f);

	// Hub perimeter pylons
	SpawnEnergyPylon(FVector(5000.f, 4000.f, 0.f), 800.f, CyanGlow);
	SpawnEnergyPylon(FVector(-5000.f, 4000.f, 0.f), 800.f, CyanGlow);
	SpawnEnergyPylon(FVector(5000.f, -4000.f, 0.f), 800.f, CyanGlow);
	SpawnEnergyPylon(FVector(-5000.f, -4000.f, 0.f), 800.f, CyanGlow);

	// North compound gate pylons
	SpawnEnergyPylon(FVector(-4000.f, 73000.f, 0.f), 1000.f, TealGlow);
	SpawnEnergyPylon(FVector(4000.f, 73000.f, 0.f), 1000.f, TealGlow);

	// East power station pylons — amber warning glow
	SpawnEnergyPylon(FVector(73000.f, -3000.f, 0.f), 1200.f, AmberGlow);
	SpawnEnergyPylon(FVector(73000.f, 3000.f, 0.f), 1200.f, AmberGlow);

	// South research lab entry
	SpawnEnergyPylon(FVector(3000.f, -73000.f, 0.f), 900.f, CyanGlow);
	SpawnEnergyPylon(FVector(-3000.f, -73000.f, 0.f), 900.f, CyanGlow);

	// West barracks
	SpawnEnergyPylon(FVector(-73000.f, 5000.f, 0.f), 700.f, TealGlow);
	SpawnEnergyPylon(FVector(-73000.f, -5000.f, 0.f), 700.f, TealGlow);

	// Craters scattered across the battlefield
	SpawnCrater(FVector(25000.f, 15000.f, 0.f), 2000.f);
	SpawnCrater(FVector(-35000.f, -25000.f, 0.f), 3000.f);
	SpawnCrater(FVector(55000.f, -30000.f, 0.f), 1500.f);
	SpawnCrater(FVector(-15000.f, 60000.f, 0.f), 2500.f);
	SpawnCrater(FVector(70000.f, 70000.f, 0.f), 1800.f);
	SpawnCrater(FVector(-90000.f, -60000.f, 0.f), 3500.f);
	SpawnCrater(FVector(40000.f, -80000.f, 0.f), 2200.f);

	// === AMBIENT GROUND LIGHTS ===
	// Very subtle ground-level lights at compound approaches only (not open field)
	struct FGroundGlow { FVector Pos; FLinearColor Color; float Intensity; float Radius; };
	TArray<FGroundGlow> Glows = {
		// Hub approach — cool blue (subtle)
		{{15000.f, 0.f, 20.f}, {0.1f, 0.3f, 0.8f}, 800.f, 2000.f},
		{{-15000.f, 0.f, 20.f}, {0.1f, 0.3f, 0.8f}, 800.f, 2000.f},
		// North industrial — warm amber
		{{0.f, 70000.f, 20.f}, {0.8f, 0.5f, 0.1f}, 600.f, 2000.f},
		// South research — teal
		{{0.f, -70000.f, 20.f}, {0.1f, 0.6f, 0.5f}, 600.f, 2000.f},
		// East power — red warning
		{{70000.f, 0.f, 20.f}, {0.8f, 0.15f, 0.05f}, 700.f, 2000.f},
		// West barracks — green
		{{-70000.f, 0.f, 20.f}, {0.1f, 0.5f, 0.2f}, 600.f, 2000.f},
	};
	for (const auto& G : Glows)
	{
		UPointLightComponent* GL = NewObject<UPointLightComponent>(this);
		GL->SetupAttachment(RootComponent);
		GL->SetWorldLocation(G.Pos);
		GL->SetIntensity(G.Intensity);
		GL->SetAttenuationRadius(G.Radius);
		GL->SetLightColor(G.Color);
		GL->CastShadows = false;
		GL->RegisterComponent();
	}

	// Ground mist pools — low-lying glowing haze near water and valleys
	SpawnGroundMist(FVector(30000.f, -20000.f, 5.f), 4000.f, FLinearColor(0.1f, 0.15f, 0.25f));
	SpawnGroundMist(FVector(-20000.f, 30000.f, 5.f), 3500.f, FLinearColor(0.1f, 0.15f, 0.25f));
	SpawnGroundMist(FVector(80000.f, 5000.f, 5.f), 5000.f, FLinearColor(0.15f, 0.1f, 0.08f));
	SpawnGroundMist(FVector(-5000.f, -80000.f, 5.f), 3000.f, FLinearColor(0.08f, 0.15f, 0.12f));
	SpawnGroundMist(FVector(-80000.f, 8000.f, 5.f), 3000.f, FLinearColor(0.08f, 0.12f, 0.08f));
	// River channel mist
	SpawnGroundMist(FVector(40000.f, -40000.f, 10.f), 6000.f, FLinearColor(0.08f, 0.1f, 0.2f));
	SpawnGroundMist(FVector(-40000.f, 40000.f, 10.f), 5000.f, FLinearColor(0.08f, 0.1f, 0.2f));

	// Ground clutter — small debris near compounds
	SpawnGroundClutter(FVector(0.f, 0.f, 0.f), 8000.f, 30);          // Hub
	SpawnGroundClutter(FVector(0.f, 80000.f, 0.f), 6000.f, 25);      // North
	SpawnGroundClutter(FVector(0.f, -80000.f, 0.f), 6000.f, 20);     // South
	SpawnGroundClutter(FVector(80000.f, 0.f, 0.f), 6000.f, 22);      // East
	SpawnGroundClutter(FVector(-80000.f, 0.f, 0.f), 5000.f, 18);     // West

	// Hazard floor markings near dangerous areas
	FLinearColor YellowStripe(0.6f, 0.5f, 0.05f);
	struct FHazardMark { FVector Pos; float Yaw; };
	TArray<FHazardMark> Marks = {
		{{-50000.f, 50000.f, 3.f}, 0.f},    // Radiation zone
		{{60000.f, -60000.f, 3.f}, 45.f},    // Electric zone
		{{-100000.f, -40000.f, 3.f}, -15.f}, // Toxic zone
		{{40000.f, 100000.f, 3.f}, 30.f},    // Fire zone
	};
	// Hazard-specific tint colors for area lighting
	FLinearColor HazardTints[] = {
		FLinearColor(0.15f, 0.6f, 0.05f),  // Radiation — green
		FLinearColor(0.2f, 0.4f, 0.9f),    // Electric — blue
		FLinearColor(0.5f, 0.15f, 0.6f),   // Toxic — purple
		FLinearColor(0.9f, 0.25f, 0.05f),  // Fire — orange
	};
	for (int32 i = 0; i < Marks.Num(); i++)
	{
		const auto& M = Marks[i];
		// Warning stripe pair
		SpawnStaticMesh(M.Pos + FVector(500.f, 0.f, 0.f),
			FVector(0.5f, 20.f, 0.05f), FRotator(0.f, M.Yaw, 0.f),
			CubeMesh, YellowStripe);
		SpawnStaticMesh(M.Pos + FVector(-500.f, 0.f, 0.f),
			FVector(0.5f, 20.f, 0.05f), FRotator(0.f, M.Yaw, 0.f),
			CubeMesh, YellowStripe);

		// Hazard zone area tint light (subtle, localized)
		UPointLightComponent* HZ = NewObject<UPointLightComponent>(this);
		HZ->SetupAttachment(RootComponent);
		HZ->SetWorldLocation(M.Pos + FVector(0.f, 0.f, 300.f));
		HZ->SetIntensity(3000.f);
		HZ->SetAttenuationRadius(5000.f);
		HZ->SetLightColor(HazardTints[i]);
		HZ->CastShadows = false;
		HZ->RegisterComponent();

		// Emissive ground pool at hazard center
		UStaticMeshComponent* HPool = SpawnStaticMesh(
			M.Pos + FVector(0.f, 0.f, 2.f),
			FVector(40.f, 40.f, 0.02f), FRotator::ZeroRotator,
			CylinderMesh, HazardTints[i]);
		if (HPool)
		{
			UMaterialInterface* HMat = FExoMaterialFactory::GetEmissiveAdditive();
			UMaterialInstanceDynamic* HD = UMaterialInstanceDynamic::Create(HMat, this);
			if (!HD) { return; }
			HD->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(HazardTints[i].R * 1.2f, HazardTints[i].G * 1.2f,
					HazardTints[i].B * 1.2f));
			HPool->SetMaterial(0, HD);
		}
	}
}

void AExoLevelBuilder::SpawnFloorPanels(const FVector& Center, float Radius, int32 Count)
{
	FLinearColor PanelLight(0.055f, 0.06f, 0.07f);
	FLinearColor PanelDark(0.035f, 0.04f, 0.045f);
	FLinearColor SeamGlow(0.05f, 0.2f, 0.4f);

	for (int32 i = 0; i < Count; i++)
	{
		float Angle = (float)i / (float)Count * 2.f * PI;
		float R = Radius * (0.4f + 0.6f * FMath::Fmod(i * 0.618034f, 1.f));
		FVector Pos = Center + FVector(
			FMath::Cos(Angle) * R,
			FMath::Sin(Angle) * R, 0.f);

		// Alternating panel sizes and colors
		float PanelSize = 400.f + (i % 3) * 200.f;
		float PanelScale = PanelSize / 100.f;
		FLinearColor Col = (i % 2 == 0) ? PanelLight : PanelDark;

		SpawnStaticMesh(Pos,
			FVector(PanelScale, PanelScale, 0.02f),
			FRotator(0.f, i * 47.f, 0.f),
			CubeMesh, Col);

		// Thin glowing seam line on every other panel
		if (i % 3 == 0)
		{
			UStaticMeshComponent* Seam = SpawnStaticMesh(
				Pos + FVector(0.f, 0.f, 1.f),
				FVector(PanelScale * 0.9f, 0.03f, 0.03f),
				FRotator(0.f, i * 47.f, 0.f),
				CubeMesh, SeamGlow);
			if (Seam)
			{
				UMaterialInterface* SeamEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
				UMaterialInstanceDynamic* SeamMat = UMaterialInstanceDynamic::Create(SeamEmissiveMat, this);
				if (!SeamMat) { return; }
				SeamMat->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(0.15f, 0.6f, 1.2f));
				Seam->SetMaterial(0, SeamMat);
			}
		}
	}
}

void AExoLevelBuilder::SpawnEnergyPylon(const FVector& Base, float Height,
	const FLinearColor& Color)
{
	// Dark metal column
	float ColH = Height / 100.f;
	SpawnStaticMesh(
		Base + FVector(0.f, 0.f, Height * 0.5f),
		FVector(0.6f, 0.6f, ColH),
		FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.06f, 0.06f, 0.08f));

	// Energy ring at top
	UStaticMeshComponent* Ring = SpawnStaticMesh(
		Base + FVector(0.f, 0.f, Height * 0.85f),
		FVector(1.0f, 1.0f, 0.06f),
		FRotator::ZeroRotator, CylinderMesh, Color);
	if (Ring)
	{
		UMaterialInterface* RingEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(RingEmissiveMat, this);
		if (!RingMat) { return; }
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 3.f, Color.G * 3.f, Color.B * 3.f));
		Ring->SetMaterial(0, RingMat);
	}

	// Energy ring at mid
	UStaticMeshComponent* MidRing = SpawnStaticMesh(
		Base + FVector(0.f, 0.f, Height * 0.45f),
		FVector(0.8f, 0.8f, 0.04f),
		FRotator::ZeroRotator, CylinderMesh, Color);
	if (MidRing)
	{
		UMaterialInterface* MidEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* MidMat = UMaterialInstanceDynamic::Create(MidEmissiveMat, this);
		if (!MidMat) { return; }
		MidMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 2.f, Color.G * 2.f, Color.B * 2.f));
		MidRing->SetMaterial(0, MidMat);
	}

	// Cap sphere
	SpawnStaticMesh(
		Base + FVector(0.f, 0.f, Height),
		FVector(0.7f, 0.7f, 0.5f),
		FRotator::ZeroRotator, SphereMesh,
		FLinearColor(0.08f, 0.08f, 0.1f));

	// Point light at top (localized glow only)
	UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
	Light->SetupAttachment(RootComponent);
	Light->SetWorldLocation(Base + FVector(0.f, 0.f, Height));
	Light->SetIntensity(3000.f);
	Light->SetAttenuationRadius(Height * 1.5f);
	Light->SetLightColor(Color);
	Light->CastShadows = false;
	Light->RegisterComponent();

	// Register ring for rotation + pulse animation
	if (AExoEnvironmentAnimator* Anim = AExoEnvironmentAnimator::Get(GetWorld()))
	{
		Anim->RegisterPylonRing(Ring, Light);
	}
}

void AExoLevelBuilder::SpawnCrater(const FVector& Center, float Radius)
{
	// Crater rim — ring of raised terrain
	float RimScale = Radius / 50.f;
	SpawnStaticMesh(
		Center + FVector(0.f, 0.f, -20.f),
		FVector(RimScale, RimScale, 0.4f),
		FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.03f, 0.035f, 0.04f));

	// Inner depression — darker center
	float InnerScale = Radius * 0.7f / 50.f;
	SpawnStaticMesh(
		Center + FVector(0.f, 0.f, -40.f),
		FVector(InnerScale, InnerScale, 0.3f),
		FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.02f, 0.025f, 0.03f));

	// Scattered debris chunks around rim
	int32 NumDebris = FMath::RandRange(3, 6);
	for (int32 i = 0; i < NumDebris; i++)
	{
		float Angle = (float)i / (float)NumDebris * 2.f * PI + FMath::RandRange(-0.3f, 0.3f);
		float Dist = Radius * FMath::RandRange(0.8f, 1.2f);
		FVector DebrisPos = Center + FVector(
			FMath::Cos(Angle) * Dist,
			FMath::Sin(Angle) * Dist,
			FMath::RandRange(0.f, 30.f));
		float S = FMath::RandRange(0.3f, 1.2f);
		SpawnStaticMesh(DebrisPos,
			FVector(S, S * 0.6f, S * 0.4f),
			FRotator(FMath::RandRange(-15.f, 15.f), FMath::RandRange(0.f, 360.f), 0.f),
			CubeMesh, FLinearColor(0.04f, 0.04f, 0.05f));
	}
}

void AExoLevelBuilder::SpawnGroundMist(const FVector& Center, float Radius,
	const FLinearColor& Color)
{
	// Large flat emissive cylinder = glowing low-lying mist pool
	float S = Radius / 50.f;
	UStaticMeshComponent* Mist = SpawnStaticMesh(Center,
		FVector(S, S, 0.01f), FRotator::ZeroRotator, CylinderMesh, Color);
	if (Mist)
	{
		UMaterialInterface* Mat = FExoMaterialFactory::GetEmissiveAdditive();
		UMaterialInstanceDynamic* MM = UMaterialInstanceDynamic::Create(Mat, this);
		if (!MM) { return; }
		MM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 0.5f, Color.G * 0.5f, Color.B * 0.5f));
		Mist->SetMaterial(0, MM);
	}
	// Subtle glow light
	UPointLightComponent* GL = NewObject<UPointLightComponent>(this);
	GL->SetupAttachment(RootComponent);
	GL->SetWorldLocation(Center + FVector(0.f, 0.f, 50.f));
	GL->SetIntensity(1000.f);
	GL->SetAttenuationRadius(Radius * 0.6f);
	GL->SetLightColor(Color);
	GL->CastShadows = false;
	GL->RegisterComponent();
}

void AExoLevelBuilder::SpawnGroundClutter(const FVector& Center, float Radius, int32 Count)
{
	FLinearColor DarkMetal(0.04f, 0.04f, 0.05f);
	FLinearColor RustMetal(0.06f, 0.04f, 0.03f);
	for (int32 i = 0; i < Count; i++)
	{
		float Angle = FMath::RandRange(0.f, 2.f * PI);
		float Dist = FMath::RandRange(Radius * 0.2f, Radius);
		FVector Pos = Center + FVector(
			FMath::Cos(Angle) * Dist,
			FMath::Sin(Angle) * Dist, 0.f);
		float Yaw = FMath::RandRange(0.f, 360.f);
		FLinearColor Col = (i % 3 == 0) ? RustMetal : DarkMetal;

		if (i % 4 == 0)
		{
			// Small rock/rubble (sphere)
			float S = FMath::RandRange(0.15f, 0.5f);
			SpawnStaticMesh(Pos + FVector(0.f, 0.f, S * 25.f),
				FVector(S, S * 0.8f, S * 0.6f),
				FRotator(FMath::RandRange(-10.f, 10.f), Yaw, 0.f),
				SphereMesh, Col);
		}
		else
		{
			// Metal fragment (stretched cube)
			float SX = FMath::RandRange(0.2f, 0.8f);
			float SY = FMath::RandRange(0.1f, 0.4f);
			float SZ = FMath::RandRange(0.02f, 0.12f);
			SpawnStaticMesh(Pos + FVector(0.f, 0.f, SZ * 50.f),
				FVector(SX, SY, SZ),
				FRotator(FMath::RandRange(-5.f, 5.f), Yaw, FMath::RandRange(-3.f, 3.f)),
				CubeMesh, Col);
		}
	}
}
