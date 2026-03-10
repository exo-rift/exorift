// ExoMuzzleSmoke.cpp — Post-fire muzzle smoke wisp
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

	SmokePuff = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Puff"));
	if (SphereFinder.Succeeded()) SmokePuff->SetStaticMesh(SphereFinder.Object);
	SmokePuff->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SmokePuff->CastShadow = false;
	SmokePuff->SetGenerateOverlapEvents(false);
	RootComponent = SmokePuff;
}

void AExoMuzzleSmoke::InitSmoke(const FVector& DriftDir)
{
	DriftVelocity = DriftDir;
	BaseScale = FMath::RandRange(0.18f, 0.35f);
	Lifetime = FMath::RandRange(0.5f, 0.9f);

	SmokeMat = UMaterialInstanceDynamic::Create(
		FExoMaterialFactory::GetEmissiveOpaque(), this);
	SmokeMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(0.06f, 0.06f, 0.08f));
	SmokePuff->SetMaterial(0, SmokeMat);
	SmokePuff->SetRelativeScale3D(FVector(BaseScale * 0.4f));
}

void AExoMuzzleSmoke::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Expand and drift
	FVector Pos = GetActorLocation();
	Pos += DriftVelocity * DeltaTime;
	DriftVelocity.Z += 30.f * DeltaTime; // Rise
	DriftVelocity *= (1.f - 0.8f * DeltaTime); // Drag
	SetActorLocation(Pos);

	// Expand then shrink
	float ExpandT = FMath::Min(T * 3.f, 1.f); // Quick expand
	float ShrinkT = FMath::Max((T - 0.5f) * 2.f, 0.f); // Late shrink
	float S = BaseScale * (0.3f + 0.7f * ExpandT) * (1.f - ShrinkT * 0.6f);
	SmokePuff->SetRelativeScale3D(FVector(S, S, S * 0.7f));

	// Fade — more visible smoke that lingers
	if (SmokeMat)
	{
		float Alpha = (1.f - T * T) * 0.7f;
		SmokeMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.06f * Alpha, 0.06f * Alpha, 0.08f * Alpha));
	}

	if (Age >= Lifetime) Destroy();
}

void AExoMuzzleSmoke::SpawnSmoke(UWorld* World, const FVector& MuzzlePos,
	const FRotator& MuzzleRot)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 2-3 wisps per shot for visible smoke cloud
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
					FMath::RandRange(-15.f, 15.f),
					FMath::RandRange(-15.f, 15.f),
					FMath::RandRange(10.f, 30.f));
			Smoke->InitSmoke(Drift);
		}
	}
}
