// ExoTracer.cpp — Dramatic sci-fi energy bolt with corona, sparks, and lighting
#include "Visual/ExoTracer.h"
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

	// Weapon-specific beam parameters — much more dramatic than before
	switch (WeaponType)
	{
	case EWeaponType::Rifle:
		CoreRadius = 0.030f; GlowRadius = 0.090f; TrailRadius = 0.015f;
		CoronaRadius = 0.18f; HeadScale = 0.30f; BeamLength = 1800.f;
		TravelSpeed = 140000.f; LightIntensity = 55000.f; FadeTime = 0.18f;
		break;
	case EWeaponType::SMG:
		CoreRadius = 0.022f; GlowRadius = 0.065f; TrailRadius = 0.010f;
		CoronaRadius = 0.13f; HeadScale = 0.22f; BeamLength = 1200.f;
		TravelSpeed = 160000.f; LightIntensity = 40000.f; FadeTime = 0.12f;
		break;
	case EWeaponType::Pistol:
		CoreRadius = 0.026f; GlowRadius = 0.075f; TrailRadius = 0.012f;
		CoronaRadius = 0.15f; HeadScale = 0.26f; BeamLength = 1400.f;
		TravelSpeed = 130000.f; LightIntensity = 48000.f; FadeTime = 0.15f;
		break;
	case EWeaponType::Shotgun:
		CoreRadius = 0.040f; GlowRadius = 0.110f; TrailRadius = 0.020f;
		CoronaRadius = 0.22f; HeadScale = 0.35f; BeamLength = 800.f;
		TravelSpeed = 100000.f; LightIntensity = 65000.f; FadeTime = 0.10f;
		break;
	case EWeaponType::Sniper:
		CoreRadius = 0.035f; GlowRadius = 0.100f; TrailRadius = 0.025f;
		CoronaRadius = 0.20f; HeadScale = 0.40f; BeamLength = 3200.f;
		TravelSpeed = 200000.f; LightIntensity = 80000.f; FadeTime = 0.30f;
		break;
	default:
		CoreRadius = 0.030f; GlowRadius = 0.090f; TrailRadius = 0.015f;
		CoronaRadius = 0.18f; HeadScale = 0.30f; BeamLength = 1600.f;
		TravelSpeed = 120000.f; LightIntensity = 50000.f; FadeTime = 0.18f;
		break;
	}

	FRotator BeamRot = Direction.Rotation();
	BeamRot.Pitch += 90.f;
	SetActorRotation(BeamRot);

	float HitBoost = bIsHit ? 3.f : 1.f;
	// Core: white-hot center with massive emissive
	CoreColor = FLinearColor(
		WeaponColor.R * 15.f * HitBoost + 3.f,
		WeaponColor.G * 15.f * HitBoost + 3.f,
		WeaponColor.B * 15.f * HitBoost + 3.f);
	// Glow: saturated weapon color
	GlowColor = FLinearColor(
		WeaponColor.R * 6.f,
		WeaponColor.G * 6.f,
		WeaponColor.B * 6.f);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!MatFinder.Succeeded()) return;
	UMaterialInterface* BaseMat = MatFinder.Object;

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

	// Corona — large soft outer halo (dim but wide)
	CoronaMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	FLinearColor CoronaColor = GlowColor * 0.15f;
	CoronaMat->SetVectorParameterValue(TEXT("BaseColor"), CoronaColor);
	CoronaMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoronaColor);
	Corona->SetMaterial(0, CoronaMat);

	// Head — blazing energy ball leading the bolt
	HeadMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	FLinearColor HeadColor = CoreColor * 2.f;
	HeadMat->SetVectorParameterValue(TEXT("BaseColor"), HeadColor);
	HeadMat->SetVectorParameterValue(TEXT("EmissiveColor"), HeadColor);
	HeadMesh->SetMaterial(0, HeadMat);
	HeadMesh->SetRelativeScale3D(FVector(HeadScale, HeadScale * 0.7f, HeadScale * 0.7f));

	// Trailing sparks — random scatter velocities
	SparkOffsets.SetNum(NUM_SPARKS);
	SparkVelocities.SetNum(NUM_SPARKS);
	for (int32 i = 0; i < NUM_SPARKS; i++)
	{
		SparkOffsets[i] = FVector::ZeroVector;
		SparkVelocities[i] = FVector(
			FMath::RandRange(-80.f, 80.f),
			FMath::RandRange(-80.f, 80.f),
			FMath::RandRange(-40.f, 40.f));
		float S = FMath::RandRange(0.015f, 0.04f);
		SparkMeshes[i]->SetRelativeScale3D(FVector(S, S * 0.4f, S * 2.f));
		UMaterialInstanceDynamic* SM = UMaterialInstanceDynamic::Create(BaseMat, this);
		FLinearColor SparkCol = CoreColor * FMath::RandRange(0.4f, 0.8f);
		SM->SetVectorParameterValue(TEXT("BaseColor"), SparkCol);
		SM->SetVectorParameterValue(TEXT("EmissiveColor"), SparkCol);
		SparkMeshes[i]->SetMaterial(0, SM);
	}

	// Lights — strong illumination
	HeadLight->SetLightColor(WeaponColor);
	HeadLight->SetIntensity(LightIntensity);
	HeadLight->SetAttenuationRadius(1200.f + LightIntensity * 0.015f);

	TailLight->SetLightColor(WeaponColor);
	TailLight->SetIntensity(LightIntensity * 0.35f);
	TailLight->SetAttenuationRadius(800.f);

	SetActorLocation(StartPos);

	BeamCore->SetRelativeScale3D(FVector(CoreRadius, CoreRadius, 0.01f));
	BeamGlow->SetRelativeScale3D(FVector(GlowRadius, GlowRadius, 0.01f));
	BeamTrail->SetRelativeScale3D(FVector(TrailRadius, TrailRadius, 0.01f));
	Corona->SetRelativeScale3D(FVector(CoronaRadius, CoronaRadius, 0.01f));
}

