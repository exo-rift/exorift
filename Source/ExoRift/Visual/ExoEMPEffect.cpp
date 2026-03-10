// ExoEMPEffect.cpp — Blue expanding EMP pulse with arc fragments
#include "Visual/ExoEMPEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

static constexpr int32 NUM_ARCS = 6;

AExoEMPEffect::AExoEMPEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.2f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));

	// Central energy core
	CoreSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreSphere"));
	RootComponent = CoreSphere;
	CoreSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreSphere->CastShadow = false;
	CoreSphere->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) CoreSphere->SetStaticMesh(SphereFinder.Object);

	// Primary expanding ring
	PulseRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PulseRing"));
	PulseRing->SetupAttachment(CoreSphere);
	PulseRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PulseRing->CastShadow = false;
	PulseRing->SetGenerateOverlapEvents(false);
	if (CylinderFinder.Succeeded()) PulseRing->SetStaticMesh(CylinderFinder.Object);

	// Secondary ring (offset timing)
	PulseRing2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PulseRing2"));
	PulseRing2->SetupAttachment(CoreSphere);
	PulseRing2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PulseRing2->CastShadow = false;
	PulseRing2->SetGenerateOverlapEvents(false);
	if (CylinderFinder.Succeeded()) PulseRing2->SetStaticMesh(CylinderFinder.Object);

	// EMP light
	PulseLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PulseLight"));
	PulseLight->SetupAttachment(CoreSphere);
	PulseLight->SetIntensity(200000.f);
	PulseLight->SetAttenuationRadius(5000.f);
	PulseLight->SetLightColor(FLinearColor(0.2f, 0.5f, 1.f));
	PulseLight->CastShadows = false;

	// Arc fragments — thin elongated cubes simulating electrical arcs
	for (int32 i = 0; i < NUM_ARCS; i++)
	{
		FName Name = *FString::Printf(TEXT("Arc_%d"), i);
		UStaticMeshComponent* Arc = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Arc->SetupAttachment(CoreSphere);
		Arc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Arc->CastShadow = false;
		Arc->SetGenerateOverlapEvents(false);
		if (CubeFinder.Succeeded()) Arc->SetStaticMesh(CubeFinder.Object);
		ArcFragments.Add(Arc);
	}
}

void AExoEMPEffect::InitPulse(float Radius)
{
	PulseRadius = Radius;
	CoreSphere->SetWorldScale3D(FVector(0.3f));

	UMaterialInterface* BaseMat = CoreSphere->GetMaterial(0);
	if (!BaseMat) return;

	// Core — bright cyan-white
	UMaterialInstanceDynamic* CoreMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	CoreMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.5f, 0.8f, 1.f));
	CoreMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(20.f, 30.f, 50.f));
	CoreSphere->SetMaterial(0, CoreMat);

	// Primary ring — blue
	UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	RingMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.1f, 0.3f, 1.f));
	RingMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(5.f, 10.f, 30.f));
	PulseRing->SetMaterial(0, RingMat);

	// Secondary ring — lighter cyan
	UMaterialInstanceDynamic* Ring2Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
	Ring2Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.3f, 0.6f, 1.f));
	Ring2Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(8.f, 15.f, 25.f));
	PulseRing2->SetMaterial(0, Ring2Mat);

	// Arc fragments — white-blue, thin and bright
	ArcDirections.SetNum(NUM_ARCS);
	for (int32 i = 0; i < NUM_ARCS; i++)
	{
		FVector Dir = FMath::VRand();
		Dir.Z *= 0.3f; // Mostly horizontal
		ArcDirections[i] = Dir.GetSafeNormal();

		ArcFragments[i]->SetWorldScale3D(FVector(0.02f, 0.02f, 0.5f));

		UMaterialInstanceDynamic* ArcMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		ArcMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.7f, 0.9f, 1.f));
		float Bright = FMath::RandRange(10.f, 20.f);
		ArcMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Bright * 0.5f, Bright * 0.8f, Bright));
		ArcFragments[i]->SetMaterial(0, ArcMat);
	}
}

void AExoEMPEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Core sphere: rapid expand then shrink
	{
		float ExpandT = FMath::Clamp(Age / (Lifetime * 0.15f), 0.f, 1.f);
		float MaxScale = PulseRadius / 800.f;
		float Scale = FMath::Lerp(0.3f, MaxScale, FMath::Sqrt(ExpandT));
		float FadeT = FMath::Clamp((Age - Lifetime * 0.15f) / (Lifetime * 0.6f), 0.f, 1.f);
		Scale *= (1.f - FadeT * FadeT);
		CoreSphere->SetWorldScale3D(FVector(FMath::Max(Scale, 0.01f)));
	}

	// Primary ring: expand outward
	{
		float RingT = FMath::Clamp(Age / (Lifetime * 0.5f), 0.f, 1.f);
		float RingScale = PulseRadius / 250.f * RingT;
		float RingAlpha = 1.f - RingT;
		PulseRing->SetWorldScale3D(FVector(RingScale, RingScale, 0.003f * RingAlpha));
		PulseRing->SetVisibility(RingT < 1.f);
	}

	// Secondary ring: delayed expansion
	{
		float Ring2T = FMath::Clamp((Age - Lifetime * 0.1f) / (Lifetime * 0.5f), 0.f, 1.f);
		float Ring2Scale = PulseRadius / 300.f * Ring2T;
		float Ring2Alpha = 1.f - Ring2T;
		PulseRing2->SetWorldScale3D(FVector(Ring2Scale, Ring2Scale, 0.003f * Ring2Alpha));
		PulseRing2->SetVisibility(Ring2T > 0.f && Ring2T < 1.f);
	}

	// Light: fast quadratic decay
	PulseLight->SetIntensity(200000.f * (1.f - T) * (1.f - T));

	// Arc fragments: fly outward from center, flickering
	float Time = GetWorld()->GetTimeSeconds();
	for (int32 i = 0; i < ArcFragments.Num(); i++)
	{
		if (!ArcFragments[i]) continue;

		float ArcT = FMath::Clamp(Age / (Lifetime * 0.6f), 0.f, 1.f);
		float Dist = PulseRadius * ArcT * 0.8f;

		// Jitter position for electrical look
		FVector Jitter = FVector(
			FMath::Sin(Time * 30.f + i * 7.f) * 30.f,
			FMath::Cos(Time * 25.f + i * 11.f) * 30.f,
			FMath::Sin(Time * 20.f + i * 5.f) * 15.f);

		FVector Pos = ArcDirections[i] * Dist + Jitter;
		ArcFragments[i]->SetRelativeLocation(Pos);

		// Orient along direction of travel
		FRotator ArcRot = ArcDirections[i].Rotation();
		ArcRot.Roll = Time * 500.f + i * 60.f;
		ArcFragments[i]->SetRelativeRotation(ArcRot);

		// Scale: elongated, shrinking over time
		float ArcLen = FMath::Lerp(0.8f, 0.05f, ArcT);
		float ArcWidth = 0.03f * (1.f - ArcT);
		ArcFragments[i]->SetWorldScale3D(FVector(ArcWidth, ArcWidth, ArcLen));
		ArcFragments[i]->SetVisibility(ArcT < 1.f);
	}

	if (Age >= Lifetime) Destroy();
}
