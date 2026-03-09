#include "Visual/ExoTracer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoTracer::AExoTracer()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 4.f;

	// Core beam — thin bright center line
	BeamCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamCore"));
	RootComponent = BeamCore;
	BeamCore->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamCore->CastShadow = false;
	BeamCore->SetGenerateOverlapEvents(false);

	// Outer glow — wider softer beam
	BeamGlow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamGlow"));
	BeamGlow->SetupAttachment(BeamCore);
	BeamGlow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamGlow->CastShadow = false;
	BeamGlow->SetGenerateOverlapEvents(false);

	// Head sphere — bright dot at the leading edge
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(BeamCore);
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMesh->CastShadow = false;
	HeadMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFinder.Succeeded())
	{
		BeamCore->SetStaticMesh(CubeFinder.Object);
		BeamGlow->SetStaticMesh(CubeFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		HeadMesh->SetStaticMesh(SphereFinder.Object);
	}

	// Head light — illuminates the path
	HeadLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("HeadLight"));
	HeadLight->SetupAttachment(BeamCore);
	HeadLight->SetIntensity(8000.f);
	HeadLight->SetAttenuationRadius(400.f);
	HeadLight->CastShadows = false;
}

void AExoTracer::InitTracer(const FVector& Start, const FVector& End, bool bIsHit)
{
	StartPos = Start;
	EndPos = End;
	Direction = (End - Start).GetSafeNormal();
	TotalDistance = FVector::Distance(Start, End);

	if (TotalDistance < 1.f) { Destroy(); return; }

	SetActorRotation(Direction.Rotation());

	// Sci-fi energy colors — bright cyan vs hot orange
	CoreColor = bIsHit
		? FLinearColor(12.f, 3.f, 0.5f, 1.f)
		: FLinearColor(2.f, 8.f, 15.f, 1.f);
	GlowColor = bIsHit
		? FLinearColor(4.f, 1.f, 0.2f, 1.f)
		: FLinearColor(0.5f, 3.f, 8.f, 1.f);

	// Core material
	if (BeamCore->GetMaterial(0))
	{
		CoreMat = BeamCore->CreateDynamicMaterialInstance(0);
		if (CoreMat)
		{
			CoreMat->SetVectorParameterValue(TEXT("BaseColor"), CoreColor);
			CoreMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor);
		}
	}

	// Glow material — dimmer, wider
	if (BeamGlow->GetMaterial(0))
	{
		GlowMat = BeamGlow->CreateDynamicMaterialInstance(0);
		if (GlowMat)
		{
			GlowMat->SetVectorParameterValue(TEXT("BaseColor"), GlowColor * 0.5f);
			GlowMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 0.5f);
		}
	}

	// Head mesh — bright dot
	if (HeadMesh->GetMaterial(0))
	{
		UMaterialInstanceDynamic* HeadMat = HeadMesh->CreateDynamicMaterialInstance(0);
		if (HeadMat)
		{
			HeadMat->SetVectorParameterValue(TEXT("BaseColor"), CoreColor * 1.5f);
			HeadMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor * 1.5f);
		}
	}
	HeadMesh->SetRelativeScale3D(FVector(0.06f));

	// Light color
	FLinearColor LightColor = bIsHit
		? FLinearColor(1.f, 0.4f, 0.1f)
		: FLinearColor(0.2f, 0.6f, 1.f);
	HeadLight->SetLightColor(LightColor);

	// Start at muzzle
	SetActorLocation(StartPos);

	// Initial beam scale (zero length, grows as it travels)
	BeamCore->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.01f));
	BeamGlow->SetRelativeScale3D(FVector(0.01f, 0.03f, 0.03f));
}

void AExoTracer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bReachedEnd)
	{
		TraveledDist += TravelSpeed * DeltaTime;

		if (TraveledDist >= TotalDistance)
		{
			TraveledDist = TotalDistance;
			bReachedEnd = true;
			FadeAge = 0.f;
		}

		// Beam visible length grows until it reaches BeamLength, then stays constant
		float VisibleLen = FMath::Min(TraveledDist, BeamLength);

		// Tail position: beam trails behind the head
		float TailDist = FMath::Max(TraveledDist - BeamLength, 0.f);
		FVector HeadPos = StartPos + Direction * TraveledDist;
		FVector TailPos = StartPos + Direction * TailDist;
		FVector BeamCenter = (HeadPos + TailPos) * 0.5f;

		SetActorLocation(BeamCenter);

		// Scale beam to visible length
		float LenScale = VisibleLen / 100.f;
		BeamCore->SetRelativeScale3D(FVector(LenScale, 0.01f, 0.01f));
		BeamGlow->SetRelativeScale3D(FVector(LenScale * 0.85f, 0.03f, 0.03f));

		// Head mesh at front of beam (local space)
		HeadMesh->SetRelativeLocation(FVector(VisibleLen * 0.5f, 0.f, 0.f));
		HeadLight->SetRelativeLocation(FVector(VisibleLen * 0.5f, 0.f, 0.f));
		HeadLight->SetIntensity(8000.f);
	}
	else
	{
		// Rapid fade after reaching target
		FadeAge += DeltaTime;
		float Alpha = 1.f - FMath::Clamp(FadeAge / FadeTime, 0.f, 1.f);
		float AlphaSq = Alpha * Alpha;

		HeadLight->SetIntensity(8000.f * AlphaSq);
		HeadMesh->SetRelativeScale3D(FVector(0.06f * Alpha));

		// Shrink beam cross-section while maintaining length
		FVector CoreScale = BeamCore->GetRelativeScale3D();
		BeamCore->SetRelativeScale3D(FVector(CoreScale.X, 0.01f * Alpha, 0.01f * Alpha));
		BeamGlow->SetRelativeScale3D(FVector(CoreScale.X * 0.85f, 0.03f * Alpha, 0.03f * Alpha));

		if (FadeAge >= FadeTime) Destroy();
	}
}
