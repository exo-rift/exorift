#include "Visual/ExoPickupFlash.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoPickupFlash::AExoPickupFlash()
{
	PrimaryActorTick.bCanEverTick = true;
	SetLifeSpan(0.4f);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Light flash
	FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlashLight"));
	FlashLight->SetupAttachment(RootComponent);
	FlashLight->SetIntensity(45000.f);
	FlashLight->SetAttenuationRadius(600.f);
	FlashLight->CastShadows = false;

	// Small glowing sphere
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		FlashSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashSphere"));
		FlashSphere->SetupAttachment(RootComponent);
		FlashSphere->SetStaticMesh(SphereFinder.Object);
		FlashSphere->SetRelativeScale3D(FVector(0.3f));
		FlashSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FlashSphere->CastShadow = false;
	}
}

void AExoPickupFlash::Init(const FLinearColor& Color)
{
	FlashColor = Color;
	if (FlashLight) FlashLight->SetLightColor(Color);

	if (FlashSphere)
	{
		FlashMat = UMaterialInstanceDynamic::Create(
			FExoMaterialFactory::GetEmissiveAdditive(), this);
		if (!FlashMat) { return; }
		FlashMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 18.f, Color.G * 18.f, Color.B * 18.f));
		FlashSphere->SetMaterial(0, FlashMat);
	}
}

void AExoPickupFlash::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	// Cubic falloff for fast fade
	Alpha = Alpha * Alpha * Alpha;

	if (FlashLight)
	{
		FlashLight->SetIntensity(BaseIntensity * Alpha);
		FlashLight->SetAttenuationRadius(300.f + 600.f * (1.f - Alpha));
	}

	if (FlashSphere)
	{
		// Expand and fade
		float Scale = 0.3f + 0.5f * (1.f - Alpha);
		FlashSphere->SetRelativeScale3D(FVector(Scale));
	}

	if (FlashMat)
	{
		FlashMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(FlashColor.R * 18.f * Alpha, FlashColor.G * 18.f * Alpha,
				FlashColor.B * 18.f * Alpha));
	}
}

void AExoPickupFlash::SpawnAt(UWorld* World, const FVector& Location, const FLinearColor& Color)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoPickupFlash* Flash = World->SpawnActor<AExoPickupFlash>(
		AExoPickupFlash::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Flash) Flash->Init(Color);
}
