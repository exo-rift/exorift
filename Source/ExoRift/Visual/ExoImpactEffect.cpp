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
	CoreMesh->SetWorldScale3D(FVector(0.1f));

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
	FlashLight->SetIntensity(10000.f);
	FlashLight->SetAttenuationRadius(350.f);
	FlashLight->CastShadows = false;
}

void AExoImpactEffect::InitEffect(const FVector& HitNormal, bool bHitCharacter)
{
	// Color scheme: red/orange for characters, bright cyan-white for surfaces
	FLinearColor SparkColor = bHitCharacter
		? FLinearColor(8.f, 1.f, 0.3f, 1.f)
		: FLinearColor(4.f, 6.f, 8.f, 1.f);

	FLinearColor LightColor = bHitCharacter
		? FLinearColor(1.f, 0.2f, 0.1f)
		: FLinearColor(0.4f, 0.7f, 1.f);

	FLinearColor DustColor = bHitCharacter
		? FLinearColor(0.4f, 0.05f, 0.02f, 1.f)
		: FLinearColor(0.15f, 0.15f, 0.18f, 1.f);

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
			SM->SetVectorParameterValue(TEXT("BaseColor"), SparkColor * 0.8f);
			SM->SetVectorParameterValue(TEXT("EmissiveColor"), SparkColor * 0.6f);
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
		FVector Vel = HitNormal * FMath::RandRange(100.f, 250.f);
		Vel += Tangent * FMath::RandRange(-120.f, 120.f);
		Vel += Bitangent * FMath::RandRange(-120.f, 120.f);
		SparkVelocities[i] = Vel;

		float S = FMath::RandRange(0.015f, 0.03f);
		SparkMeshes[i]->SetWorldScale3D(FVector(S * 3.f, S, S));
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

	// Core: rapid shrink
	if (CoreMesh)
	{
		float S = 0.1f * FMath::Max(Alpha * 2.f - 1.f, 0.f);
		CoreMesh->SetWorldScale3D(FVector(FMath::Max(S, 0.001f)));
	}

	// Dust puff: expand outward and fade
	if (DustPuff)
	{
		float PuffScale = FMath::Lerp(0.05f, 0.4f, FMath::Sqrt(T));
		DustPuff->SetWorldScale3D(FVector(PuffScale));
		DustPuff->SetRelativeLocation(FVector(0.f, 0.f, Age * 30.f)); // Drift upward
	}

	// Sparks: fly outward with gravity
	for (int32 i = 0; i < SparkMeshes.Num(); i++)
	{
		if (!SparkMeshes[i]) continue;
		FVector Pos = SparkVelocities[i] * Age;
		Pos.Z -= 300.f * Age * Age;
		SparkMeshes[i]->SetRelativeLocation(Pos);

		// Shrink and orient along velocity
		float S = 0.025f * Alpha;
		SparkMeshes[i]->SetWorldScale3D(FVector(S * 2.f, S * 0.5f, S * 0.5f));
		FVector Dir = SparkVelocities[i] + FVector(0.f, 0.f, -600.f * Age);
		if (!Dir.IsNearlyZero()) SparkMeshes[i]->SetWorldRotation(Dir.Rotation());
	}

	if (Age >= Lifetime) Destroy();
}
