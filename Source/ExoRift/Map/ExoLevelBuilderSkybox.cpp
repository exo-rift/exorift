// ExoLevelBuilderSkybox.cpp — Sky sphere, nebulae, stars, planet, moon, orbital debris
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"

void AExoLevelBuilder::BuildSkybox()
{
	// SkyAtmosphere handles the sky rendering now.
	// We only add celestial objects: stars, nebulae, planet, station, debris.
	if (!SphereMesh) return;

	// Distant nebula lights — subtle sky coloring that works with atmosphere
	float NebulaDist = 400000.f;

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
			if (ColorType < 6) StarCol = FLinearColor(8.f, 8.f, 10.f);         // White
			else if (ColorType < 9) StarCol = FLinearColor(5.f, 7.f, 15.f);    // Blue
			else if (ColorType < 11) StarCol = FLinearColor(15.f, 8.f, 3.f);   // Orange
			else if (ColorType < 13) StarCol = FLinearColor(12.f, 12.f, 5.f);  // Yellow
			else StarCol = FLinearColor(16.f, 4.f, 1.5f);                       // Red giant

			// Supergiants are a bit brighter
			if (i % 25 == 0) StarCol = StarCol * 1.3f;

			UMaterialInstanceDynamic* StarMat = UMaterialInstanceDynamic::Create(StarEmissiveMat, this);
			if (!StarMat) { continue; }
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
			{{-250000.f, 120000.f, 300000.f}, 40000.f, {0.18f, 0.045f, 0.34f}},
			{{-200000.f, 150000.f, 280000.f}, 30000.f, {0.14f, 0.07f, 0.27f}},
			{{-220000.f, 100000.f, 320000.f}, 25000.f, {0.22f, 0.022f, 0.22f}},
			// Teal nebula (lower right)
			{{200000.f, -160000.f, 180000.f}, 35000.f, {0.045f, 0.18f, 0.22f}},
			{{180000.f, -130000.f, 200000.f}, 28000.f, {0.022f, 0.22f, 0.18f}},
			// Warm orange wisps (near planet)
			{{160000.f, -80000.f, 140000.f}, 20000.f, {0.22f, 0.09f, 0.022f}},
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
			if (!CM) { continue; }
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
		if (!PlanetMat) { return; }
		PlanetMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.09f, 0.056f, 0.034f));
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
			if (!RingMat) { return; }
			RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.045f, 0.034f, 0.022f));
			PlanetRing->SetMaterial(0, RingMat);
		}
	}

	// Planet ambient light — warm glow from the planet
	UPointLightComponent* PlanetGlow = NewObject<UPointLightComponent>(this);
	PlanetGlow->SetupAttachment(RootComponent);
	PlanetGlow->SetWorldLocation(PlanetPos);
	PlanetGlow->SetIntensity(5000.f);
	PlanetGlow->SetAttenuationRadius(PlanetDist * 0.15f);
	PlanetGlow->SetLightColor(FLinearColor(0.55f, 0.34f, 0.18f));
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
		if (!MoonMat) { return; }
		MoonMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.34f, 0.36f, 0.45f));
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
			if (!M) { return; }
			M->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Col.R * 0.7f, Col.G * 0.7f, Col.B * 0.9f));
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
		StationLight->SetIntensity(3000.f);
		StationLight->SetAttenuationRadius(StationDist * 0.1f);
		StationLight->SetLightColor(FLinearColor(0.7f, 1.1f, 1.8f));
		StationLight->CastShadows = false;
		StationLight->RegisterComponent();
	}

	// === ASTEROID BELT — scattered rocky bodies near the planet ===
	if (SphereMesh)
	{
		UMaterialInterface* AsteroidMat = FExoMaterialFactory::GetEmissiveOpaque();
		float BeltCenter = PlanetDist * 0.4f;
		for (int32 i = 0; i < 20; i++)
		{
			float Angle = FMath::RandRange(0.f, 360.f);
			float Rad = FMath::DegreesToRadians(Angle);
			float Dist = BeltCenter + FMath::RandRange(-40000.f, 40000.f);
			float Elev = PlanetDist * 0.2f + FMath::RandRange(-30000.f, 30000.f);
			FVector APos(
				PlanetPos.X + FMath::Cos(Rad) * Dist,
				PlanetPos.Y + FMath::Sin(Rad) * Dist,
				Elev);
			float AScale = FMath::RandRange(30.f, 120.f);

			UStaticMeshComponent* Ast = NewObject<UStaticMeshComponent>(this);
			Ast->SetupAttachment(RootComponent);
			Ast->SetStaticMesh(SphereMesh);
			Ast->SetWorldLocation(APos);
			Ast->SetWorldScale3D(FVector(AScale, AScale * 0.7f, AScale * 0.5f));
			Ast->SetWorldRotation(FRotator(
				FMath::RandRange(-30.f, 30.f),
				FMath::RandRange(0.f, 360.f),
				FMath::RandRange(-20.f, 20.f)));
			Ast->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Ast->CastShadow = false;
			Ast->RegisterComponent();

			UMaterialInstanceDynamic* AM = UMaterialInstanceDynamic::Create(AsteroidMat, this);
			if (!AM) { continue; }
			AM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.07f + (i % 3) * 0.022f, 0.056f, 0.045f));
			Ast->SetMaterial(0, AM);
		}
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
			if (!DM) { continue; }
			DM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(D.Col.R * 1.8f, D.Col.G * 1.4f, D.Col.B * 1.1f));
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
			if (!DM) { continue; }
			DM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.09f, 0.08f, 0.07f));
			SmallFrag->SetMaterial(0, DM);
		}
	}
}
