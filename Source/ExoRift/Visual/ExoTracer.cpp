#include "Visual/ExoTracer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoTracer::AExoTracer()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 4.f;

	// Cache meshes — cylinders give a proper beam look
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFinder.Succeeded()) CylinderMesh = CylFinder.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded()) SphereMesh = SphereFinder.Object;

	// Core beam — thin bright center cylinder
	BeamCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamCore"));
	RootComponent = BeamCore;
	if (CylinderMesh) BeamCore->SetStaticMesh(CylinderMesh);
	BeamCore->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamCore->CastShadow = false;
	BeamCore->SetGenerateOverlapEvents(false);

	// Outer glow — wider softer cylinder
	BeamGlow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamGlow"));
	BeamGlow->SetupAttachment(BeamCore);
	if (CylinderMesh) BeamGlow->SetStaticMesh(CylinderMesh);
	BeamGlow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamGlow->CastShadow = false;
	BeamGlow->SetGenerateOverlapEvents(false);

	// Trail — faint lingering beam that follows behind
	BeamTrail = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamTrail"));
	BeamTrail->SetupAttachment(BeamCore);
	if (CylinderMesh) BeamTrail->SetStaticMesh(CylinderMesh);
	BeamTrail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamTrail->CastShadow = false;
	BeamTrail->SetGenerateOverlapEvents(false);

	// Head sphere — bright dot at leading edge
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(BeamCore);
	if (SphereMesh) HeadMesh->SetStaticMesh(SphereMesh);
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMesh->CastShadow = false;
	HeadMesh->SetGenerateOverlapEvents(false);

	// Head light — illuminates surroundings as bolt travels
	HeadLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("HeadLight"));
	HeadLight->SetupAttachment(BeamCore);
	HeadLight->SetIntensity(15000.f);
	HeadLight->SetAttenuationRadius(600.f);
	HeadLight->CastShadows = false;
}

