#include "Visual/ExoImpactEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoImpactEffect::AExoImpactEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.4f;

	CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreMesh"));
	RootComponent = CoreMesh;
	CoreMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreMesh->CastShadow = false;
	CoreMesh->SetGenerateOverlapEvents(false);
	CoreMesh->SetWorldScale3D(FVector(0.2f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		CoreMesh->SetStaticMesh(SphereFinder.Object);
	}

	// Dust puff — expanding translucent sphere
	DustPuff = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DustPuff"));
	DustPuff->SetupAttachment(CoreMesh);
	DustPuff->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DustPuff->CastShadow = false;
	DustPuff->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded())
	{
		DustPuff->SetStaticMesh(SphereFinder.Object);
	}
	DustPuff->SetWorldScale3D(FVector(0.05f));

	// Spark shards
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	for (int32 i = 0; i < NUM_SPARKS; i++)
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
	FlashLight->SetIntensity(25000.f);
	FlashLight->SetAttenuationRadius(600.f);
	FlashLight->CastShadows = false;
}

void AExoImpactEffect::InitEffect(const FVector& HitNormal, bool bHitCharacter)
{
	// Color scheme: red/orange for characters, bright cyan-white for surfaces
	FLinearColor SparkColor = bHitCharacter
		? FLinearColor(12.f, 2.f, 0.5f, 1.f)
		: FLinearColor(6.f, 10.f, 14.f, 1.f);

	FLinearColor LightColor = bHitCharacter
		? FLinearColor(1.f, 0.3f, 0.1f)
		: FLinearColor(0.5f, 0.8f, 1.f);

	FLinearColor DustColor = bHitCharacter
		? FLinearColor(0.5f, 0.08f, 0.03f, 1.f)
		: FLinearColor(0.2f, 0.2f, 0.25f, 1.f);

	FlashLight->SetLightColor(LightColor);
	BaseIntensity = FlashLight->Intensity;

	// Apply core material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		UMaterialInstanceDynamic* CoreMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		CoreMat->SetVectorParameterValue(TEXT("BaseColor"), SparkColor);
		CoreMat->SetVectorParameterValue(TEXT("EmissiveColor"), SparkColor);
		CoreMesh->SetMaterial(0, CoreMat);

		UMaterialInstanceDynamic* DustMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		DustMat->SetVectorParameterValue(TEXT("BaseColor"), DustColor);
		DustPuff->SetMaterial(0, DustMat);

		for (auto* Spark : SparkMeshes)
		{
			UMaterialInstanceDynamic* SM = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
			SM->SetVectorParameterValue(TEXT("BaseColor"), SparkColor);
			SM->SetVectorParameterValue(TEXT("EmissiveColor"), SparkColor * 1.5f);
			Spark->SetMaterial(0, SM);
		}
	}

	// Build tangent frame from hit normal
	FVector Tangent, Bitangent;
	HitNormal.FindBestAxisVectors(Tangent, Bitangent);

	// Random spark velocities in hemisphere above surface
	SparkVelocities.SetNum(SparkMeshes.Num());
	for (int32 i = 0; i < SparkMeshes.Num(); i++)
	{
		FVector Vel = HitNormal * FMath::RandRange(200.f, 500.f);
		Vel += Tangent * FMath::RandRange(-200.f, 200.f);
		Vel += Bitangent * FMath::RandRange(-200.f, 200.f);
		SparkVelocities[i] = Vel;

		float S = FMath::RandRange(0.02f, 0.05f);
		SparkMeshes[i]->SetWorldScale3D(FVector(S * 4.f, S, S));
	}
}

void AExoImpactEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float Alpha = 1.f - T;

	// Flash light: very sharp falloff
	if (FlashLight)
	{
		FlashLight->SetIntensity(BaseIntensity * Alpha * Alpha * Alpha);
	}

	// Core: rapid expand then shrink
	if (CoreMesh)
	{
		float CorePhase = FMath::Clamp(Age / (Lifetime * 0.3f), 0.f, 1.f);
		float S = 0.2f * (1.f - CorePhase * CorePhase);
		// Flicker for energy-burst feel
		S *= 1.f + 0.3f * FMath::Sin(Age * 80.f);
		CoreMesh->SetWorldScale3D(FVector(FMath::Max(S, 0.001f)));
	}

	// Dust puff: expand outward and fade
	if (DustPuff)
	{
		float PuffScale = FMath::Lerp(0.08f, 0.5f, FMath::Sqrt(T));
		DustPuff->SetWorldScale3D(FVector(PuffScale));
		DustPuff->SetRelativeLocation(FVector(0.f, 0.f, Age * 40.f));
	}

	// Sparks: fly outward with gravity
	for (int32 i = 0; i < SparkMeshes.Num(); i++)
	{
		if (!SparkMeshes[i]) continue;
		FVector Pos = SparkVelocities[i] * Age;
		Pos.Z -= 400.f * Age * Age;
		SparkMeshes[i]->SetRelativeLocation(Pos);

		// Elongate sparks along velocity for streak effect
		float S = 0.04f * Alpha;
		float Stretch = FMath::Min(SparkVelocities[i].Size() / 200.f, 6.f);
		SparkMeshes[i]->SetWorldScale3D(FVector(S * Stretch, S * 0.4f, S * 0.4f));
		FVector Dir = SparkVelocities[i] + FVector(0.f, 0.f, -800.f * Age);
		if (!Dir.IsNearlyZero()) SparkMeshes[i]->SetWorldRotation(Dir.Rotation());
	}

	if (Age >= Lifetime) Destroy();
}
