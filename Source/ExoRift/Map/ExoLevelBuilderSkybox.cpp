// ExoLevelBuilderSkybox.cpp — Sky sphere, nebulae, stars, planet, moon, orbital debris
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"

void AExoLevelBuilder::BuildSkybox()
{
	// Large inverted sphere as skybox
	if (!SphereMesh) return;

	UStaticMeshComponent* SkySphere = NewObject<UStaticMeshComponent>(this);
	SkySphere->SetupAttachment(RootComponent);
	SkySphere->SetStaticMesh(SphereMesh);
	SkySphere->SetWorldLocation(FVector(0.f, 0.f, 0.f));
	SkySphere->SetWorldScale3D(FVector(-5000.f, -5000.f, -5000.f)); // Inverted normals
	SkySphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkySphere->CastShadow = false;
	SkySphere->RegisterComponent();

	{
		UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* SkyMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		// Deep space blue-purple gradient
		SkyMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.01f, 0.015f, 0.04f));
		SkySphere->SetMaterial(0, SkyMat);
	}

	// Nebula glow — colored spheres at far distance to tint the sky
	float NebulaDist = 400000.f;

	// Purple-blue nebula (upper left)
	UPointLightComponent* Nebula1 = NewObject<UPointLightComponent>(this);
	Nebula1->SetupAttachment(RootComponent);
	Nebula1->SetWorldLocation(FVector(-NebulaDist * 0.6f, NebulaDist * 0.3f, NebulaDist * 0.7f));
	Nebula1->SetIntensity(80000.f);
	Nebula1->SetAttenuationRadius(NebulaDist);
	Nebula1->SetLightColor(FLinearColor(0.15f, 0.05f, 0.3f)); // Purple
	Nebula1->CastShadows = false;
	Nebula1->RegisterComponent();

	// Teal-green nebula (lower right)
	UPointLightComponent* Nebula2 = NewObject<UPointLightComponent>(this);
	Nebula2->SetupAttachment(RootComponent);
	Nebula2->SetWorldLocation(FVector(NebulaDist * 0.5f, -NebulaDist * 0.4f, NebulaDist * 0.3f));
	Nebula2->SetIntensity(50000.f);
	Nebula2->SetAttenuationRadius(NebulaDist * 0.8f);
	Nebula2->SetLightColor(FLinearColor(0.02f, 0.15f, 0.12f)); // Teal
	Nebula2->CastShadows = false;
	Nebula2->RegisterComponent();

	// Star field — dense scattered spheres across the sky dome
	if (SphereMesh)
	{
		UMaterialInterface* StarEmissiveMat = FExoMaterialFactory::GetEmissiveAdditive();
		int32 NumStars = 150;
		for (int32 i = 0; i < NumStars; i++)
		{
			float Seed = i * 137.508f;
			float Phi = FMath::Acos(1.f - 2.f * FMath::Fmod(Seed * 0.381966f, 1.f));
			float Theta = 2.f * PI * FMath::Fmod(Seed * 0.618034f, 1.f);
			if (Phi > PI * 0.6f) continue;

			float R = 350000.f + (i % 7) * 20000.f;
			FVector StarPos(
				R * FMath::Sin(Phi) * FMath::Cos(Theta),
				R * FMath::Sin(Phi) * FMath::Sin(Theta),
				R * FMath::Cos(Phi));

			// Size variety: mostly tiny, a few bright supergiant stars
			float StarScale;
			if (i % 25 == 0) StarScale = 20.f + (i % 3) * 8.f; // Bright supergiant
			else if (i % 8 == 0) StarScale = 12.f + (i % 4) * 3.f; // Medium
			else StarScale = 5.f + (i % 5) * 2.f; // Small background star

			UStaticMeshComponent* Star = NewObject<UStaticMeshComponent>(this);
			Star->SetupAttachment(RootComponent);
			Star->SetStaticMesh(SphereMesh);
			Star->SetWorldLocation(StarPos);
			Star->SetWorldScale3D(FVector(StarScale));
			Star->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Star->CastShadow = false;
			Star->RegisterComponent();

			// More color variety: white, blue, orange, yellow, red giant
			FLinearColor StarCol;
			int32 ColorType = i % 15;
			if (ColorType < 6) StarCol = FLinearColor(10.f, 10.f, 12.f);       // White
			else if (ColorType < 9) StarCol = FLinearColor(6.f, 8.f, 18.f);    // Blue
			else if (ColorType < 11) StarCol = FLinearColor(18.f, 10.f, 3.f);  // Orange
			else if (ColorType < 13) StarCol = FLinearColor(14.f, 14.f, 6.f);  // Yellow
			else StarCol = FLinearColor(20.f, 5.f, 2.f);                        // Red giant

			// Supergiants are extra bright
			if (i % 25 == 0) StarCol = StarCol * 1.5f;

			UMaterialInstanceDynamic* StarMat = UMaterialInstanceDynamic::Create(StarEmissiveMat, this);
			StarMat->SetVectorParameterValue(TEXT("EmissiveColor"), StarCol);
			Star->SetMaterial(0, StarMat);
		}
	}

	// Nebula clouds — large soft emissive spheres for visible color regions
	if (SphereMesh)
	{
		UMaterialInterface* NebMat = FExoMaterialFactory::GetEmissiveAdditive();
		struct FNebCloud { FVector Pos; float Scale; FLinearColor Col; };
		TArray<FNebCloud> Clouds = {
			// Purple nebula cluster (upper left)
			{{-250000.f, 120000.f, 300000.f}, 40000.f, {0.08f, 0.02f, 0.15f}},
			{{-200000.f, 150000.f, 280000.f}, 30000.f, {0.06f, 0.03f, 0.12f}},
			{{-220000.f, 100000.f, 320000.f}, 25000.f, {0.10f, 0.01f, 0.10f}},
			// Teal nebula (lower right)
			{{200000.f, -160000.f, 180000.f}, 35000.f, {0.02f, 0.08f, 0.10f}},
			{{180000.f, -130000.f, 200000.f}, 28000.f, {0.01f, 0.10f, 0.08f}},
			// Warm orange wisps (near planet)
			{{160000.f, -80000.f, 140000.f}, 20000.f, {0.10f, 0.04f, 0.01f}},
		};
		for (const auto& C : Clouds)
		{
			UStaticMeshComponent* Cloud = NewObject<UStaticMeshComponent>(this);
			Cloud->SetupAttachment(RootComponent);
			Cloud->SetStaticMesh(SphereMesh);
			Cloud->SetWorldLocation(C.Pos);
			Cloud->SetWorldScale3D(FVector(C.Scale));
			Cloud->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Cloud->CastShadow = false;
			Cloud->RegisterComponent();
			UMaterialInstanceDynamic* CM = UMaterialInstanceDynamic::Create(NebMat, this);
			CM->SetVectorParameterValue(TEXT("EmissiveColor"), C.Col);
			Cloud->SetMaterial(0, CM);
		}
	}

	// === PLANET on the horizon — large gas giant with rings ===
	float PlanetDist = 380000.f;
	FVector PlanetPos(PlanetDist * 0.5f, -PlanetDist * 0.3f, PlanetDist * 0.25f);

	// Planet body — large sphere with banded colors
	UStaticMeshComponent* Planet = NewObject<UStaticMeshComponent>(this);
	Planet->SetupAttachment(RootComponent);
	Planet->SetStaticMesh(SphereMesh);
	Planet->SetWorldLocation(PlanetPos);
	Planet->SetWorldScale3D(FVector(2500.f));
	Planet->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Planet->CastShadow = false;
	Planet->RegisterComponent();

	{
		UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* PlanetMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		PlanetMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.04f, 0.025f, 0.015f));
		Planet->SetMaterial(0, PlanetMat);
	}

	// Planet ring — flat cylinder tilted
	if (CylinderMesh)
	{
		UStaticMeshComponent* PlanetRing = NewObject<UStaticMeshComponent>(this);
		PlanetRing->SetupAttachment(RootComponent);
		PlanetRing->SetStaticMesh(CylinderMesh);
		PlanetRing->SetWorldLocation(PlanetPos);
		PlanetRing->SetWorldScale3D(FVector(4000.f, 4000.f, 5.f));
		PlanetRing->SetWorldRotation(FRotator(25.f, 15.f, 0.f));
		PlanetRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PlanetRing->CastShadow = false;
		PlanetRing->RegisterComponent();

		{
			UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
			UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
			RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.02f, 0.015f, 0.01f));
			PlanetRing->SetMaterial(0, RingMat);
		}
	}

	// Planet ambient light — warm glow from the planet
	UPointLightComponent* PlanetGlow = NewObject<UPointLightComponent>(this);
	PlanetGlow->SetupAttachment(RootComponent);
	PlanetGlow->SetWorldLocation(PlanetPos);
	PlanetGlow->SetIntensity(40000.f);
	PlanetGlow->SetAttenuationRadius(PlanetDist * 0.5f);
	PlanetGlow->SetLightColor(FLinearColor(0.25f, 0.15f, 0.08f));
	PlanetGlow->CastShadows = false;
	PlanetGlow->RegisterComponent();

	// === MOON — smaller, brighter, opposite side of sky ===
	FVector MoonPos(-PlanetDist * 0.4f, PlanetDist * 0.5f, PlanetDist * 0.4f);

	UStaticMeshComponent* Moon = NewObject<UStaticMeshComponent>(this);
	Moon->SetupAttachment(RootComponent);
	Moon->SetStaticMesh(SphereMesh);
	Moon->SetWorldLocation(MoonPos);
	Moon->SetWorldScale3D(FVector(400.f));
	Moon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Moon->CastShadow = false;
	Moon->RegisterComponent();

	{
		UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* MoonMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		MoonMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.15f, 0.16f, 0.2f));
		Moon->SetMaterial(0, MoonMat);
	}

	// === ORBITAL SPACE STATION — visible structure in orbit ===
	if (CubeMesh && CylinderMesh)
	{
		float StationDist = 280000.f;
		FVector StationPos(StationDist * 0.1f, StationDist * 0.3f, StationDist * 0.55f);
		FLinearColor StationHull(0.08f, 0.08f, 0.1f);
		FLinearColor StationAccent(0.06f, 0.06f, 0.08f);
		UMaterialInterface* StationEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();

		// Central hub module
		auto AddStationPart = [&](const FVector& Offset, const FVector& Scale,
			const FRotator& Rot, UStaticMesh* Mesh, const FLinearColor& Col)
		{
			UStaticMeshComponent* P = NewObject<UStaticMeshComponent>(this);
			P->SetupAttachment(RootComponent);
			P->SetStaticMesh(Mesh);
			P->SetWorldLocation(StationPos + Offset);
			P->SetWorldScale3D(Scale);
			P->SetWorldRotation(Rot);
			P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			P->CastShadow = false;
			P->RegisterComponent();
			UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(StationEmissiveMat, this);
			M->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Col.R * 0.3f, Col.G * 0.3f, Col.B * 0.4f));
			P->SetMaterial(0, M);
		};

		// Main cylinder body
		AddStationPart(FVector::ZeroVector, FVector(200.f, 200.f, 600.f),
			FRotator(0.f, 0.f, 90.f), CylinderMesh, StationHull);
		// Solar panel arrays (4 flat rectangles)
		for (int32 i = 0; i < 4; i++)
		{
			float Angle = i * 90.f;
			FVector PanelOffset(
				FMath::Cos(FMath::DegreesToRadians(Angle)) * 800.f,
				FMath::Sin(FMath::DegreesToRadians(Angle)) * 800.f, 0.f);
			AddStationPart(PanelOffset, FVector(50.f, 500.f, 5.f),
				FRotator(0.f, Angle, 0.f), CubeMesh,
				FLinearColor(0.02f, 0.03f, 0.06f));
		}
		// Docking ring at each end
		AddStationPart(FVector(0.f, 0.f, 35000.f), FVector(250.f, 250.f, 30.f),
			FRotator::ZeroRotator, CylinderMesh, StationAccent);
		AddStationPart(FVector(0.f, 0.f, -35000.f), FVector(250.f, 250.f, 30.f),
			FRotator::ZeroRotator, CylinderMesh, StationAccent);

		// Running lights on station
		UPointLightComponent* StationLight = NewObject<UPointLightComponent>(this);
		StationLight->SetupAttachment(RootComponent);
		StationLight->SetWorldLocation(StationPos);
		StationLight->SetIntensity(30000.f);
		StationLight->SetAttenuationRadius(StationDist * 0.3f);
		StationLight->SetLightColor(FLinearColor(0.3f, 0.5f, 0.8f));
		StationLight->CastShadows = false;
		StationLight->RegisterComponent();
	}

	// === ORBITAL DEBRIS — broken ship hulls from a recent battle ===
	if (CubeMesh)
	{
		UMaterialInterface* DebrisEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		struct FDebris { FVector Pos; FVector Scale; FRotator Rot; FLinearColor Col; };
		float OrbDist = 300000.f;
		TArray<FDebris> Debris = {
			// Large hull fragment
			{{OrbDist * 0.3f, OrbDist * 0.2f, OrbDist * 0.45f},
				{800.f, 200.f, 60.f}, {15.f, 45.f, 10.f},
				FLinearColor(0.06f, 0.06f, 0.08f)},
			// Broken wing section
			{{-OrbDist * 0.25f, OrbDist * 0.1f, OrbDist * 0.5f},
				{400.f, 600.f, 20.f}, {-20.f, 120.f, 35.f},
				FLinearColor(0.05f, 0.055f, 0.07f)},
			// Engine block
			{{OrbDist * 0.15f, -OrbDist * 0.3f, OrbDist * 0.35f},
				{150.f, 150.f, 350.f}, {40.f, -30.f, 15.f},
				FLinearColor(0.07f, 0.065f, 0.06f)},
			// Small hull plate
			{{-OrbDist * 0.4f, -OrbDist * 0.15f, OrbDist * 0.55f},
				{300.f, 180.f, 15.f}, {-10.f, 200.f, -25.f},
				FLinearColor(0.05f, 0.05f, 0.065f)},
		};

		for (const auto& D : Debris)
		{
			UStaticMeshComponent* Frag = NewObject<UStaticMeshComponent>(this);
			Frag->SetupAttachment(RootComponent);
			Frag->SetStaticMesh(CubeMesh);
			Frag->SetWorldLocation(D.Pos);
			Frag->SetWorldScale3D(D.Scale);
			Frag->SetWorldRotation(D.Rot);
			Frag->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Frag->CastShadow = false;
			Frag->RegisterComponent();

			// Visible hull glow — sunlit edge + subtle warmth from planet
			UMaterialInstanceDynamic* DM = UMaterialInstanceDynamic::Create(DebrisEmissiveMat, this);
			DM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(D.Col.R * 0.8f, D.Col.G * 0.6f, D.Col.B * 0.5f));
			Frag->SetMaterial(0, DM);
		}

		// Additional small debris fragments for a denser field
		for (int32 i = 0; i < 8; i++)
		{
			float Angle = i * 45.f + FMath::RandRange(-15.f, 15.f);
			float Elev = FMath::RandRange(0.3f, 0.6f);
			float Dist = OrbDist * FMath::RandRange(0.8f, 1.1f);
			FVector Pos(
				FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist * (1.f - Elev),
				FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist * (1.f - Elev),
				Dist * Elev);
			FVector Scale(
				FMath::RandRange(60.f, 200.f),
				FMath::RandRange(40.f, 120.f),
				FMath::RandRange(10.f, 40.f));

			UStaticMeshComponent* SmallFrag = NewObject<UStaticMeshComponent>(this);
			SmallFrag->SetupAttachment(RootComponent);
			SmallFrag->SetStaticMesh(CubeMesh);
			SmallFrag->SetWorldLocation(Pos);
			SmallFrag->SetWorldScale3D(Scale);
			SmallFrag->SetWorldRotation(FRotator(
				FMath::RandRange(-30.f, 30.f),
				FMath::RandRange(0.f, 360.f),
				FMath::RandRange(-20.f, 20.f)));
			SmallFrag->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SmallFrag->CastShadow = false;
			SmallFrag->RegisterComponent();

			UMaterialInstanceDynamic* DM = UMaterialInstanceDynamic::Create(DebrisEmissiveMat, this);
			DM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.04f, 0.035f, 0.03f));
			SmallFrag->SetMaterial(0, DM);
		}
	}
}
