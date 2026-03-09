// ExoExplosionEffect.cpp — Fireball, shockwave ring, flash, debris
#include "Visual/ExoExplosionEffect.h"
#include "Visual/ExoScreenShake.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

static constexpr int32 NUM_DEBRIS = 8;

AExoExplosionEffect::AExoExplosionEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.2f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));

	// Fireball — outer orange-red sphere
	FireballMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fireball"));
	RootComponent = FireballMesh;
	FireballMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FireballMesh->CastShadow = false;
	FireballMesh->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) FireballMesh->SetStaticMesh(SphereFinder.Object);

	// Inner white flash — bright core
	InnerFlashMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerFlash"));
	InnerFlashMesh->SetupAttachment(FireballMesh);
	InnerFlashMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InnerFlashMesh->CastShadow = false;
	InnerFlashMesh->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) InnerFlashMesh->SetStaticMesh(SphereFinder.Object);

	// Shockwave ring
	ShockwaveRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShockwaveRing"));
	ShockwaveRing->SetupAttachment(FireballMesh);
	ShockwaveRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShockwaveRing->CastShadow = false;
	ShockwaveRing->SetGenerateOverlapEvents(false);
	if (CylinderFinder.Succeeded()) ShockwaveRing->SetStaticMesh(CylinderFinder.Object);

	// Main explosion light (orange, lingers)
	ExplosionLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ExplosionLight"));
	ExplosionLight->SetupAttachment(FireballMesh);
	ExplosionLight->SetIntensity(60000.f);
	ExplosionLight->SetAttenuationRadius(2500.f);
	ExplosionLight->SetLightColor(FLinearColor(1.f, 0.45f, 0.1f));
	ExplosionLight->CastShadows = false;

	// Flash light (white, very brief)
	FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlashLight"));
	FlashLight->SetupAttachment(FireballMesh);
	FlashLight->SetIntensity(100000.f);
	FlashLight->SetAttenuationRadius(3000.f);
	FlashLight->SetLightColor(FLinearColor(1.f, 0.9f, 0.7f));
	FlashLight->CastShadows = false;

	// Debris chunks
	for (int32 i = 0; i < NUM_DEBRIS; i++)
	{
		FName Name = *FString::Printf(TEXT("Debris_%d"), i);
		UStaticMeshComponent* Debris = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Debris->SetupAttachment(FireballMesh);
		Debris->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Debris->CastShadow = false;
		Debris->SetGenerateOverlapEvents(false);
		if (CubeFinder.Succeeded()) Debris->SetStaticMesh(CubeFinder.Object);
		DebrisMeshes.Add(Debris);
	}
}

void AExoExplosionEffect::InitExplosion(float Radius)
{
	ExpRadius = Radius;

	float InitScale = Radius / 5000.f;
	FireballMesh->SetWorldScale3D(FVector(InitScale));
	InnerFlashMesh->SetWorldScale3D(FVector(InitScale * 0.5f));
	ShockwaveRing->SetWorldScale3D(FVector(0.01f, 0.01f, 0.001f));

	BaseIntensity = ExplosionLight->Intensity;

	// Fireball material — hot orange-red with emissive
	UMaterialInterface* BaseMat = FireballMesh->GetMaterial(0);
	if (BaseMat)
	{
		UMaterialInstanceDynamic* FBMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		FBMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.35f, 0.05f));
		FBMat->SetScalarParameterValue(TEXT("Emissive"), 8.f);
		FireballMesh->SetMaterial(0, FBMat);

		// Inner flash — white-hot
		UMaterialInstanceDynamic* FlashMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		FlashMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.95f, 0.7f));
		FlashMat->SetScalarParameterValue(TEXT("Emissive"), 20.f);
		InnerFlashMesh->SetMaterial(0, FlashMat);

		// Shockwave — cyan-white
		UMaterialInstanceDynamic* SwMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		SwMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.5f, 0.7f, 1.f));
		SwMat->SetScalarParameterValue(TEXT("Emissive"), 5.f);
		ShockwaveRing->SetMaterial(0, SwMat);
	}

	// Screen shake
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		FExoScreenShake::AddExplosionShake(GetActorLocation(),
			PC->GetPawn()->GetActorLocation(), Radius * 3.f, 2.f);
	}

	// Random debris velocities
	DebrisVelocities.SetNum(DebrisMeshes.Num());
	for (int32 i = 0; i < DebrisMeshes.Num(); i++)
	{
		FVector Vel = FMath::VRand() * FMath::RandRange(400.f, 1000.f);
		Vel.Z = FMath::Abs(Vel.Z) + 300.f;
		DebrisVelocities[i] = Vel;

		float S = FMath::RandRange(0.03f, 0.1f);
		DebrisMeshes[i]->SetWorldScale3D(FVector(S, S * 0.6f, S * 0.4f));

		// Debris material — dark with orange emissive
		UMaterialInterface* DBMat = DebrisMeshes[i]->GetMaterial(0);
		if (DBMat)
		{
			UMaterialInstanceDynamic* DMat = UMaterialInstanceDynamic::Create(DBMat, this);
			DMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.3f + FMath::FRand() * 0.2f, 0.15f, 0.05f));
			DMat->SetScalarParameterValue(TEXT("Emissive"), 3.f);
			DebrisMeshes[i]->SetMaterial(0, DMat);
		}
	}
}

void AExoExplosionEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Fireball: rapid expand then shrink and fade
	if (FireballMesh)
	{
		float ExpandT = FMath::Clamp(Age / (Lifetime * 0.25f), 0.f, 1.f);
		float MaxScale = ExpRadius / 700.f;
		float Scale = FMath::Lerp(MaxScale * 0.1f, MaxScale, FMath::Sqrt(ExpandT));
		float FadeT = FMath::Clamp((Age - Lifetime * 0.25f) / (Lifetime * 0.75f), 0.f, 1.f);
		Scale *= (1.f - FadeT * FadeT);
		FireballMesh->SetWorldScale3D(FVector(FMath::Max(Scale, 0.01f)));
	}

	// Inner flash: very fast expand and fade (first 20% of lifetime)
	if (InnerFlashMesh)
	{
		float FlashT = FMath::Clamp(Age / (Lifetime * 0.15f), 0.f, 1.f);
		float FlashScale = ExpRadius / 1200.f * FMath::Sqrt(FlashT);
		float FlashFade = 1.f - FlashT;
		InnerFlashMesh->SetWorldScale3D(FVector(FMath::Max(FlashScale * FlashFade, 0.01f)));
		InnerFlashMesh->SetVisibility(FlashT < 1.f);
	}

	// Shockwave ring: expand outward, flatten and fade
	if (ShockwaveRing)
	{
		float RingT = FMath::Clamp(Age / (Lifetime * 0.6f), 0.f, 1.f);
		float RingScale = ExpRadius / 350.f * RingT;
		float RingAlpha = 1.f - RingT;
		ShockwaveRing->SetWorldScale3D(
			FVector(RingScale, RingScale, 0.004f * RingAlpha));
		ShockwaveRing->SetVisibility(RingT < 1.f);
	}

	// Main light: cubic falloff
	if (ExplosionLight)
	{
		float LightAlpha = 1.f - T;
		ExplosionLight->SetIntensity(BaseIntensity * LightAlpha * LightAlpha * LightAlpha);
	}

	// Flash light: very fast decay (first 10%)
	if (FlashLight)
	{
		float FlashT = FMath::Clamp(Age / (Lifetime * 0.1f), 0.f, 1.f);
		float FlashI = 100000.f * (1.f - FlashT) * (1.f - FlashT);
		FlashLight->SetIntensity(FlashI);
	}

	// Debris: fly outward with gravity and tumble
	for (int32 i = 0; i < DebrisMeshes.Num(); i++)
	{
		if (!DebrisMeshes[i]) continue;
		FVector Pos = DebrisVelocities[i] * Age;
		Pos.Z -= 490.f * Age * Age;
		DebrisMeshes[i]->SetRelativeLocation(Pos);

		FRotator Rot = FRotator(Age * 360.f * (i + 1), Age * 200.f * (i + 2), Age * 150.f);
		DebrisMeshes[i]->SetRelativeRotation(Rot);

		// Shrink and fade debris
		float S = FMath::Lerp(0.08f, 0.005f, T * T);
		DebrisMeshes[i]->SetWorldScale3D(FVector(S, S * 0.6f, S * 0.4f));
	}

	if (Age >= Lifetime) Destroy();
}
