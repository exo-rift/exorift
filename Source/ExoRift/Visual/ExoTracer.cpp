// ExoTracer.cpp — Dramatic sci-fi energy bolt with corona, sparks, and lighting
#include "Visual/ExoTracer.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoTracer::AExoTracer()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 4.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFinder.Succeeded()) CylinderMesh = CylFinder.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded()) SphereMesh = SphereFinder.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFinder.Succeeded()) CubeMesh = CubeFinder.Object;

	auto MakeMesh = [&](const TCHAR* Name, UStaticMesh* Mesh) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Comp = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (Mesh) Comp->SetStaticMesh(Mesh);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Comp->CastShadow = false;
		Comp->SetGenerateOverlapEvents(false);
		return Comp;
	};

	BeamCore = MakeMesh(TEXT("BeamCore"), CylinderMesh);
	RootComponent = BeamCore;

	BeamGlow = MakeMesh(TEXT("BeamGlow"), CylinderMesh);
	BeamGlow->SetupAttachment(BeamCore);

	BeamTrail = MakeMesh(TEXT("BeamTrail"), CylinderMesh);
	BeamTrail->SetupAttachment(BeamCore);

	Corona = MakeMesh(TEXT("Corona"), CylinderMesh);
	Corona->SetupAttachment(BeamCore);

	HeadMesh = MakeMesh(TEXT("HeadMesh"), SphereMesh);
	HeadMesh->SetupAttachment(BeamCore);

	// Trailing sparks
	for (int32 i = 0; i < NUM_SPARKS; i++)
	{
		FName SName = *FString::Printf(TEXT("Spark_%d"), i);
		UStaticMeshComponent* S = MakeMesh(*SName.ToString(), CubeMesh);
		S->SetupAttachment(BeamCore);
		SparkMeshes.Add(S);
	}

	// Helix orbiters — small energy spheres spiraling around the beam
	for (int32 i = 0; i < NUM_HELIX; i++)
	{
		FName HName = *FString::Printf(TEXT("HelixOrb_%d"), i);
		UStaticMeshComponent* H = MakeMesh(*HName.ToString(), SphereMesh);
		H->SetupAttachment(BeamCore);
		HelixOrbs.Add(H);
	}

	// Head shockwave ring
	HeadRing = MakeMesh(TEXT("HeadRing"), CylinderMesh);
	HeadRing->SetupAttachment(BeamCore);

	HeadLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("HeadLight"));
	HeadLight->SetupAttachment(BeamCore);
	HeadLight->SetIntensity(55000.f);
	HeadLight->SetAttenuationRadius(1500.f);
	HeadLight->CastShadows = false;

	TailLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TailLight"));
	TailLight->SetupAttachment(BeamCore);
	TailLight->SetIntensity(15000.f);
	TailLight->SetAttenuationRadius(800.f);
	TailLight->CastShadows = false;
}

