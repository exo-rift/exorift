// ExoShieldShatter.cpp — Shield break VFX with scattering hex fragments
#include "Visual/ExoShieldShatter.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoShieldShatter::AExoShieldShatter()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.2f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));

	auto MakeMesh = [&](const TCHAR* Name, UStaticMesh* Mesh) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (Mesh) C->SetStaticMesh(Mesh);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->SetGenerateOverlapEvents(false);
		return C;
	};

	// Central energy burst (sphere)
	CoreBurst = MakeMesh(TEXT("CoreBurst"), SphereFinder.Succeeded() ? SphereFinder.Object : nullptr);
	RootComponent = CoreBurst;

	// Shockwave ring (cylinder)
	ShockwaveRing = MakeMesh(TEXT("ShockwaveRing"), CylFinder.Succeeded() ? CylFinder.Object : nullptr);
	ShockwaveRing->SetupAttachment(CoreBurst);

	// Shield fragments (flat cubes representing hex-like shield plates)
	for (int32 i = 0; i < NUM_FRAGMENTS; i++)
	{
		FName Name = *FString::Printf(TEXT("Frag_%d"), i);
		UStaticMeshComponent* Frag = MakeMesh(*Name.ToString(),
			CubeFinder.Succeeded() ? CubeFinder.Object : nullptr);
		Frag->SetupAttachment(CoreBurst);
		Fragments.Add(Frag);
	}

	// Energy sparks
	for (int32 i = 0; i < NUM_SPARKS; i++)
	{
		FName Name = *FString::Printf(TEXT("ShieldSpark_%d"), i);
		UStaticMeshComponent* Spark = MakeMesh(*Name.ToString(),
			CubeFinder.Succeeded() ? CubeFinder.Object : nullptr);
		Spark->SetupAttachment(CoreBurst);
		Sparks.Add(Spark);
	}

	BurstLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BurstLight"));
	BurstLight->SetupAttachment(CoreBurst);
	BurstLight->SetIntensity(600000.f);
	BurstLight->SetAttenuationRadius(3000.f);
	BurstLight->SetLightColor(FLinearColor(0.2f, 0.5f, 1.f));
	BurstLight->CastShadows = false;
}

void AExoShieldShatter::InitShatter(const FVector& HitDirection)
{
	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (!EmissiveMat) return;

	auto MakeMat = [&](const FLinearColor& Col) -> UMaterialInstanceDynamic*
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!M) { return nullptr; }
		M->SetVectorParameterValue(TEXT("EmissiveColor"), Col);
		return M;
	};

	// Core burst — bright white-blue flash
	FLinearColor CoreCol(150.f, 250.f, 500.f);
	CoreBurst->SetMaterial(0, MakeMat(CoreCol));
	CoreBurst->SetRelativeScale3D(FVector(2.f));

	// Shockwave ring — expanding blue halo
	FLinearColor RingCol(40.f, 100.f, 200.f);
	ShockwaveRing->SetMaterial(0, MakeMat(RingCol));
	ShockwaveRing->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.01f));

	// Build tangent frame from hit direction for biased fragment scatter
	FVector ScatterBias = HitDirection.IsNearlyZero() ? FVector::UpVector : -HitDirection.GetSafeNormal();
	FVector Tangent, Bitangent;
	ScatterBias.FindBestAxisVectors(Tangent, Bitangent);

	// Fragments — flat shield plates that fly outward
	FragVelocities.SetNum(NUM_FRAGMENTS);
	FragRotSpeeds.SetNum(NUM_FRAGMENTS);
	FLinearColor FragCol(60.f, 150.f, 300.f);
	for (int32 i = 0; i < NUM_FRAGMENTS; i++)
	{
		// Distribute fragments in a sphere, biased away from hit
		float Angle = (2.f * PI * i) / NUM_FRAGMENTS;
		FVector Dir = ScatterBias * FMath::RandRange(0.3f, 1.f)
			+ Tangent * FMath::Cos(Angle) * FMath::RandRange(0.5f, 1.f)
			+ Bitangent * FMath::Sin(Angle) * FMath::RandRange(0.5f, 1.f);
		Dir.Normalize();
		FragVelocities[i] = Dir * FMath::RandRange(800.f, 2000.f);
		FragRotSpeeds[i] = FMath::RandRange(200.f, 600.f);

		// Flat hex-like plates with random orientation
		float S = FMath::RandRange(0.15f, 0.35f);
		Fragments[i]->SetRelativeScale3D(FVector(S, S * 1.3f, S * 0.08f));
		Fragments[i]->SetMaterial(0, MakeMat(FragCol * FMath::RandRange(0.7f, 1.3f)));
	}

	// Sparks — small energy bits
	SparkVelocities.SetNum(NUM_SPARKS);
	FLinearColor SparkCol(100.f, 200.f, 400.f);
	for (int32 i = 0; i < NUM_SPARKS; i++)
	{
		FVector Dir = FVector(
			FMath::RandRange(-1.f, 1.f),
			FMath::RandRange(-1.f, 1.f),
			FMath::RandRange(-0.5f, 1.f)).GetSafeNormal();
		SparkVelocities[i] = Dir * FMath::RandRange(1500.f, 3000.f);

		float S = FMath::RandRange(0.04f, 0.1f);
		Sparks[i]->SetRelativeScale3D(FVector(S * 4.f, S, S));
		Sparks[i]->SetMaterial(0, MakeMat(SparkCol * FMath::RandRange(0.6f, 1.4f)));
	}
}

