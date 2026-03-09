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

	// Core beam — bright center
	BeamCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamCore"));
	RootComponent = BeamCore;
	if (CylinderMesh) BeamCore->SetStaticMesh(CylinderMesh);
	BeamCore->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamCore->CastShadow = false;
	BeamCore->SetGenerateOverlapEvents(false);

	// Outer glow
	BeamGlow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamGlow"));
	BeamGlow->SetupAttachment(BeamCore);
	if (CylinderMesh) BeamGlow->SetStaticMesh(CylinderMesh);
	BeamGlow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamGlow->CastShadow = false;
	BeamGlow->SetGenerateOverlapEvents(false);

	// Trail — lingering wake
	BeamTrail = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamTrail"));
	BeamTrail->SetupAttachment(BeamCore);
	if (CylinderMesh) BeamTrail->SetStaticMesh(CylinderMesh);
	BeamTrail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamTrail->CastShadow = false;
	BeamTrail->SetGenerateOverlapEvents(false);

	// Head sphere — bright leading dot
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(BeamCore);
	if (SphereMesh) HeadMesh->SetStaticMesh(SphereMesh);
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMesh->CastShadow = false;
	HeadMesh->SetGenerateOverlapEvents(false);

	// Head light — dynamic illumination at bolt front
	HeadLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("HeadLight"));
	HeadLight->SetupAttachment(BeamCore);
	HeadLight->SetIntensity(20000.f);
	HeadLight->SetAttenuationRadius(800.f);
	HeadLight->CastShadows = false;

	// Tail light — subtle glow at beam wake
	TailLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TailLight"));
	TailLight->SetupAttachment(BeamCore);
	TailLight->SetIntensity(5000.f);
	TailLight->SetAttenuationRadius(400.f);
	TailLight->CastShadows = false;
}

