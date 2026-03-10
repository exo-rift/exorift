// ExoShieldBubbleActor.cpp — Energy dome with surface arcs for ShieldBubble ability
#include "Visual/ExoShieldBubbleActor.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoShieldBubbleActor::AExoShieldBubbleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 2.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));

	// Outer dome shell
	DomeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DomeMesh"));
	RootComponent = DomeMesh;
	DomeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DomeMesh->CastShadow = false;
	DomeMesh->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) DomeMesh->SetStaticMesh(SphereFinder.Object);
	DomeMesh->SetWorldScale3D(FVector(0.1f));

	// Inner energy shell — slightly smaller, different pulse rate
	InnerDome = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerDome"));
	InnerDome->SetupAttachment(DomeMesh);
	InnerDome->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InnerDome->CastShadow = false;
	InnerDome->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) InnerDome->SetStaticMesh(SphereFinder.Object);
	InnerDome->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.82f));

	// Base ring on ground
	BaseRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseRing"));
	BaseRing->SetupAttachment(DomeMesh);
	BaseRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseRing->CastShadow = false;
	BaseRing->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) BaseRing->SetStaticMesh(CylFinder.Object);
	BaseRing->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
	BaseRing->SetWorldScale3D(FVector(0.01f, 0.01f, 0.001f));

	// Crown ring at top of dome
	TopCrown = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopCrown"));
	TopCrown->SetupAttachment(DomeMesh);
	TopCrown->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TopCrown->CastShadow = false;
	TopCrown->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) TopCrown->SetStaticMesh(CylFinder.Object);
	TopCrown->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	TopCrown->SetWorldScale3D(FVector(0.01f, 0.01f, 0.001f));

	// Interior shield glow
	ShieldLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ShieldLight"));
	ShieldLight->SetupAttachment(DomeMesh);
	ShieldLight->SetIntensity(60000.f);
	ShieldLight->SetAttenuationRadius(3000.f);
	ShieldLight->SetLightColor(FLinearColor(0.1f, 0.8f, 1.f));
	ShieldLight->CastShadows = false;

	// Top crown light
	TopLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TopLight"));
	TopLight->SetupAttachment(DomeMesh);
	TopLight->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	TopLight->SetIntensity(20000.f);
	TopLight->SetAttenuationRadius(1500.f);
	TopLight->SetLightColor(FLinearColor(0.2f, 0.9f, 1.f));
	TopLight->CastShadows = false;

	// Surface energy arcs — thin emissive bars that crawl across the dome
	for (int32 i = 0; i < NUM_ARCS; i++)
	{
		FName Name = *FString::Printf(TEXT("Arc_%d"), i);
		SurfaceArcs[i] = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		SurfaceArcs[i]->SetupAttachment(DomeMesh);
		SurfaceArcs[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SurfaceArcs[i]->CastShadow = false;
		SurfaceArcs[i]->SetGenerateOverlapEvents(false);
		if (CubeFinder.Succeeded()) SurfaceArcs[i]->SetStaticMesh(CubeFinder.Object);
		SurfaceArcs[i]->SetRelativeScale3D(FVector(0.01f));
		ArcMats[i] = nullptr;
	}
}

void AExoShieldBubbleActor::Init()
{
	UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveAdditive();

	// Outer dome — translucent cyan
	DomeMat = UMaterialInstanceDynamic::Create(EmMat, this);
	DomeMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(0.5f, 3.f, 5.f));
	DomeMesh->SetMaterial(0, DomeMat);

	// Inner dome — dimmer, slightly different hue
	InnerMat = UMaterialInstanceDynamic::Create(EmMat, this);
	InnerMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(0.3f, 1.5f, 3.f));
	InnerDome->SetMaterial(0, InnerMat);

	// Base ring — bright accent
	RingMat = UMaterialInstanceDynamic::Create(EmMat, this);
	RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(1.f, 6.f, 10.f));
	BaseRing->SetMaterial(0, RingMat);

	// Top crown — bright white-cyan
	UMaterialInstanceDynamic* CrownMat = UMaterialInstanceDynamic::Create(EmMat, this);
	CrownMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(2.f, 8.f, 12.f));
	TopCrown->SetMaterial(0, CrownMat);

	// Surface arcs — bright energy crawlers
	for (int32 i = 0; i < NUM_ARCS; i++)
	{
		ArcMats[i] = UMaterialInstanceDynamic::Create(EmMat, this);
		ArcMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(3.f, 10.f, 15.f));
		SurfaceArcs[i]->SetMaterial(0, ArcMats[i]);
	}
}

void AExoShieldBubbleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float Time = GetWorld()->GetTimeSeconds();

	// Phase 1 (0-20%): rapid expand
	// Phase 2 (20-80%): hold with pulse
	// Phase 3 (80-100%): shrink and fade
	float DomeScale;
	float EmScale;
	if (T < 0.2f)
	{
		float ExpandT = T / 0.2f;
		DomeScale = FMath::Lerp(0.1f, 2.5f, FMath::Sqrt(ExpandT));
		EmScale = 1.f + 2.f * (1.f - ExpandT);
	}
	else if (T < 0.8f)
	{
		DomeScale = 2.5f;
		float PulseT = (T - 0.2f) / 0.6f;
		EmScale = 1.f + 0.15f * FMath::Sin(PulseT * 8.f * PI);
	}
	else
	{
		float FadeT = (T - 0.8f) / 0.2f;
		DomeScale = FMath::Lerp(2.5f, 0.1f, FadeT * FadeT);
		EmScale = 1.f - FadeT;
	}

	// Hex-grid flicker
	float Flicker = 1.f + 0.06f * FMath::Sin(Time * 40.f)
		+ 0.04f * FMath::Sin(Time * 67.f);
	DomeMesh->SetWorldScale3D(FVector(DomeScale * Flicker,
		DomeScale * Flicker, DomeScale * 0.7f * Flicker));

	// Base ring + top crown scale with dome
	float RingS = DomeScale * 1.1f;
	BaseRing->SetWorldScale3D(FVector(RingS, RingS, 0.005f));
	float CrownS = DomeScale * 0.3f;
	TopCrown->SetWorldScale3D(FVector(CrownS, CrownS, 0.003f));

	// Animate emissive — outer dome
	if (DomeMat)
	{
		DomeMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.5f * EmScale, 3.f * EmScale, 5.f * EmScale));
	}
	// Inner dome — counter-phase pulse
	if (InnerMat)
	{
		float InnerPulse = 1.f + 0.2f * FMath::Sin(Time * 5.f + PI * 0.5f);
		InnerMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.3f * EmScale * InnerPulse,
				1.5f * EmScale * InnerPulse, 3.f * EmScale * InnerPulse));
	}

	// Lights follow intensity
	ShieldLight->SetIntensity(60000.f * EmScale * (1.f - T * 0.5f));
	TopLight->SetIntensity(20000.f * EmScale * (1.f - T * 0.5f));

	// Surface arcs — crawl across dome surface
	for (int32 i = 0; i < NUM_ARCS; i++)
	{
		float ArcAngle = Time * (2.f + i * 0.5f) + i * (2.f * PI / NUM_ARCS);
		float ArcPhi = FMath::Sin(Time * (1.5f + i * 0.3f)) * 0.6f + 0.3f;

		// Position on dome surface (spherical coords)
		float R = DomeScale * 52.f; // Slightly outside dome surface
		float X = R * FMath::Cos(ArcAngle) * FMath::Sin(ArcPhi * PI);
		float Y = R * FMath::Sin(ArcAngle) * FMath::Sin(ArcPhi * PI);
		float Z = R * 0.7f * FMath::Cos(ArcPhi * PI);
		SurfaceArcs[i]->SetRelativeLocation(FVector(X, Y, Z));

		// Orient tangent to dome
		FRotator ArcRot = FVector(X, Y, Z).Rotation();
		ArcRot.Roll = Time * (200.f + i * 50.f);
		SurfaceArcs[i]->SetRelativeRotation(ArcRot);

		// Flickering scale — arcs randomly stretch
		float ArcFlicker = FMath::Abs(FMath::Sin(Time * (15.f + i * 7.f)));
		float ArcLen = (0.5f + ArcFlicker * 1.5f) * DomeScale * 0.3f;
		float ArcThick = 0.02f + 0.015f * ArcFlicker;
		SurfaceArcs[i]->SetRelativeScale3D(FVector(ArcLen, ArcThick, ArcThick));

		// Arc emissive flicker
		if (ArcMats[i])
		{
			float ArcBright = (2.f + 12.f * ArcFlicker) * EmScale;
			ArcMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(ArcBright * 0.2f, ArcBright * 0.7f, ArcBright));
		}
	}

	if (Age >= Lifetime) Destroy();
}