void AExoTracer::InitTracer(const FVector& Start, const FVector& End, bool bIsHit,
	const FLinearColor& WeaponColor, EWeaponType WeaponType)
{
	StartPos = Start;
	EndPos = End;
	Direction = (End - Start).GetSafeNormal();
	TotalDistance = FVector::Distance(Start, End);
	BaseWeaponColor = WeaponColor;

	if (TotalDistance < 1.f) { Destroy(); return; }

	// Weapon-specific beam parameters — thick, dramatic sci-fi energy bolts
	switch (WeaponType)
	{
	case EWeaponType::Rifle:
		CoreRadius = 0.40f; GlowRadius = 1.0f; TrailRadius = 0.20f;
		CoronaRadius = 2.0f; HeadScale = 2.0f; BeamLength = 3200.f;
		TravelSpeed = 80000.f; LightIntensity = 200000.f; FadeTime = 0.55f;
		HelixRadius = 60.f; HelixSpeed = 30.f;
		break;
	case EWeaponType::SMG:
		CoreRadius = 0.28f; GlowRadius = 0.70f; TrailRadius = 0.14f;
		CoronaRadius = 1.3f; HeadScale = 1.4f; BeamLength = 2200.f;
		TravelSpeed = 95000.f; LightIntensity = 150000.f; FadeTime = 0.35f;
		HelixRadius = 40.f; HelixSpeed = 40.f;
		break;
	case EWeaponType::Pistol:
		CoreRadius = 0.35f; GlowRadius = 0.85f; TrailRadius = 0.16f;
		CoronaRadius = 1.6f; HeadScale = 1.7f; BeamLength = 2800.f;
		TravelSpeed = 75000.f; LightIntensity = 180000.f; FadeTime = 0.45f;
		HelixRadius = 50.f; HelixSpeed = 32.f;
		break;
	case EWeaponType::Shotgun:
		CoreRadius = 0.50f; GlowRadius = 1.2f; TrailRadius = 0.25f;
		CoronaRadius = 2.4f; HeadScale = 2.2f; BeamLength = 1600.f;
		TravelSpeed = 55000.f; LightIntensity = 280000.f; FadeTime = 0.30f;
		HelixRadius = 80.f; HelixSpeed = 22.f;
		break;
	case EWeaponType::Sniper:
		CoreRadius = 0.45f; GlowRadius = 1.1f; TrailRadius = 0.28f;
		CoronaRadius = 2.2f; HeadScale = 2.5f; BeamLength = 5500.f;
		TravelSpeed = 120000.f; LightIntensity = 350000.f; FadeTime = 0.80f;
		HelixRadius = 70.f; HelixSpeed = 25.f;
		break;
	default:
		CoreRadius = 0.38f; GlowRadius = 0.95f; TrailRadius = 0.18f;
		CoronaRadius = 1.8f; HeadScale = 1.8f; BeamLength = 3000.f;
		TravelSpeed = 70000.f; LightIntensity = 190000.f; FadeTime = 0.50f;
		HelixRadius = 55.f; HelixSpeed = 28.f;
		break;
	}

	FRotator BeamRot = Direction.Rotation();
	BeamRot.Pitch += 90.f;
	SetActorRotation(BeamRot);

	float HitBoost = bIsHit ? 5.f : 1.f;
	// Core: blazing white-hot center, extreme emissive for massive bloom
	CoreColor = FLinearColor(
		WeaponColor.R * 80.f * HitBoost + 30.f,
		WeaponColor.G * 80.f * HitBoost + 30.f,
		WeaponColor.B * 80.f * HitBoost + 30.f);
	// Glow: heavily saturated weapon color halo
	GlowColor = FLinearColor(
		WeaponColor.R * 45.f,
		WeaponColor.G * 45.f,
		WeaponColor.B * 45.f);

	UMaterialInterface* BaseMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (!BaseMat) return;

	// Core — blazing white-hot center
	CoreMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	CoreMat->SetVectorParameterValue(TEXT("BaseColor"), CoreColor);
	CoreMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor);
	BeamCore->SetMaterial(0, CoreMat);

	// Glow — saturated weapon color halo
	GlowMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	GlowMat->SetVectorParameterValue(TEXT("BaseColor"), GlowColor);
	GlowMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor);
	BeamGlow->SetMaterial(0, GlowMat);

	// Trail — fading wake with subtle glow
	TrailMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	TrailMat->SetVectorParameterValue(TEXT("BaseColor"), GlowColor * 0.4f);
	TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 0.4f);
	BeamTrail->SetMaterial(0, TrailMat);

	// Corona — large soft outer halo (visible with bloom)
	CoronaMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	FLinearColor CoronaColor = GlowColor * 0.25f;
	CoronaMat->SetVectorParameterValue(TEXT("BaseColor"), CoronaColor);
	CoronaMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoronaColor);
	Corona->SetMaterial(0, CoronaMat);

	// Head — blazing energy ball leading the bolt
	HeadMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	FLinearColor HeadColor = CoreColor * 5.f;
	HeadMat->SetVectorParameterValue(TEXT("BaseColor"), HeadColor);
	HeadMat->SetVectorParameterValue(TEXT("EmissiveColor"), HeadColor);
	HeadMesh->SetMaterial(0, HeadMat);
	HeadMesh->SetRelativeScale3D(FVector(HeadScale, HeadScale * 0.8f, HeadScale * 0.8f));

	// Trailing sparks — random scatter velocities, bigger and brighter
	SparkOffsets.SetNum(NUM_SPARKS);
	SparkVelocities.SetNum(NUM_SPARKS);
	for (int32 i = 0; i < NUM_SPARKS; i++)
	{
		SparkOffsets[i] = FVector::ZeroVector;
		SparkVelocities[i] = FVector(
			FMath::RandRange(-250.f, 250.f),
			FMath::RandRange(-250.f, 250.f),
			FMath::RandRange(-120.f, 120.f));
		float S = FMath::RandRange(0.06f, 0.14f);
		SparkMeshes[i]->SetRelativeScale3D(FVector(S, S * 0.4f, S * 3.f));
		UMaterialInstanceDynamic* SM = UMaterialInstanceDynamic::Create(BaseMat, this);
		FLinearColor SparkCol = CoreColor * FMath::RandRange(0.5f, 1.0f);
		SM->SetVectorParameterValue(TEXT("BaseColor"), SparkCol);
		SM->SetVectorParameterValue(TEXT("EmissiveColor"), SparkCol);
		SparkMeshes[i]->SetMaterial(0, SM);
	}

	// Helix orbiters — weapon-colored plasma spheres that spiral around the bolt
	for (int32 i = 0; i < NUM_HELIX; i++)
	{
		float OrbS = CoreRadius * 0.5f;
		HelixOrbs[i]->SetRelativeScale3D(FVector(OrbS, OrbS, OrbS * 1.5f));
		UMaterialInstanceDynamic* HM = UMaterialInstanceDynamic::Create(BaseMat, this);
		FLinearColor HColor = GlowColor * 1.5f;
		HM->SetVectorParameterValue(TEXT("BaseColor"), HColor);
		HM->SetVectorParameterValue(TEXT("EmissiveColor"), HColor);
		HelixOrbs[i]->SetMaterial(0, HM);
	}

	// Head shockwave ring — flat, expanding cylinder at bolt front
	float RingS = HeadScale * 0.3f;
	HeadRing->SetRelativeScale3D(FVector(RingS, RingS, 0.01f));
	UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	FLinearColor RingColor = GlowColor * 0.8f;
	RingMat->SetVectorParameterValue(TEXT("BaseColor"), RingColor);
	RingMat->SetVectorParameterValue(TEXT("EmissiveColor"), RingColor);
	HeadRing->SetMaterial(0, RingMat);

	// Lights — strong illumination that lights up the environment
	HeadLight->SetLightColor(WeaponColor);
	HeadLight->SetIntensity(LightIntensity);
	HeadLight->SetAttenuationRadius(3000.f + LightIntensity * 0.015f);

	TailLight->SetLightColor(WeaponColor);
	TailLight->SetIntensity(LightIntensity * 0.5f);
	TailLight->SetAttenuationRadius(2000.f);

	SetActorLocation(StartPos);

	BeamCore->SetRelativeScale3D(FVector(CoreRadius, CoreRadius, 0.01f));
	BeamGlow->SetRelativeScale3D(FVector(GlowRadius, GlowRadius, 0.01f));
	BeamTrail->SetRelativeScale3D(FVector(TrailRadius, TrailRadius, 0.01f));
	Corona->SetRelativeScale3D(FVector(CoronaRadius, CoronaRadius, 0.01f));
}

// Tick, UpdateTraveling, UpdateFading are in ExoTracerTick.cpp