void AExoTracer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bReachedEnd)
		UpdateTraveling(DeltaTime);
	else
		UpdateFading(DeltaTime);
}

void AExoTracer::UpdateTraveling(float DeltaTime)
{
	TraveledDist += TravelSpeed * DeltaTime;
	if (TraveledDist >= TotalDistance)
	{
		TraveledDist = TotalDistance;
		bReachedEnd = true;
		FadeAge = 0.f;
	}

	float VisibleLen = FMath::Min(TraveledDist, BeamLength);
	float TailDist = FMath::Max(TraveledDist - BeamLength, 0.f);
	FVector HeadPos = StartPos + Direction * TraveledDist;
	FVector TailPos = StartPos + Direction * TailDist;
	SetActorLocation((HeadPos + TailPos) * 0.5f);

	float LenScale = VisibleLen / 100.f;
	float Time = GetWorld()->GetTimeSeconds();

	// Multi-frequency flicker for organic energy bolt feel
	float F1 = FMath::Sin(Time * 95.f) * 0.12f;
	float F2 = FMath::Sin(Time * 61.f) * 0.08f;
	float F3 = FMath::Sin(Time * 137.f) * 0.05f; // High freq shimmer
	float CoreFlicker = 1.f + F1 + F2 + F3;

	BeamCore->SetRelativeScale3D(FVector(
		CoreRadius * CoreFlicker, CoreRadius * CoreFlicker, LenScale));

	// Glow: slower organic pulse with breathing
	float GlowPulse = 1.f + 0.18f * FMath::Sin(Time * 28.f)
		+ 0.08f * FMath::Cos(Time * 47.f);
	BeamGlow->SetRelativeScale3D(FVector(
		GlowRadius * GlowPulse, GlowRadius * GlowPulse, LenScale * 0.95f));

	// Corona: large soft outer halo with slow drift
	float CoronaPulse = 1.f + 0.25f * FMath::Sin(Time * 18.f);
	Corona->SetRelativeScale3D(FVector(
		CoronaRadius * CoronaPulse, CoronaRadius * CoronaPulse, LenScale * 0.85f));

	// Trail: extends further back
	float TrailLen = FMath::Min(TraveledDist, BeamLength * 2.2f);
	BeamTrail->SetRelativeScale3D(FVector(TrailRadius, TrailRadius, TrailLen / 100.f));

	// Head: elongated energy ball at front, pulsing
	float HeadHalf = VisibleLen * 0.5f;
	float HeadPulse = HeadScale * (1.f + 0.25f * FMath::Sin(Time * 55.f));
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, HeadHalf));
	HeadMesh->SetRelativeScale3D(FVector(
		HeadPulse, HeadPulse * 0.65f, HeadPulse * 1.4f));

	// Dynamic emissive pulses on core
	float EmissivePulse = 1.f + 0.3f * FMath::Sin(Time * 70.f);
	if (CoreMat)
		CoreMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor * EmissivePulse);
	if (GlowMat)
	{
		float GP = 1.f + 0.2f * FMath::Sin(Time * 40.f);
		GlowMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * GP);
	}

	// Lights: strong with flicker
	float LightFlicker = 1.f + 0.2f * FMath::Sin(Time * 35.f)
		+ 0.1f * FMath::Sin(Time * 73.f);
	HeadLight->SetRelativeLocation(FVector(0.f, 0.f, HeadHalf));
	HeadLight->SetIntensity(LightIntensity * LightFlicker);
	TailLight->SetRelativeLocation(FVector(0.f, 0.f, -VisibleLen * 0.4f));

	// Trailing sparks: scatter outward from the beam wake
	for (int32 i = 0; i < SparkMeshes.Num(); i++)
	{
		SparkOffsets[i] += SparkVelocities[i] * DeltaTime;
		// Position along the trailing half of the beam
		float SparkZ = -VisibleLen * (0.2f + 0.15f * i);
		SparkMeshes[i]->SetRelativeLocation(
			FVector(SparkOffsets[i].X, SparkOffsets[i].Y, SparkZ));
		// Tumble rotation
		float R = Time * (200.f + i * 80.f);
		SparkMeshes[i]->SetRelativeRotation(FRotator(R, R * 0.7f, R * 0.5f));
	}
}

