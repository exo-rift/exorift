// ExoMuzzleFlash.cpp — Dramatic weapon flash with halo ring and dual lights
#include "Visual/ExoMuzzleFlash.h"
#include "Visual/ExoMaterialFactory.h"
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

	// Weapon-specific flash shape — big, bright energy bursts
	switch (WeaponType)
	{
	case EWeaponType::Shotgun:
		CoreScale = 0.80f; CrossScale = 0.90f; FlareScale = 0.75f;
		RingScale = 1.3f; Lifetime = 0.16f;
		break;
	case EWeaponType::Sniper:
		CoreScale = 1.0f; CrossScale = 0.45f; FlareScale = 0.35f;
		RingScale = 0.90f; Lifetime = 0.18f;
		break;
	case EWeaponType::SMG:
		CoreScale = 0.50f; CrossScale = 0.40f; FlareScale = 0.30f;
		RingScale = 0.65f; Lifetime = 0.09f;
		break;
	case EWeaponType::Pistol:
		CoreScale = 0.60f; CrossScale = 0.50f; FlareScale = 0.40f;
		RingScale = 0.80f; Lifetime = 0.12f;
		break;
	default: // Rifle
		CoreScale = 0.70f; CrossScale = 0.60f; FlareScale = 0.50f;
		RingScale = 0.95f; Lifetime = 0.13f;
		break;
	}

	FlashCore->SetRelativeScale3D(FVector(CoreScale, CoreScale * 0.4f, CoreScale * 0.4f));
	FlashCross->SetRelativeScale3D(FVector(0.04f, CrossScale, 0.03f));
	FlashFlare->SetRelativeScale3D(FVector(0.04f, 0.03f, FlareScale));
	FlashRing->SetRelativeScale3D(FVector(RingScale * 0.3f, RingScale * 0.3f, 0.005f));

	// Blazing emissive energy burst (cranked for massive bloom)
	FLinearColor HotCenter(
		WeaponColor.R * 70.f + 40.f,
		WeaponColor.G * 70.f + 40.f,
		WeaponColor.B * 70.f + 40.f);
	FLinearColor FlareColor(
		WeaponColor.R * 90.f,
		WeaponColor.G * 90.f,
		WeaponColor.B * 90.f);
	FLinearColor RingColor(
		WeaponColor.R * 45.f,
		WeaponColor.G * 45.f,
		WeaponColor.B * 45.f);

	UMaterialInterface* BaseMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (BaseMat)
	{
		auto ApplyMat = [&](UStaticMeshComponent* Mesh, const FLinearColor& Col)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), Col);
			Mesh->SetMaterial(0, Mat);
		};
		ApplyMat(FlashCore, HotCenter);
		ApplyMat(FlashCross, FlareColor);
		ApplyMat(FlashFlare, FlareColor);
		ApplyMat(FlashRing, RingColor);
	}

	FlashLight->SetLightColor(WeaponColor);
	FlashLight->SetIntensity(250000.f);
	FlashLight->SetAttenuationRadius(3000.f);
	BaseIntensity = FlashLight->Intensity;

	BounceLight->SetLightColor(WeaponColor);
	BounceLight->SetIntensity(80000.f);
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
