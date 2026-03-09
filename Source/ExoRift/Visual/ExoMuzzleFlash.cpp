#include "Visual/ExoMuzzleFlash.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"

AExoMuzzleFlash::AExoMuzzleFlash()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.12f;

	FlashMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashMesh"));
	RootComponent = FlashMesh;
	FlashMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlashMesh->CastShadow = false;
	FlashMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		FlashMesh->SetStaticMesh(SphereFinder.Object);
	}

	FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlashLight"));
	FlashLight->SetupAttachment(FlashMesh);
	FlashLight->SetIntensity(15000.f);
	FlashLight->SetAttenuationRadius(400.f);
	FlashLight->SetLightColor(FLinearColor(1.f, 0.8f, 0.4f));
	FlashLight->CastShadows = false;
}

void AExoMuzzleFlash::InitFlash(const FRotator& FireDirection)
{
	// Elongate the flash along the fire direction
	SetActorRotation(FireDirection);
	FlashMesh->SetWorldScale3D(FVector(0.12f, 0.06f, 0.06f));

	BaseIntensity = FlashLight->Intensity;
}

void AExoMuzzleFlash::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	if (FlashLight)
	{
		FlashLight->SetIntensity(BaseIntensity * Alpha * Alpha);
	}

	if (FlashMesh)
	{
		float S = FMath::Lerp(0.02f, 0.12f, Alpha);
		FlashMesh->SetWorldScale3D(FVector(S * 2.f, S, S));
	}

	if (Age >= Lifetime)
	{
		Destroy();
	}
}