void AExoTracer::InitTracer(const FVector& Start, const FVector& End, bool bIsHit)
{
	StartPos = Start;
	EndPos = End;
	Direction = (End - Start).GetSafeNormal();
	TotalDistance = FVector::Distance(Start, End);

	if (TotalDistance < 1.f) { Destroy(); return; }

	// Cylinder default axis is Z — we need to rotate to face along direction
	// Build rotation: cylinder Z maps to our travel direction
	FRotator BeamRot = Direction.Rotation();
	BeamRot.Pitch += 90.f; // Cylinder points up by default, rotate to forward
	SetActorRotation(BeamRot);

	// Sci-fi energy colors: bright cyan (normal) vs hot orange (hit confirmed)
	CoreColor = bIsHit
		? FLinearColor(15.f, 4.f, 0.5f, 1.f)
		: FLinearColor(3.f, 10.f, 20.f, 1.f);
	GlowColor = bIsHit
		? FLinearColor(5.f, 1.5f, 0.3f, 1.f)
		: FLinearColor(0.8f, 4.f, 10.f, 1.f);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!MatFinder.Succeeded()) return;

	// Core material — bright hot center
	CoreMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
	CoreMat->SetVectorParameterValue(TEXT("BaseColor"), CoreColor);
	CoreMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor);
	BeamCore->SetMaterial(0, CoreMat);

	// Glow material — wider, softer
	GlowMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
	GlowMat->SetVectorParameterValue(TEXT("BaseColor"), GlowColor * 0.4f);
	GlowMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 0.4f);
	BeamGlow->SetMaterial(0, GlowMat);

	// Trail material — very dim, lingers
	TrailMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
	TrailMat->SetVectorParameterValue(TEXT("BaseColor"), GlowColor * 0.15f);
	TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 0.15f);
	BeamTrail->SetMaterial(0, TrailMat);

	// Head — bright sphere with punchier emissive
	HeadMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
	HeadMat->SetVectorParameterValue(TEXT("BaseColor"), CoreColor * 2.f);
	HeadMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor * 2.f);
	HeadMesh->SetMaterial(0, HeadMat);
	HeadMesh->SetRelativeScale3D(FVector(0.1f));

	// Light color
	FLinearColor LightColor = bIsHit
		? FLinearColor(1.f, 0.4f, 0.1f)
		: FLinearColor(0.3f, 0.7f, 1.f);
	HeadLight->SetLightColor(LightColor);

	SetActorLocation(StartPos);

	// Start small — grows as it travels
	BeamCore->SetRelativeScale3D(FVector(0.005f, 0.005f, 0.01f));
	BeamGlow->SetRelativeScale3D(FVector(0.015f, 0.015f, 0.01f));
	BeamTrail->SetRelativeScale3D(FVector(0.008f, 0.008f, 0.01f));
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

	// Beam visible length: grows up to BeamLength, then stays constant
	float VisibleLen = FMath::Min(TraveledDist, BeamLength);

	// Head and tail positions
	float TailDist = FMath::Max(TraveledDist - BeamLength, 0.f);
	FVector HeadPos = StartPos + Direction * TraveledDist;
	FVector TailPos = StartPos + Direction * TailDist;
	FVector BeamCenter = (HeadPos + TailPos) * 0.5f;

	SetActorLocation(BeamCenter);

	// Cylinder default height is 100 units, scale Z for length
	float LenScale = VisibleLen / 100.f;
	float Time = GetWorld()->GetTimeSeconds();
	float Flicker = 1.f + 0.08f * FMath::Sin(Time * 60.f);

	// Core: thin bright cylinder
	BeamCore->SetRelativeScale3D(FVector(0.006f * Flicker, 0.006f * Flicker, LenScale));

	// Glow: wider, pulsing
	float GlowPulse = 1.f + 0.12f * FMath::Sin(Time * 40.f);
	BeamGlow->SetRelativeScale3D(FVector(0.018f * GlowPulse, 0.018f * GlowPulse, LenScale * 0.9f));

	// Trail: thinner, extends further back
	float TrailLen = FMath::Min(TraveledDist, BeamLength * 1.5f);
	float TrailLenScale = TrailLen / 100.f;
	BeamTrail->SetRelativeScale3D(FVector(0.003f, 0.003f, TrailLenScale));

	// Head mesh at front of beam (in local Z space since cylinder is Z-aligned)
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, VisibleLen * 0.5f));
	HeadLight->SetRelativeLocation(FVector(0.f, 0.f, VisibleLen * 0.5f));

	// Head: pulsing glow
	float HeadPulse = 0.1f + 0.03f * FMath::Sin(Time * 50.f);
	HeadMesh->SetRelativeScale3D(FVector(HeadPulse));
	HeadLight->SetIntensity(15000.f + 3000.f * FMath::Sin(Time * 35.f));
}

void AExoTracer::UpdateFading(float DeltaTime)
{
	FadeAge += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(FadeAge / FadeTime, 0.f, 1.f);
	float AlphaSq = Alpha * Alpha;

	// Light fades
	HeadLight->SetIntensity(15000.f * AlphaSq);

	// Head shrinks
	HeadMesh->SetRelativeScale3D(FVector(0.1f * Alpha));

	// Core beam: shrink cross-section, maintain length
	FVector CoreScale = BeamCore->GetRelativeScale3D();
	BeamCore->SetRelativeScale3D(FVector(0.006f * Alpha, 0.006f * Alpha, CoreScale.Z));

	// Glow: fades faster
	float GlowAlpha = AlphaSq;
	BeamGlow->SetRelativeScale3D(FVector(0.018f * GlowAlpha, 0.018f * GlowAlpha, CoreScale.Z * 0.9f));

	// Trail: lingers longest — fades to nothing
	float TrailAlpha = FMath::Clamp(Alpha * 1.5f, 0.f, 1.f);
	FVector TrailScale = BeamTrail->GetRelativeScale3D();
	BeamTrail->SetRelativeScale3D(FVector(0.003f * TrailAlpha, 0.003f * TrailAlpha, TrailScale.Z));

	// Update trail emissive to fade
	if (TrailMat)
	{
		TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 0.15f * TrailAlpha);
	}

	if (FadeAge >= FadeTime) Destroy();
}
