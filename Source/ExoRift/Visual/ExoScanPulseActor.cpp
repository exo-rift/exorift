// ExoScanPulseActor.cpp — Expanding ring visual for AreaScan ability
#include "Visual/ExoScanPulseActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoScanPulseActor::AExoScanPulseActor()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.5f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));

	// Expanding ring
	RingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
	RootComponent = RingMesh;
	RingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RingMesh->CastShadow = false;
	RingMesh->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) RingMesh->SetStaticMesh(CylFinder.Object);
	RingMesh->SetWorldScale3D(FVector(0.01f, 0.01f, 0.002f));

	// Inner flash at origin
	InnerFlash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerFlash"));
	InnerFlash->SetupAttachment(RingMesh);
	InnerFlash->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InnerFlash->CastShadow = false;
	InnerFlash->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) InnerFlash->SetStaticMesh(SphereFinder.Object);
	InnerFlash->SetWorldScale3D(FVector(0.5f));

	// Pulse light
	PulseLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PulseLight"));
	PulseLight->SetupAttachment(RingMesh);
	PulseLight->SetIntensity(60000.f);
	PulseLight->SetAttenuationRadius(4000.f);
	PulseLight->SetLightColor(FLinearColor(0.1f, 0.5f, 1.f));
	PulseLight->CastShadows = false;
}

void AExoScanPulseActor::Init(float Radius)
{
	ScanRadius = Radius;

	UMaterialInterface* BaseMat = RingMesh->GetMaterial(0);
	if (!BaseMat) return;

	// Ring — bright cyan-blue energy
	UMaterialInstanceDynamic* RMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	RMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.1f, 0.4f, 1.f));
	RMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(1.f, 4.f, 10.f));
	RingMesh->SetMaterial(0, RMat);

	// Inner flash — bright white-blue origin burst
	UMaterialInstanceDynamic* FMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	FMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.7f, 0.85f, 1.f));
	FMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(10.f, 14.f, 20.f));
	InnerFlash->SetMaterial(0, FMat);
}

void AExoScanPulseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Ring: expand from center to ScanRadius
	float RingScale = (ScanRadius / 100.f) * T;
	float FadeAlpha = 1.f - T * T; // Quadratic fade
	RingMesh->SetWorldScale3D(FVector(RingScale, RingScale, 0.003f * FadeAlpha));

	// Inner flash: rapid expand and fade (first 30%)
	float FlashT = FMath::Clamp(Age / (Lifetime * 0.25f), 0.f, 1.f);
	float FlashScale = 1.5f * FMath::Sqrt(FlashT);
	float FlashFade = 1.f - FlashT;
	InnerFlash->SetWorldScale3D(
		FVector(FMath::Max(FlashScale * FlashFade, 0.01f)));
	InnerFlash->SetVisibility(FlashT < 1.f);

	// Light: trails with ring
	PulseLight->SetIntensity(60000.f * FadeAlpha);
	PulseLight->SetAttenuationRadius(2000.f + RingScale * 50.f);

	if (Age >= Lifetime) Destroy();
}