void AExoShieldShatter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float Alpha = 1.f - T;

	// Core burst: rapid expand then vanish
	float CorePhase = FMath::Clamp(Age / 0.15f, 0.f, 1.f);
	float CoreS = 2.f * (1.f - CorePhase * CorePhase);
	CoreBurst->SetRelativeScale3D(FVector(FMath::Max(CoreS, 0.001f)));

	// Shockwave ring expands
	float RingS = 0.5f + T * 8.f;
	float RingThick = 0.02f * Alpha * Alpha;
	ShockwaveRing->SetRelativeScale3D(FVector(RingS, RingS, RingThick));

	// Light fades
	BurstLight->SetIntensity(600000.f * Alpha * Alpha * Alpha);

	// Fragments fly outward, tumble, and fade
	for (int32 i = 0; i < Fragments.Num(); i++)
	{
		FVector Pos = FragVelocities[i] * Age;
		Pos.Z -= 300.f * Age * Age; // Gravity
		Fragments[i]->SetRelativeLocation(Pos);

		float Rot = FragRotSpeeds[i] * Age;
		Fragments[i]->SetRelativeRotation(FRotator(Rot, Rot * 0.7f, Rot * 0.5f));

		// Fade size
		FVector Scale = Fragments[i]->GetRelativeScale3D();
		float Fade = FMath::Max(Alpha * 1.5f, 0.f);
		Fragments[i]->SetRelativeScale3D(FVector(
			Scale.X * Fade / FMath::Max(Fade, 0.01f), // Keep proportions
			Scale.Y, Scale.Z) * FMath::Min(Fade, 1.f));
	}

	// Sparks scatter and shrink
	for (int32 i = 0; i < Sparks.Num(); i++)
	{
		FVector Pos = SparkVelocities[i] * Age;
		Pos.Z -= 800.f * Age * Age;
		Sparks[i]->SetRelativeLocation(Pos);

		FVector Dir = SparkVelocities[i] + FVector(0.f, 0.f, -1600.f * Age);
		if (!Dir.IsNearlyZero()) Sparks[i]->SetWorldRotation(Dir.Rotation());

		float S = 0.06f * Alpha;
		float Stretch = FMath::Min(SparkVelocities[i].Size() / 300.f, 5.f);
		Sparks[i]->SetRelativeScale3D(FVector(S * Stretch, S * 0.4f, S * 0.4f));
	}

	if (Age >= Lifetime) Destroy();
}

void AExoShieldShatter::SpawnShatter(UWorld* World, const FVector& Location,
	const FVector& HitDirection)
{
	if (!World) return;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoShieldShatter* FX = World->SpawnActor<AExoShieldShatter>(
		AExoShieldShatter::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (FX) FX->InitShatter(HitDirection);
}