void AExoTracer::UpdateFading(float DeltaTime)
{
	FadeAge += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(FadeAge / FadeTime, 0.f, 1.f);
	float AlphaSq = Alpha * Alpha;

	HeadLight->SetIntensity(LightIntensity * AlphaSq);
	TailLight->SetIntensity(LightIntensity * 0.35f * Alpha);

	HeadMesh->SetRelativeScale3D(FVector(HeadScale * Alpha));

	FVector CoreScale = BeamCore->GetRelativeScale3D();
	float Z = CoreScale.Z;
	BeamCore->SetRelativeScale3D(FVector(CoreRadius * Alpha, CoreRadius * Alpha, Z));
	BeamGlow->SetRelativeScale3D(FVector(
		GlowRadius * AlphaSq, GlowRadius * AlphaSq, Z * 0.95f));
	Corona->SetRelativeScale3D(FVector(
		CoronaRadius * AlphaSq * 0.5f, CoronaRadius * AlphaSq * 0.5f, Z * 0.8f));

	// Trail lingers longest
	float TrailAlpha = FMath::Clamp(Alpha * 1.8f, 0.f, 1.f);
	FVector TS = BeamTrail->GetRelativeScale3D();
	BeamTrail->SetRelativeScale3D(FVector(
		TrailRadius * TrailAlpha, TrailRadius * TrailAlpha, TS.Z));

	if (TrailMat)
		TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 0.4f * TrailAlpha);
	if (CoronaMat)
		CoronaMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			GlowColor * 0.15f * AlphaSq);

	// Sparks scatter outward and fade
	for (int32 i = 0; i < SparkMeshes.Num(); i++)
	{
		SparkOffsets[i] += SparkVelocities[i] * DeltaTime * 3.f;
		float S = FMath::Max(0.01f, 0.03f * Alpha);
		SparkMeshes[i]->SetRelativeScale3D(FVector(S, S * 0.4f, S * 2.f));
	}

	if (FadeAge >= FadeTime) Destroy();
}
