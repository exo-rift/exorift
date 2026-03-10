// ExoShieldBubbleActor.cpp — Brief protective dome for ShieldBubble ability
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

	// Dome sphere (upper hemisphere visual via scale squash)
	DomeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DomeMesh"));
	RootComponent = DomeMesh;
	DomeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DomeMesh->CastShadow = false;
	DomeMesh->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) DomeMesh->SetStaticMesh(SphereFinder.Object);
	DomeMesh->SetWorldScale3D(FVector(0.1f));

	// Base ring on ground
	BaseRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseRing"));
	BaseRing->SetupAttachment(DomeMesh);
	BaseRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseRing->CastShadow = false;
	BaseRing->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) BaseRing->SetStaticMesh(CylFinder.Object);
	BaseRing->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
	BaseRing->SetWorldScale3D(FVector(0.01f, 0.01f, 0.001f));

	// Interior shield glow
	ShieldLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ShieldLight"));
	ShieldLight->SetupAttachment(DomeMesh);
	ShieldLight->SetIntensity(40000.f);
	ShieldLight->SetAttenuationRadius(2000.f);
	ShieldLight->SetLightColor(FLinearColor(0.1f, 0.8f, 1.f));
	ShieldLight->CastShadows = false;
}

void AExoShieldBubbleActor::Init()
{
	UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveAdditive();

	// Dome — translucent cyan with emissive
	DomeMat = UMaterialInstanceDynamic::Create(EmMat, this);
	DomeMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(0.5f, 3.f, 5.f));
	DomeMesh->SetMaterial(0, DomeMat);

	// Base ring — brighter accent
	RingMat = UMaterialInstanceDynamic::Create(EmMat, this);
	RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(1.f, 6.f, 10.f));
	BaseRing->SetMaterial(0, RingMat);
}

void AExoShieldBubbleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Phase 1 (0-20%): rapid expand
	// Phase 2 (20-80%): hold with pulse
	// Phase 3 (80-100%): shrink and fade
	float DomeScale;
	float EmScale;
	if (T < 0.2f)
	{
		float ExpandT = T / 0.2f;
		DomeScale = FMath::Lerp(0.1f, 2.5f, FMath::Sqrt(ExpandT));
		EmScale = 1.f + 2.f * (1.f - ExpandT); // Bright flash on expand
	}
	else if (T < 0.8f)
	{
		DomeScale = 2.5f;
		float PulseT = (T - 0.2f) / 0.6f;
		float Pulse = 1.f + 0.15f * FMath::Sin(PulseT * 8.f * PI);
		EmScale = Pulse;
	}
	else
	{
		float FadeT = (T - 0.8f) / 0.2f;
		DomeScale = FMath::Lerp(2.5f, 0.1f, FadeT * FadeT);
		EmScale = 1.f - FadeT;
	}

	DomeMesh->SetWorldScale3D(FVector(DomeScale, DomeScale, DomeScale * 0.7f));

	// Base ring expands/contracts with dome
	float RingS = DomeScale * 1.1f;
	BaseRing->SetWorldScale3D(FVector(RingS, RingS, 0.005f));

	// Animate emissive
	if (DomeMat)
	{
		DomeMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.5f * EmScale, 3.f * EmScale, 5.f * EmScale));
	}

	// Light follows intensity
	ShieldLight->SetIntensity(40000.f * EmScale * (1.f - T * 0.5f));

	// Hex-grid flicker effect on dome
	float Time = GetWorld()->GetTimeSeconds();
	float Flicker = 1.f + 0.08f * FMath::Sin(Time * 40.f)
		+ 0.05f * FMath::Sin(Time * 67.f);
	DomeMesh->SetWorldScale3D(DomeMesh->GetComponentScale() * Flicker);

	if (Age >= Lifetime) Destroy();
}
