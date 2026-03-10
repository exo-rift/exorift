// ExoLevelBuilderSkybox.cpp — Sky sphere, nebulae, stars, planet, moon, orbital debris
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

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

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* SkyMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		// Deep space blue-purple gradient
		SkyMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.015f, 0.02f, 0.05f));
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

	// Star field — small bright spheres scattered across the sky
	if (SphereMesh && BaseMaterial)
	{
		int32 NumStars = 60;
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

			float StarScale = 8.f + (i % 5) * 4.f;

			UStaticMeshComponent* Star = NewObject<UStaticMeshComponent>(this);
			Star->SetupAttachment(RootComponent);
			Star->SetStaticMesh(SphereMesh);
			Star->SetWorldLocation(StarPos);
			Star->SetWorldScale3D(FVector(StarScale));
			Star->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Star->CastShadow = false;
			Star->RegisterComponent();

			FLinearColor StarCol;
			int32 ColorType = i % 10;
			if (ColorType < 6) StarCol = FLinearColor(10.f, 10.f, 12.f);
			else if (ColorType < 8) StarCol = FLinearColor(6.f, 8.f, 15.f);
			else if (ColorType < 9) StarCol = FLinearColor(15.f, 10.f, 4.f);
			else StarCol = FLinearColor(12.f, 12.f, 6.f);

			UMaterialInstanceDynamic* StarMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			StarMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.9f, 0.9f, 1.f));
			StarMat->SetVectorParameterValue(TEXT("EmissiveColor"), StarCol);
			Star->SetMaterial(0, StarMat);
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

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* PlanetMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		PlanetMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.12f, 0.08f, 0.04f));
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

		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			RingMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.08f, 0.06f, 0.04f));
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

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* MoonMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		MoonMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.5f, 0.55f, 0.6f));
		MoonMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.15f, 0.16f, 0.2f));
		Moon->SetMaterial(0, MoonMat);
	}

	// === ORBITAL DEBRIS — broken ship hulls from a recent battle ===
	if (CubeMesh && BaseMaterial)
	{
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

			UMaterialInstanceDynamic* DM = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			DM->SetVectorParameterValue(TEXT("BaseColor"), D.Col);
			// Faint edge glow from reflected planet light
			DM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(D.Col.R * 0.2f, D.Col.G * 0.15f, D.Col.B * 0.1f));
			Frag->SetMaterial(0, DM);
		}
	}
}
