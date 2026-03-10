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
	InitialLifeSpan = 1.5f;

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
	ExplosionLight->SetIntensity(180000.f);
	ExplosionLight->SetAttenuationRadius(6000.f);
	ExplosionLight->SetLightColor(FLinearColor(1.f, 0.45f, 0.1f));
	ExplosionLight->CastShadows = false;

	// Flash light (white, very brief)
	FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlashLight"));
	FlashLight->SetupAttachment(FireballMesh);
	FlashLight->SetIntensity(350000.f);
	FlashLight->SetAttenuationRadius(8000.f);
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

	// Ground scorch mark — flat dark cylinder
	ScorchMark = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScorchMark"));
	ScorchMark->SetupAttachment(FireballMesh);
	ScorchMark->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScorchMark->CastShadow = false;
	ScorchMark->SetGenerateOverlapEvents(false);
	ScorchMark->SetVisibility(false);
	if (CylinderFinder.Succeeded()) ScorchMark->SetStaticMesh(CylinderFinder.Object);

	// Smoke column — rising dark cylinder
	SmokeColumn = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SmokeColumn"));
	SmokeColumn->SetupAttachment(FireballMesh);
	SmokeColumn->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SmokeColumn->CastShadow = false;
	SmokeColumn->SetGenerateOverlapEvents(false);
	SmokeColumn->SetVisibility(false);
	if (CylinderFinder.Succeeded()) SmokeColumn->SetStaticMesh(CylinderFinder.Object);
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
		FBMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.4f, 0.08f));
		FBMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(18.f, 7.f, 1.2f));
		FireballMesh->SetMaterial(0, FBMat);

		// Inner flash — white-hot
		UMaterialInstanceDynamic* FlashMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		FlashMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.95f, 0.7f));
		FlashMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(40.f, 35.f, 25.f));
		InnerFlashMesh->SetMaterial(0, FlashMat);

		// Shockwave — cyan-white
		UMaterialInstanceDynamic* SwMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		SwMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.6f, 0.8f, 1.f));
		SwMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(8.f, 10.f, 15.f));
		ShockwaveRing->SetMaterial(0, SwMat);

		// Ground scorch — dark burn mark
		if (ScorchMark)
		{
			UMaterialInstanceDynamic* ScorchMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			ScorchMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.015f, 0.012f, 0.01f));
			ScorchMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.3f, 0.08f, 0.02f));
			ScorchMark->SetMaterial(0, ScorchMat);
		}

		// Smoke column — dark translucent rising pillar
		if (SmokeColumn)
		{
			UMaterialInstanceDynamic* SmokeMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			SmokeMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.06f, 0.05f, 0.04f));
			SmokeColumn->SetMaterial(0, SmokeMat);
		}
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
		FVector Vel = FMath::VRand() * FMath::RandRange(600.f, 1500.f);
		Vel.Z = FMath::Abs(Vel.Z) + 500.f;
		DebrisVelocities[i] = Vel;

		float S = FMath::RandRange(0.05f, 0.15f);
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
		float MaxScale = ExpRadius / 450.f;
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
		float FlashI = 200000.f * (1.f - FlashT) * (1.f - FlashT);
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

	// Ground scorch: appears after initial flash, expands to final size
	if (ScorchMark)
	{
		float ScorchT = FMath::Clamp((Age - Lifetime * 0.1f) / (Lifetime * 0.3f), 0.f, 1.f);
		if (ScorchT > 0.f && !ScorchMark->IsVisible())
		{
			ScorchMark->SetVisibility(true);
		}
		float ScorchScale = ExpRadius / 400.f * FMath::Sqrt(ScorchT);
		ScorchMark->SetRelativeLocation(FVector(0.f, 0.f, -GetActorLocation().Z + 2.f));
		ScorchMark->SetWorldScale3D(FVector(ScorchScale, ScorchScale, 0.02f));
	}

	// Smoke column: rises from explosion center, expanding and fading
	if (SmokeColumn)
	{
		float SmokeT = FMath::Clamp((Age - Lifetime * 0.15f) / (Lifetime * 0.7f), 0.f, 1.f);
		if (SmokeT > 0.f && !SmokeColumn->IsVisible())
		{
			SmokeColumn->SetVisibility(true);
		}
		float SmokeHeight = ExpRadius * 2.f * SmokeT;
		float SmokeWidth = ExpRadius / 800.f * (0.5f + SmokeT * 0.5f);
		SmokeColumn->SetRelativeLocation(FVector(0.f, 0.f, SmokeHeight * 0.5f));
		SmokeColumn->SetWorldScale3D(FVector(SmokeWidth, SmokeWidth, SmokeHeight / 100.f));
	}

	if (Age >= Lifetime) Destroy();
}