void AExoTracer::InitTracer(const FVector& Start, const FVector& End, bool bIsHit,
	const FLinearColor& WeaponColor, EWeaponType WeaponType)
{
	StartPos = Start;
	EndPos = End;
	Direction = (End - Start).GetSafeNormal();
	TotalDistance = FVector::Distance(Start, End);

	if (TotalDistance < 1.f) { Destroy(); return; }

	// Weapon-specific beam parameters
	switch (WeaponType)
	{
	case EWeaponType::Rifle:
		CoreRadius = 0.012f; GlowRadius = 0.035f; TrailRadius = 0.006f;
		HeadScale = 0.15f; BeamLength = 1400.f; TravelSpeed = 140000.f;
		LightIntensity = 22000.f; FadeTime = 0.15f;
		break;
	case EWeaponType::SMG:
		CoreRadius = 0.008f; GlowRadius = 0.022f; TrailRadius = 0.004f;
		HeadScale = 0.10f; BeamLength = 900.f; TravelSpeed = 160000.f;
		LightIntensity = 16000.f; FadeTime = 0.10f;
		break;
	case EWeaponType::Pistol:
		CoreRadius = 0.010f; GlowRadius = 0.028f; TrailRadius = 0.005f;
		HeadScale = 0.12f; BeamLength = 1000.f; TravelSpeed = 130000.f;
		LightIntensity = 18000.f; FadeTime = 0.12f;
		break;
	case EWeaponType::Shotgun:
		CoreRadius = 0.016f; GlowRadius = 0.045f; TrailRadius = 0.008f;
		HeadScale = 0.18f; BeamLength = 600.f; TravelSpeed = 100000.f;
		LightIntensity = 25000.f; FadeTime = 0.08f;
		break;
	case EWeaponType::Sniper:
		CoreRadius = 0.014f; GlowRadius = 0.040f; TrailRadius = 0.010f;
		HeadScale = 0.20f; BeamLength = 2500.f; TravelSpeed = 200000.f;
		LightIntensity = 30000.f; FadeTime = 0.25f;
		break;
	default:
		CoreRadius = 0.012f; GlowRadius = 0.035f; TrailRadius = 0.006f;
		HeadScale = 0.15f; BeamLength = 1200.f; TravelSpeed = 120000.f;
		LightIntensity = 20000.f; FadeTime = 0.15f;
		break;
	}

	// Orient beam along travel direction
	FRotator BeamRot = Direction.Rotation();
	BeamRot.Pitch += 90.f;
	SetActorRotation(BeamRot);

	// Hit confirmation uses brighter, hotter version of weapon color
	float HitBoost = bIsHit ? 2.5f : 1.f;
	CoreColor = FLinearColor(
		WeaponColor.R * 5.f * HitBoost,
		WeaponColor.G * 5.f * HitBoost,
		WeaponColor.B * 5.f * HitBoost);
	GlowColor = FLinearColor(
		WeaponColor.R * 2.f,
		WeaponColor.G * 2.f,
		WeaponColor.B * 2.f);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!MatFinder.Succeeded()) return;

	// Core — white-hot center
	CoreMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
	CoreMat->SetVectorParameterValue(TEXT("BaseColor"), CoreColor);
	CoreMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor);
	BeamCore->SetMaterial(0, CoreMat);

	// Glow — colored halo
	GlowMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
	GlowMat->SetVectorParameterValue(TEXT("BaseColor"), GlowColor * 0.5f);
	GlowMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 0.5f);
	BeamGlow->SetMaterial(0, GlowMat);

	// Trail — dim lingering wake
	TrailMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
	TrailMat->SetVectorParameterValue(TEXT("BaseColor"), GlowColor * 0.2f);
	TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 0.2f);
	BeamTrail->SetMaterial(0, TrailMat);

	// Head — bright leading sphere
	HeadMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
	HeadMat->SetVectorParameterValue(TEXT("BaseColor"), CoreColor * 1.5f);
	HeadMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor * 1.5f);
	HeadMesh->SetMaterial(0, HeadMat);
	HeadMesh->SetRelativeScale3D(FVector(HeadScale));

	// Lights use the weapon's base color
	HeadLight->SetLightColor(WeaponColor);
	HeadLight->SetIntensity(LightIntensity);
	HeadLight->SetAttenuationRadius(600.f + LightIntensity * 0.02f);

	TailLight->SetLightColor(WeaponColor);
	TailLight->SetIntensity(LightIntensity * 0.3f);

	SetActorLocation(StartPos);

	// Start at minimal size
	BeamCore->SetRelativeScale3D(FVector(CoreRadius, CoreRadius, 0.01f));
	BeamGlow->SetRelativeScale3D(FVector(GlowRadius, GlowRadius, 0.01f));
	BeamTrail->SetRelativeScale3D(FVector(TrailRadius, TrailRadius, 0.01f));
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
	FVector BeamCenter = (HeadPos + TailPos) * 0.5f;

	SetActorLocation(BeamCenter);

	float LenScale = VisibleLen / 100.f;
	float Time = GetWorld()->GetTimeSeconds();

	// Core: high-frequency flicker for energy feel
	float Flicker = 1.f + 0.1f * FMath::Sin(Time * 80.f) + 0.06f * FMath::Sin(Time * 53.f);
	BeamCore->SetRelativeScale3D(FVector(CoreRadius * Flicker, CoreRadius * Flicker, LenScale));

	// Glow: slower organic pulse
	float GlowPulse = 1.f + 0.15f * FMath::Sin(Time * 35.f);
	BeamGlow->SetRelativeScale3D(FVector(
		GlowRadius * GlowPulse, GlowRadius * GlowPulse, LenScale * 0.92f));

	// Trail: extends further back for dramatic wake
	float TrailLen = FMath::Min(TraveledDist, BeamLength * 1.8f);
	float TrailLenScale = TrailLen / 100.f;
	BeamTrail->SetRelativeScale3D(FVector(TrailRadius, TrailRadius, TrailLenScale));

	// Head at front of beam
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, VisibleLen * 0.5f));
	HeadLight->SetRelativeLocation(FVector(0.f, 0.f, VisibleLen * 0.5f));
	TailLight->SetRelativeLocation(FVector(0.f, 0.f, -VisibleLen * 0.4f));

	// Pulsing head
	float HeadPulse = HeadScale * (1.f + 0.2f * FMath::Sin(Time * 45.f));
	HeadMesh->SetRelativeScale3D(FVector(HeadPulse));
	HeadLight->SetIntensity(LightIntensity * (1.f + 0.15f * FMath::Sin(Time * 30.f)));
}

void AExoTracer::UpdateFading(float DeltaTime)
{
	FadeAge += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(FadeAge / FadeTime, 0.f, 1.f);
	float AlphaSq = Alpha * Alpha;

	HeadLight->SetIntensity(LightIntensity * AlphaSq);
	TailLight->SetIntensity(LightIntensity * 0.3f * Alpha);

	HeadMesh->SetRelativeScale3D(FVector(HeadScale * Alpha));

	FVector CoreScale = BeamCore->GetRelativeScale3D();
	BeamCore->SetRelativeScale3D(FVector(CoreRadius * Alpha, CoreRadius * Alpha, CoreScale.Z));

	float GlowAlpha = AlphaSq;
	BeamGlow->SetRelativeScale3D(FVector(
		GlowRadius * GlowAlpha, GlowRadius * GlowAlpha, CoreScale.Z * 0.92f));

	// Trail lingers longest
	float TrailAlpha = FMath::Clamp(Alpha * 1.5f, 0.f, 1.f);
	FVector TrailScale = BeamTrail->GetRelativeScale3D();
	BeamTrail->SetRelativeScale3D(FVector(
		TrailRadius * TrailAlpha, TrailRadius * TrailAlpha, TrailScale.Z));

	if (TrailMat)
	{
		TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 0.2f * TrailAlpha);
	}

	if (FadeAge >= FadeTime) Destroy();
}
