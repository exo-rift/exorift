// ExoDeathEffect.cpp — Energy burst + scattering fragments on elimination
#include "Visual/ExoDeathEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoDeathEffect::AExoDeathEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.5f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));

	// Core flash sphere
	CoreFlash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreFlash"));
	RootComponent = CoreFlash;
	CoreFlash->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreFlash->CastShadow = false;
	CoreFlash->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) CoreFlash->SetStaticMesh(SphereFinder.Object);
	CoreFlash->SetWorldScale3D(FVector(0.1f));

	// Expanding shockwave ring
	ShockRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShockRing"));
	ShockRing->SetupAttachment(CoreFlash);
	ShockRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShockRing->CastShadow = false;
	ShockRing->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) ShockRing->SetStaticMesh(CylFinder.Object);
	ShockRing->SetWorldScale3D(FVector(0.01f, 0.01f, 0.001f));

	// Bright burst light
	BurstLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BurstLight"));
	BurstLight->SetupAttachment(CoreFlash);
	BurstLight->SetIntensity(80000.f);
	BurstLight->SetAttenuationRadius(3000.f);
	BurstLight->SetLightColor(FLinearColor(0.3f, 0.6f, 1.f));
	BurstLight->CastShadows = false;

	// Fragment cubes
	for (int32 i = 0; i < NumFragments; i++)
	{
		FName Name = *FString::Printf(TEXT("Frag_%d"), i);
		UStaticMeshComponent* Frag = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Frag->SetupAttachment(CoreFlash);
		Frag->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Frag->CastShadow = false;
		Frag->SetGenerateOverlapEvents(false);
		if (CubeFinder.Succeeded()) Frag->SetStaticMesh(CubeFinder.Object);
		Fragments.Add(Frag);
	}
}

void AExoDeathEffect::Init(const FLinearColor& AccentColor)
{
	UMaterialInterface* BaseMat = CoreFlash->GetMaterial(0);
	if (!BaseMat) return;

	// Core flash — bright white-blue
	UMaterialInstanceDynamic* FlashMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	FlashMat->SetVectorParameterValue(TEXT("BaseColor"),
		FLinearColor(0.8f, 0.9f, 1.f));
	FlashMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(15.f, 18.f, 25.f));
	CoreFlash->SetMaterial(0, FlashMat);

	// Shock ring — accent-colored
	UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	RingMat->SetVectorParameterValue(TEXT("BaseColor"), AccentColor);
	RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(AccentColor.R * 5.f, AccentColor.G * 5.f, AccentColor.B * 5.f));
	ShockRing->SetMaterial(0, RingMat);

	BurstLight->SetLightColor(AccentColor);

	// Fragment velocities and materials
	FragVelocities.SetNum(NumFragments);
	for (int32 i = 0; i < NumFragments; i++)
	{
		FVector Vel = FMath::VRand() * FMath::RandRange(300.f, 800.f);
		Vel.Z = FMath::Abs(Vel.Z) + 200.f;
		FragVelocities[i] = Vel;

		float S = FMath::RandRange(0.02f, 0.07f);
		Fragments[i]->SetWorldScale3D(FVector(S));

		UMaterialInstanceDynamic* FMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		float Brightness = FMath::RandRange(2.f, 6.f);
		FMat->SetVectorParameterValue(TEXT("BaseColor"), AccentColor);
		FMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(AccentColor.R * Brightness,
				AccentColor.G * Brightness, AccentColor.B * Brightness));
		Fragments[i]->SetMaterial(0, FMat);
	}
}

void AExoDeathEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Core flash: rapid expand then fade
	{
		float ExpandT = FMath::Clamp(Age / (Lifetime * 0.2f), 0.f, 1.f);
		float MaxScale = 2.5f;
		float Scale = FMath::Lerp(0.1f, MaxScale, FMath::Sqrt(ExpandT));
		float FadeT = FMath::Clamp((Age - Lifetime * 0.15f) / (Lifetime * 0.5f), 0.f, 1.f);
		Scale *= (1.f - FadeT * FadeT);
		CoreFlash->SetWorldScale3D(FVector(FMath::Max(Scale, 0.01f)));
	}

	// Shock ring: expand outward
	{
		float RingT = FMath::Clamp(Age / (Lifetime * 0.7f), 0.f, 1.f);
		float RingScale = 4.f * RingT;
		float RingAlpha = 1.f - RingT;
		ShockRing->SetWorldScale3D(FVector(RingScale, RingScale, 0.003f * RingAlpha));
		ShockRing->SetVisibility(RingT < 1.f);
	}

	// Light: fast decay
	BurstLight->SetIntensity(80000.f * (1.f - T) * (1.f - T));

	// Fragments: fly outward with gravity and spin
	for (int32 i = 0; i < Fragments.Num(); i++)
	{
		if (!Fragments[i]) continue;
		FVector Pos = FragVelocities[i] * Age;
		Pos.Z -= 490.f * Age * Age;
		Fragments[i]->SetRelativeLocation(Pos);

		FRotator Rot(Age * 400.f * (i + 1), Age * 250.f * (i + 2), Age * 180.f);
		Fragments[i]->SetRelativeRotation(Rot);

		float S = FMath::Lerp(0.05f, 0.002f, T * T);
		Fragments[i]->SetWorldScale3D(FVector(S));
	}

	if (Age >= Lifetime) Destroy();
}
