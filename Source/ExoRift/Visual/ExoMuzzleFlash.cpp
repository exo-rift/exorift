#include "Visual/ExoMuzzleFlash.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoMuzzleFlash::AExoMuzzleFlash()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.15f;

	FlashCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashCore"));
	RootComponent = FlashCore;
	FlashCore->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlashCore->CastShadow = false;
	FlashCore->SetGenerateOverlapEvents(false);

	FlashCross = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashCross"));
	FlashCross->SetupAttachment(FlashCore);
	FlashCross->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlashCross->CastShadow = false;
	FlashCross->SetGenerateOverlapEvents(false);

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
	FlashLight->SetIntensity(30000.f);
	FlashLight->SetAttenuationRadius(700.f);
	FlashLight->CastShadows = false;
}

void AExoMuzzleFlash::InitFlash(const FRotator& FireDirection,
	const FLinearColor& WeaponColor, EWeaponType WeaponType)
{
	SetActorRotation(FireDirection);

	// Weapon-specific flash shape: shotgun is wide, sniper is long and narrow
	switch (WeaponType)
	{
	case EWeaponType::Shotgun:
		CoreScale = 0.12f; CrossScale = 0.18f; FlareScale = 0.14f;
		Lifetime = 0.09f;
		break;
	case EWeaponType::Sniper:
		CoreScale = 0.22f; CrossScale = 0.08f; FlareScale = 0.06f;
		Lifetime = 0.10f;
		break;
	case EWeaponType::SMG:
		CoreScale = 0.10f; CrossScale = 0.08f; FlareScale = 0.06f;
		Lifetime = 0.05f;
		break;
	case EWeaponType::Pistol:
		CoreScale = 0.12f; CrossScale = 0.10f; FlareScale = 0.08f;
		Lifetime = 0.06f;
		break;
	default: // Rifle
		CoreScale = 0.15f; CrossScale = 0.12f; FlareScale = 0.10f;
		Lifetime = 0.07f;
		break;
	}

	FlashCore->SetRelativeScale3D(FVector(CoreScale, CoreScale * 0.35f, CoreScale * 0.35f));
	FlashCross->SetRelativeScale3D(FVector(0.03f, CrossScale, 0.02f));
	FlashFlare->SetRelativeScale3D(FVector(0.03f, 0.02f, FlareScale));

	// Weapon-colored flash with white-hot center
	FLinearColor HotCenter(
		WeaponColor.R * 4.f + 2.f,
		WeaponColor.G * 4.f + 2.f,
		WeaponColor.B * 4.f + 2.f);
	FLinearColor FlareColor(
		WeaponColor.R * 6.f,
		WeaponColor.G * 6.f,
		WeaponColor.B * 6.f);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		auto ApplyMat = [&](UStaticMeshComponent* Mesh, const FLinearColor& Col)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Col);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), Col);
			Mesh->SetMaterial(0, Mat);
		};
		ApplyMat(FlashCore, HotCenter);
		ApplyMat(FlashCross, FlareColor);
		ApplyMat(FlashFlare, FlareColor);
	}

	FlashLight->SetLightColor(WeaponColor);
	FlashLight->SetIntensity(30000.f);
	BaseIntensity = FlashLight->Intensity;
}

void AExoMuzzleFlash::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float AlphaCubed = Alpha * Alpha * Alpha;

	if (FlashLight)
	{
		FlashLight->SetIntensity(BaseIntensity * AlphaCubed);
	}

	if (FlashCore)
	{
		FlashCore->SetRelativeScale3D(FVector(
			FMath::Lerp(0.03f, CoreScale, Alpha),
			FMath::Lerp(0.01f, CoreScale * 0.35f, Alpha),
			FMath::Lerp(0.01f, CoreScale * 0.35f, Alpha)));
	}

	if (FlashCross)
	{
		FlashCross->SetRelativeScale3D(FVector(
			0.03f * Alpha, CrossScale * Alpha, 0.02f * Alpha));
	}
	if (FlashFlare)
	{
		FlashFlare->SetRelativeScale3D(FVector(
			0.03f * Alpha, 0.02f * Alpha, FlareScale * Alpha));
	}

	if (Age >= Lifetime)
	{
		Destroy();
	}
}
