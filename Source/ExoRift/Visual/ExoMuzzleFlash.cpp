#include "Visual/ExoMuzzleFlash.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoMuzzleFlash::AExoMuzzleFlash()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.15f;

	// Core flash — elongated along fire direction
	FlashCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashCore"));
	RootComponent = FlashCore;
	FlashCore->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlashCore->CastShadow = false;
	FlashCore->SetGenerateOverlapEvents(false);

	// Cross flare — perpendicular to fire direction (horizontal)
	FlashCross = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashCross"));
	FlashCross->SetupAttachment(FlashCore);
	FlashCross->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlashCross->CastShadow = false;
	FlashCross->SetGenerateOverlapEvents(false);

	// Vertical flare — perpendicular to fire direction (vertical)
	FlashFlare = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashFlare"));
	FlashFlare->SetupAttachment(FlashCore);
	FlashFlare->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlashFlare->CastShadow = false;
	FlashFlare->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		FlashCore->SetStaticMesh(SphereFinder.Object);
		FlashCross->SetStaticMesh(SphereFinder.Object);
		FlashFlare->SetStaticMesh(SphereFinder.Object);
	}

	FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlashLight"));
	FlashLight->SetupAttachment(FlashCore);
	FlashLight->SetIntensity(25000.f);
	FlashLight->SetAttenuationRadius(600.f);
	FlashLight->SetLightColor(FLinearColor(0.4f, 0.7f, 1.f));
	FlashLight->CastShadows = false;
}

void AExoMuzzleFlash::InitFlash(const FRotator& FireDirection)
{
	SetActorRotation(FireDirection);

	// Core: elongated forward
	FlashCore->SetRelativeScale3D(FVector(0.15f, 0.05f, 0.05f));

	// Cross flare: wide horizontal spread
	FlashCross->SetRelativeScale3D(FVector(0.03f, 0.12f, 0.02f));

	// Vertical flare: thin vertical streak
	FlashFlare->SetRelativeScale3D(FVector(0.03f, 0.02f, 0.10f));

	// Bright sci-fi energy flash material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		FLinearColor FlashColor(3.f, 6.f, 12.f, 1.f); // Bright blue-white energy
		auto ApplyFlashMat = [&](UStaticMeshComponent* Mesh)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), FlashColor);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FlashColor);
			Mesh->SetMaterial(0, Mat);
		};
		ApplyFlashMat(FlashCore);
		ApplyFlashMat(FlashCross);
		ApplyFlashMat(FlashFlare);
	}

	BaseIntensity = FlashLight->Intensity;
}

void AExoMuzzleFlash::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float AlphaCubed = Alpha * Alpha * Alpha;

	// Light: sharp cubic falloff
	if (FlashLight)
	{
		FlashLight->SetIntensity(BaseIntensity * AlphaCubed);
	}

	// Core: shrinks from forward edge
	if (FlashCore)
	{
		FlashCore->SetRelativeScale3D(FVector(
			FMath::Lerp(0.03f, 0.15f, Alpha),
			FMath::Lerp(0.01f, 0.05f, Alpha),
			FMath::Lerp(0.01f, 0.05f, Alpha)));
	}

	// Cross flares: collapse inward
	if (FlashCross)
	{
		FlashCross->SetRelativeScale3D(FVector(
			0.03f * Alpha, 0.12f * Alpha, 0.02f * Alpha));
	}
	if (FlashFlare)
	{
		FlashFlare->SetRelativeScale3D(FVector(
			0.03f * Alpha, 0.02f * Alpha, 0.10f * Alpha));
	}

	if (Age >= Lifetime)
	{
		Destroy();
	}
}
