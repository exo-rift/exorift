// ExoMuzzleSmoke.cpp — Multi-puff muzzle smoke cloud
#include "Visual/ExoMuzzleSmoke.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoMuzzleSmoke::AExoMuzzleSmoke()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.2f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	for (int32 i = 0; i < NUM_PUFFS; i++)
	{
		FName Name = *FString::Printf(TEXT("Puff%d"), i);
		SmokePuffs[i] = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		SmokePuffs[i]->SetupAttachment(Root);
		if (SphereFinder.Succeeded()) SmokePuffs[i]->SetStaticMesh(SphereFinder.Object);
		SmokePuffs[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SmokePuffs[i]->CastShadow = false;
		SmokePuffs[i]->SetGenerateOverlapEvents(false);
		SmokePuffs[i]->SetRelativeScale3D(FVector(0.01f));
		SmokeMats[i] = nullptr;
	}
}

void AExoMuzzleSmoke::InitSmoke(const FVector& DriftDir)
{
	BaseScale = FMath::RandRange(0.2f, 0.4f);
	Lifetime = FMath::RandRange(0.5f, 0.8f);

	UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveOpaque();

	for (int32 i = 0; i < NUM_PUFFS; i++)
	{
		// Each puff gets slightly different drift direction
		FVector PuffDrift = DriftDir;
		PuffDrift += FVector(
			FMath::RandRange(-8.f, 8.f),
			FMath::RandRange(-8.f, 8.f),
			FMath::RandRange(5.f, 15.f));
		PuffDrift *= (0.7f + 0.3f * i / (float)NUM_PUFFS); // Later puffs are slower
		DriftVelocities[i] = PuffDrift;

		SmokePuffs[i]->SetRelativeLocation(FVector(
			FMath::RandRange(-3.f, 3.f),
			FMath::RandRange(-3.f, 3.f),
			FMath::RandRange(0.f, 5.f)));

		// Subtle grey-white smoke with very faint emissive
		if (EmMat)
		{
			SmokeMats[i] = UMaterialInstanceDynamic::Create(EmMat, this);
			float Brightness = 0.07f + FMath::RandRange(0.f, 0.03f);
			SmokeMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Brightness, Brightness, Brightness * 1.1f));
			SmokePuffs[i]->SetMaterial(0, SmokeMats[i]);
		}

		float PuffScale = BaseScale * (0.5f + 0.5f * i / (float)NUM_PUFFS);
		SmokePuffs[i]->SetRelativeScale3D(FVector(PuffScale * 0.3f));
	}
}

void AExoMuzzleSmoke::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	for (int32 i = 0; i < NUM_PUFFS; i++)
	{
		if (!SmokePuffs[i]) continue;

		// Drift and rise with turbulence
		FVector Pos = SmokePuffs[i]->GetRelativeLocation();
		Pos += DriftVelocities[i] * DeltaTime;
		DriftVelocities[i].Z += 25.f * DeltaTime; // Buoyancy
		DriftVelocities[i] *= (1.f - 0.7f * DeltaTime); // Drag

		// Slight wobble for turbulence
		float Wobble = FMath::Sin(Age * 8.f + i * 3.f) * 3.f;
		Pos.X += Wobble * DeltaTime;

		SmokePuffs[i]->SetRelativeLocation(Pos);

		// Expand then shrink — staggered per puff
		float PuffT = FMath::Clamp(T + i * 0.08f, 0.f, 1.f);
		float ExpandT = FMath::Min(PuffT * 3.f, 1.f);
		float ShrinkT = FMath::Max((PuffT - 0.5f) * 2.f, 0.f);
		float PuffScale = BaseScale * (0.6f + 0.4f * i / (float)NUM_PUFFS);
		float S = PuffScale * (0.3f + 0.7f * ExpandT) * (1.f - ShrinkT * 0.5f);
		SmokePuffs[i]->SetRelativeScale3D(FVector(S, S, S * 0.7f));

		// Fade emissive
		if (SmokeMats[i])
		{
			float Alpha = (1.f - PuffT * PuffT) * 0.7f;
			float B = (0.07f + i * 0.01f) * Alpha;
			SmokeMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(B, B, B * 1.1f));
		}
	}

	if (Age >= Lifetime) Destroy();
}

void AExoMuzzleSmoke::SpawnSmoke(UWorld* World, const FVector& MuzzlePos,
	const FRotator& MuzzleRot)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 2-3 smoke actors per shot, each with 3 puffs = 6-9 visible puffs
	int32 Count = FMath::RandRange(2, 3);
	for (int32 i = 0; i < Count; i++)
	{
		FVector Offset(
			FMath::RandRange(-5.f, 5.f),
			FMath::RandRange(-5.f, 5.f),
			FMath::RandRange(2.f, 8.f));

		AExoMuzzleSmoke* Smoke = World->SpawnActor<AExoMuzzleSmoke>(
			AExoMuzzleSmoke::StaticClass(), MuzzlePos + Offset,
			FRotator::ZeroRotator, Params);
		if (Smoke)
		{
			FVector Drift = MuzzleRot.Vector() * FMath::RandRange(20.f, 60.f)
				+ FVector(
					FMath::RandRange(-12.f, 12.f),
					FMath::RandRange(-12.f, 12.f),
					FMath::RandRange(10.f, 30.f));
			Smoke->InitSmoke(Drift);
		}
	}
}
