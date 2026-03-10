// ExoImpactEffect.cpp — Dramatic energy impact burst with shockwave ring
#include "Visual/ExoImpactEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoImpactEffect::AExoImpactEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.8f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));

	CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreMesh"));
	RootComponent = CoreMesh;
	CoreMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreMesh->CastShadow = false;
	CoreMesh->SetGenerateOverlapEvents(false);
	CoreMesh->SetWorldScale3D(FVector(0.5f));
	if (SphereFinder.Succeeded()) CoreMesh->SetStaticMesh(SphereFinder.Object);

	DustPuff = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DustPuff"));
	DustPuff->SetupAttachment(CoreMesh);
	DustPuff->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DustPuff->CastShadow = false;
	DustPuff->SetGenerateOverlapEvents(false);
	DustPuff->SetWorldScale3D(FVector(0.1f));
	if (SphereFinder.Succeeded()) DustPuff->SetStaticMesh(SphereFinder.Object);

	// Expanding shockwave ring
	ShockwaveRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShockwaveRing"));
	ShockwaveRing->SetupAttachment(CoreMesh);
	ShockwaveRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShockwaveRing->CastShadow = false;
	ShockwaveRing->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) ShockwaveRing->SetStaticMesh(CylFinder.Object);

	// Spark shards — more for dramatic scatter
	for (int32 i = 0; i < IMPACT_NUM_SPARKS; i++)
	{
		FName Name = *FString::Printf(TEXT("Spark_%d"), i);
		UStaticMeshComponent* Spark = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Spark->SetupAttachment(CoreMesh);
		Spark->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Spark->CastShadow = false;
		Spark->SetGenerateOverlapEvents(false);
		if (CubeFinder.Succeeded()) Spark->SetStaticMesh(CubeFinder.Object);
		SparkMeshes.Add(Spark);
	}

	FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlashLight"));
	FlashLight->SetupAttachment(CoreMesh);
	FlashLight->SetIntensity(120000.f);
	FlashLight->SetAttenuationRadius(1800.f);
	FlashLight->CastShadows = false;
}

void AExoImpactEffect::InitEffect(const FVector& InHitNormal, bool bHitCharacter,
	const FLinearColor& WeaponColor)
{
	HitNorm = InHitNormal;

	// Weapon-colored impacts: blend weapon color with hit type tint
	FLinearColor SparkColor;
	FLinearColor LightColor;
	FLinearColor DustColor;
	FLinearColor RingColor;

	if (bHitCharacter)
	{
		// Character hits: red-orange base tinted toward weapon color
		SparkColor = FLinearColor(
			40.f + WeaponColor.R * 20.f,
			8.f + WeaponColor.G * 8.f,
			2.f + WeaponColor.B * 6.f);
		LightColor = FLinearColor(
			0.8f + WeaponColor.R * 0.2f,
			0.2f + WeaponColor.G * 0.15f,
			0.05f + WeaponColor.B * 0.1f);
		DustColor = FLinearColor(0.5f, 0.08f, 0.03f);
		RingColor = FLinearColor(
			30.f + WeaponColor.R * 15.f,
			6.f + WeaponColor.G * 6.f,
			2.f + WeaponColor.B * 4.f);
	}
	else
	{
		// Surface hits: weapon-colored energy sparks
		SparkColor = FLinearColor(
			WeaponColor.R * 55.f + 5.f,
			WeaponColor.G * 55.f + 5.f,
			WeaponColor.B * 55.f + 5.f);
		LightColor = WeaponColor;
		DustColor = FLinearColor(0.2f, 0.2f, 0.25f);
		RingColor = FLinearColor(
			WeaponColor.R * 35.f + 3.f,
			WeaponColor.G * 35.f + 3.f,
			WeaponColor.B * 35.f + 3.f);
	}

	FlashLight->SetLightColor(LightColor);
	FlashLight->SetIntensity(180000.f);
	BaseIntensity = FlashLight->Intensity;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!MatFinder.Succeeded()) return;

	auto MakeMat = [&](const FLinearColor& Col) -> UMaterialInstanceDynamic*
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		M->SetVectorParameterValue(TEXT("BaseColor"), Col);
		M->SetVectorParameterValue(TEXT("EmissiveColor"), Col);
		return M;
	};

	CoreMesh->SetMaterial(0, MakeMat(SparkColor * 2.f));

	UMaterialInstanceDynamic* DustMat = UMaterialInstanceDynamic::Create(
		MatFinder.Object, this);
	DustMat->SetVectorParameterValue(TEXT("BaseColor"), DustColor);
	DustPuff->SetMaterial(0, DustMat);

	// Shockwave ring aligned to hit normal
	ShockwaveRing->SetMaterial(0, MakeMat(RingColor));
	FRotator RingRot = InHitNormal.Rotation();
	RingRot.Pitch += 90.f;
	ShockwaveRing->SetRelativeRotation(RingRot);
	ShockwaveRing->SetWorldScale3D(FVector(0.1f, 0.1f, 0.003f));

	// Build tangent frame from hit normal
	FVector Tangent, Bitangent;
	InHitNormal.FindBestAxisVectors(Tangent, Bitangent);

	SparkVelocities.SetNum(SparkMeshes.Num());
	for (int32 i = 0; i < SparkMeshes.Num(); i++)
	{
		FVector Vel = InHitNormal * FMath::RandRange(400.f, 1200.f);
		Vel += Tangent * FMath::RandRange(-500.f, 500.f);
		Vel += Bitangent * FMath::RandRange(-500.f, 500.f);
		SparkVelocities[i] = Vel;

		float S = FMath::RandRange(0.06f, 0.14f);
		SparkMeshes[i]->SetWorldScale3D(FVector(S * 5.f, S, S));
		SparkMeshes[i]->SetMaterial(0, MakeMat(SparkColor * FMath::RandRange(0.6f, 1.2f)));
	}
}

void AExoImpactEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float Alpha = 1.f - T;

	// Flash light: sharp cubic falloff
	FlashLight->SetIntensity(BaseIntensity * Alpha * Alpha * Alpha);

	// Core: rapid expand then shrink with energy flicker
	float CorePhase = FMath::Clamp(Age / (Lifetime * 0.25f), 0.f, 1.f);
	float S = 0.5f * (1.f - CorePhase * CorePhase);
	S *= 1.f + 0.35f * FMath::Sin(Age * 90.f);
	CoreMesh->SetWorldScale3D(FVector(FMath::Max(S, 0.001f)));

	// Dust puff: expand outward and drift up
	float PuffScale = FMath::Lerp(0.15f, 1.0f, FMath::Sqrt(T));
	DustPuff->SetWorldScale3D(FVector(PuffScale));
	DustPuff->SetRelativeLocation(HitNorm * Age * 60.f);

	// Shockwave ring: expand rapidly, flatten and fade
	float RingExpand = FMath::Lerp(0.1f, 2.5f, FMath::Sqrt(T));
	float RingThick = 0.003f * FMath::Max(Alpha * 2.f, 0.01f);
	ShockwaveRing->SetWorldScale3D(FVector(RingExpand, RingExpand, RingThick));

	// Sparks: fly outward with gravity, tumbling
	for (int32 i = 0; i < SparkMeshes.Num(); i++)
	{
		if (!SparkMeshes[i]) continue;
		FVector Pos = SparkVelocities[i] * Age;
		Pos.Z -= 500.f * Age * Age;
		SparkMeshes[i]->SetRelativeLocation(Pos);

		float Sp = 0.06f * Alpha;
		float Stretch = FMath::Min(SparkVelocities[i].Size() / 200.f, 7.f);
		SparkMeshes[i]->SetWorldScale3D(FVector(Sp * Stretch, Sp * 0.4f, Sp * 0.4f));
		FVector Dir = SparkVelocities[i] + FVector(0.f, 0.f, -1000.f * Age);
		if (!Dir.IsNearlyZero())
		{
			SparkMeshes[i]->SetWorldRotation(Dir.Rotation());
		}
	}

	if (Age >= Lifetime) Destroy();
}
