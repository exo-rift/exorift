// ExoScanPulseActor.cpp — Multi-ring scan pulse visual for AreaScan ability
#include "Visual/ExoScanPulseActor.h"
#include "Visual/ExoMaterialFactory.h"
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

	// Primary expanding ring
	RingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
	RootComponent = RingMesh;
	RingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RingMesh->CastShadow = false;
	RingMesh->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) RingMesh->SetStaticMesh(CylFinder.Object);
	RingMesh->SetWorldScale3D(FVector(0.01f, 0.01f, 0.002f));

	// Secondary ring — delayed, thinner
	SecondaryRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondaryRing"));
	SecondaryRing->SetupAttachment(RingMesh);
	SecondaryRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SecondaryRing->CastShadow = false;
	SecondaryRing->SetGenerateOverlapEvents(false);
	SecondaryRing->SetVisibility(false);
	if (CylFinder.Succeeded()) SecondaryRing->SetStaticMesh(CylFinder.Object);

	// Tertiary ring — even more delayed, wider
	TertiaryRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TertiaryRing"));
	TertiaryRing->SetupAttachment(RingMesh);
	TertiaryRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TertiaryRing->CastShadow = false;
	TertiaryRing->SetGenerateOverlapEvents(false);
	TertiaryRing->SetVisibility(false);
	if (CylFinder.Succeeded()) TertiaryRing->SetStaticMesh(CylFinder.Object);

	// Inner origin flash
	InnerFlash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerFlash"));
	InnerFlash->SetupAttachment(RingMesh);
	InnerFlash->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InnerFlash->CastShadow = false;
	InnerFlash->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) InnerFlash->SetStaticMesh(SphereFinder.Object);
	InnerFlash->SetWorldScale3D(FVector(0.5f));

	// Ground wave — flat expanding disk
	GroundWave = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundWave"));
	GroundWave->SetupAttachment(RingMesh);
	GroundWave->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GroundWave->CastShadow = false;
	GroundWave->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) GroundWave->SetStaticMesh(CylFinder.Object);
	GroundWave->SetRelativeLocation(FVector(0.f, 0.f, -80.f));
	GroundWave->SetWorldScale3D(FVector(0.01f, 0.01f, 0.001f));

	// Primary pulse light
	PulseLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PulseLight"));
	PulseLight->SetupAttachment(RingMesh);
	PulseLight->SetIntensity(180000.f);
	PulseLight->SetAttenuationRadius(5000.f);
	PulseLight->SetLightColor(FLinearColor(0.1f, 0.5f, 1.f));
	PulseLight->CastShadows = false;

	// Trailing edge light
	TrailingLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TrailingLight"));
	TrailingLight->SetupAttachment(RingMesh);
	TrailingLight->SetIntensity(0.f);
	TrailingLight->SetAttenuationRadius(3000.f);
	TrailingLight->SetLightColor(FLinearColor(0.2f, 0.7f, 1.f));
	TrailingLight->CastShadows = false;
}

void AExoScanPulseActor::Init(float Radius)
{
	ScanRadius = Radius;

	UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveAdditive();

	// Primary ring — bright cyan-blue energy
	UMaterialInstanceDynamic* RMat = UMaterialInstanceDynamic::Create(EmMat, this);
	if (!RMat) { return; }
	RMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(5.f, 14.f, 35.f));
	RingMesh->SetMaterial(0, RMat);

	// Secondary ring — dimmer, blue-white
	UMaterialInstanceDynamic* SecMat = UMaterialInstanceDynamic::Create(EmMat, this);
	if (!SecMat) { return; }
	SecMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(2.5f, 7.f, 18.f));
	SecondaryRing->SetMaterial(0, SecMat);

	// Tertiary ring — faint ripple
	UMaterialInstanceDynamic* TerMat = UMaterialInstanceDynamic::Create(EmMat, this);
	if (!TerMat) { return; }
	TerMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(1.2f, 3.5f, 9.f));
	TertiaryRing->SetMaterial(0, TerMat);

	// Inner flash — searing white-blue
	UMaterialInstanceDynamic* FlashMID = UMaterialInstanceDynamic::Create(EmMat, this);
	if (!FlashMID) { return; }
	FlashMID->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(35.f, 45.f, 70.f));
	InnerFlash->SetMaterial(0, FlashMID);

	// Ground wave — subtle blue sweep
	UMaterialInstanceDynamic* GWMat = UMaterialInstanceDynamic::Create(EmMat, this);
	if (!GWMat) { return; }
	GWMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.7f, 2.3f, 5.f));
	GroundWave->SetMaterial(0, GWMat);
}

void AExoScanPulseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Primary ring: expand from center to ScanRadius
	float RingScale = (ScanRadius / 100.f) * T;
	float FadeAlpha = 1.f - T * T;
	RingMesh->SetWorldScale3D(FVector(RingScale, RingScale, 0.004f * FadeAlpha));

	// Secondary ring: delayed by 0.12s, slightly slower
	float Sec = FMath::Clamp((Age - 0.12f) / Lifetime, 0.f, 1.f);
	if (Sec > 0.f)
	{
		if (!SecondaryRing->IsVisible()) SecondaryRing->SetVisibility(true);
		float SecScale = (ScanRadius / 100.f) * Sec * 0.95f;
		float SecFade = 1.f - Sec * Sec;
		SecondaryRing->SetWorldScale3D(FVector(SecScale, SecScale, 0.003f * SecFade));
	}

	// Tertiary ring: delayed by 0.25s, even slower
	float Ter = FMath::Clamp((Age - 0.25f) / Lifetime, 0.f, 1.f);
	if (Ter > 0.f)
	{
		if (!TertiaryRing->IsVisible()) TertiaryRing->SetVisibility(true);
		float TerScale = (ScanRadius / 100.f) * Ter * 0.9f;
		float TerFade = 1.f - Ter * Ter;
		TertiaryRing->SetWorldScale3D(FVector(TerScale, TerScale, 0.002f * TerFade));
	}

	// Inner flash: rapid expand and fade (first 25%)
	float FlashT = FMath::Clamp(Age / (Lifetime * 0.2f), 0.f, 1.f);
	float FlashScale = 2.f * FMath::Sqrt(FlashT);
	float FlashFade = 1.f - FlashT;
	InnerFlash->SetWorldScale3D(
		FVector(FMath::Max(FlashScale * FlashFade, 0.01f)));
	InnerFlash->SetVisibility(FlashT < 1.f);

	// Ground wave: tracks behind primary ring, flat disk
	float GWScale = RingScale * 0.85f;
	float GWFade = FadeAlpha * 0.5f;
	GroundWave->SetWorldScale3D(FVector(GWScale, GWScale, 0.001f * GWFade));

	// Primary light: follows expansion
	PulseLight->SetIntensity(180000.f * FadeAlpha);
	PulseLight->SetAttenuationRadius(2000.f + RingScale * 50.f);

	// Trailing light: positioned at ring edge, fades with ring
	float EdgeDist = RingScale * 50.f;
	TrailingLight->SetRelativeLocation(FVector(EdgeDist, 0.f, 0.f));
	TrailingLight->SetIntensity(30000.f * FadeAlpha * FadeAlpha);

	if (Age >= Lifetime) Destroy();
}
