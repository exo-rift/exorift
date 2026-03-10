// ExoMuzzleSmoke.cpp — Post-fire muzzle smoke wisp
#include "Visual/ExoMuzzleSmoke.h"
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
	BaseScale = FMath::RandRange(0.1f, 0.2f);
	Lifetime = FMath::RandRange(0.4f, 0.7f);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!MatFinder.Succeeded()) return;

	SmokeMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
	FLinearColor SmokeCol(0.12f, 0.12f, 0.14f);
	SmokeMat->SetVectorParameterValue(TEXT("BaseColor"), SmokeCol);
	SmokeMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(0.03f, 0.03f, 0.04f));
	SmokePuff->SetMaterial(0, SmokeMat);
	SmokePuff->SetRelativeScale3D(FVector(BaseScale * 0.3f));
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

	// Fade
	if (SmokeMat)
	{
		float Alpha = (1.f - T * T) * 0.5f;
		SmokeMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.12f * Alpha, 0.12f * Alpha, 0.14f * Alpha));
	}

	if (Age >= Lifetime) Destroy();
}

void AExoMuzzleSmoke::SpawnSmoke(UWorld* World, const FVector& MuzzlePos,
	const FRotator& MuzzleRot)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 1-2 wisps per shot
	int32 Count = FMath::RandRange(1, 2);
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
