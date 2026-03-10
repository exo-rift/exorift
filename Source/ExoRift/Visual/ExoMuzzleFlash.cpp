// ExoMuzzleFlash.cpp — Dramatic weapon flash with halo ring and dual lights
#include "Visual/ExoMuzzleFlash.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoMuzzleFlash::AExoMuzzleFlash()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.2f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));

	auto MakeMesh = [&](const TCHAR* Name) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->SetGenerateOverlapEvents(false);
		if (SphereFinder.Succeeded()) C->SetStaticMesh(SphereFinder.Object);
		return C;
	};

	FlashCore = MakeMesh(TEXT("FlashCore"));
	RootComponent = FlashCore;

	FlashCross = MakeMesh(TEXT("FlashCross"));
	FlashCross->SetupAttachment(FlashCore);

	FlashFlare = MakeMesh(TEXT("FlashFlare"));
	FlashFlare->SetupAttachment(FlashCore);

	// Ring uses cylinder for donut-like expanding halo
	FlashRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashRing"));
	FlashRing->SetupAttachment(FlashCore);
	FlashRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlashRing->CastShadow = false;
	FlashRing->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) FlashRing->SetStaticMesh(CylFinder.Object);

	FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlashLight"));
	FlashLight->SetupAttachment(FlashCore);
	FlashLight->SetIntensity(80000.f);
	FlashLight->SetAttenuationRadius(1200.f);
	FlashLight->CastShadows = false;

	BounceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BounceLight"));
	BounceLight->SetupAttachment(FlashCore);
	BounceLight->SetRelativeLocation(FVector(0.f, 0.f, -100.f));
	BounceLight->SetIntensity(25000.f);
	BounceLight->SetAttenuationRadius(600.f);
	BounceLight->CastShadows = false;
}

void AExoMuzzleFlash::InitFlash(const FRotator& FireDirection,
	const FLinearColor& WeaponColor, EWeaponType WeaponType)
{
	SetActorRotation(FireDirection);

	// Weapon-specific flash shape — bigger and more dramatic
	switch (WeaponType)
	{
	case EWeaponType::Shotgun:
		CoreScale = 0.22f; CrossScale = 0.30f; FlareScale = 0.25f;
		RingScale = 0.45f; Lifetime = 0.11f;
		break;
	case EWeaponType::Sniper:
		CoreScale = 0.35f; CrossScale = 0.14f; FlareScale = 0.10f;
		RingScale = 0.30f; Lifetime = 0.13f;
		break;
	case EWeaponType::SMG:
		CoreScale = 0.16f; CrossScale = 0.14f; FlareScale = 0.10f;
		RingScale = 0.22f; Lifetime = 0.06f;
		break;
	case EWeaponType::Pistol:
		CoreScale = 0.20f; CrossScale = 0.18f; FlareScale = 0.14f;
		RingScale = 0.28f; Lifetime = 0.08f;
		break;
	default: // Rifle
		CoreScale = 0.25f; CrossScale = 0.20f; FlareScale = 0.16f;
		RingScale = 0.35f; Lifetime = 0.09f;
		break;
	}

	FlashCore->SetRelativeScale3D(FVector(CoreScale, CoreScale * 0.4f, CoreScale * 0.4f));
	FlashCross->SetRelativeScale3D(FVector(0.04f, CrossScale, 0.03f));
	FlashFlare->SetRelativeScale3D(FVector(0.04f, 0.03f, FlareScale));
	FlashRing->SetRelativeScale3D(FVector(RingScale * 0.3f, RingScale * 0.3f, 0.005f));

	// Extremely bright emissive for visible energy burst
	FLinearColor HotCenter(
		WeaponColor.R * 10.f + 5.f,
		WeaponColor.G * 10.f + 5.f,
		WeaponColor.B * 10.f + 5.f);
	FLinearColor FlareColor(
		WeaponColor.R * 15.f,
		WeaponColor.G * 15.f,
		WeaponColor.B * 15.f);
	FLinearColor RingColor(
		WeaponColor.R * 8.f,
		WeaponColor.G * 8.f,
		WeaponColor.B * 8.f);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		auto ApplyMat = [&](UStaticMeshComponent* Mesh, const FLinearColor& Col)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(
				MatFinder.Object, this);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Col);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), Col);
			Mesh->SetMaterial(0, Mat);
		};
		ApplyMat(FlashCore, HotCenter);
		ApplyMat(FlashCross, FlareColor);
		ApplyMat(FlashFlare, FlareColor);
		ApplyMat(FlashRing, RingColor);
	}

	FlashLight->SetLightColor(WeaponColor);
	FlashLight->SetIntensity(80000.f);
	BaseIntensity = FlashLight->Intensity;

	BounceLight->SetLightColor(WeaponColor);
	BounceLight->SetIntensity(25000.f);
}

void AExoMuzzleFlash::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float Alpha = 1.f - T;
	float AlphaCubed = Alpha * Alpha * Alpha;

	FlashLight->SetIntensity(BaseIntensity * AlphaCubed);
	BounceLight->SetIntensity(25000.f * AlphaCubed);

	if (FlashCore)
	{
		float S = FMath::Lerp(0.04f, CoreScale, Alpha);
		FlashCore->SetRelativeScale3D(FVector(S, S * 0.4f, S * 0.4f));
	}
	if (FlashCross)
	{
		FlashCross->SetRelativeScale3D(FVector(
			0.04f * Alpha, CrossScale * Alpha, 0.03f * Alpha));
	}
	if (FlashFlare)
	{
		FlashFlare->SetRelativeScale3D(FVector(
			0.04f * Alpha, 0.03f * Alpha, FlareScale * Alpha));
	}
	// Ring expands outward as it fades
	if (FlashRing)
	{
		float Expand = FMath::Lerp(0.3f, 1.2f, T);
		FlashRing->SetRelativeScale3D(FVector(
			RingScale * Expand, RingScale * Expand, 0.005f * Alpha));
	}

	if (Age >= Lifetime) Destroy();
}
